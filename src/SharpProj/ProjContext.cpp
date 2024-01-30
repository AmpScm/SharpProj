#include "pch.h"

#include <locale>
#include <codecvt>
#include "ProjContext.h"
#include "ProjException.h"

using namespace SharpProj;
using namespace System::IO;
using System::Collections::Generic::List;

std::string utf8_string(String^ v)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    pin_ptr<const wchar_t> pPath = PtrToStringChars(v);
    std::wstring vstr(pPath);
    std::string sstr(conv.to_bytes(vstr));
    return sstr;
}

const char* ProjContext::utf8_string(String^ value)
{
    void* chain = m_chain;
    try
    {
        return utf8_chain(value, chain);
    }
    finally
    {
        m_chain = chain;
    }
}

const char* ProjContext::utf8_chain(String^ value, void*& chain)
{
    std::string v = ::utf8_string(value);
    size_t slen = v.length() + 1;
    void** pp = (void**)malloc(slen + sizeof(void*));
    pp[0] = chain;
    chain = pp;
    return (char*)memcpy(&pp[1], v.c_str(), slen);
}

void ProjContext::free_chain(void*& chain)
{
    while (chain)
    {
        void* next = ((void**)chain)[0];
        free(chain);

        chain = next;
    }
}

static const char* my_file_finder(PJ_CONTEXT* ctx, const char* file, void* user_data)
{
    UNUSED_ALWAYS(ctx);
    const auto& ref = *(ctx_wrapper<PJ_CONTEXT, ProjContext>*)user_data;

    ProjContext^ pc;
    if (ref.TryGetTarget(pc))
    {
        pc->FlushChain();

        String^ origFile = Utf8_PtrToString(file);

        String^ newFile = pc->FindFile(origFile);

        if (newFile && !origFile->Equals(newFile))
            return pc->utf8_string(newFile);
        else
            return file;
    }
    return file;
}

static void my_log_func(void* user_data, int level, const char* message)
{
    const auto& ref = *(ctx_wrapper<PJ_CONTEXT, ProjContext>*)user_data;

    ProjContext^ pc;
    if (ref.TryGetTarget(pc))
    {
        if (level <= PJ_LOG_ERROR || (ProjLogLevel)level <= pc->LogLevel)
        {
            String^ msg = Utf8_PtrToString(message);

            pc->OnLogMessage((ProjLogLevel)level, msg);
        }
    }
}



ProjContext::ProjContext()
    : ProjContext(proj_context_create())
{
    if (EnableNetworkConnectionsOnNewContexts)
        EnableNetworkConnections = true; // Otherwise follow environment variable

    LogLevel = ProjLogLevel::Error;
    proj_context_use_proj4_init_rules(m_ctx, false); // Ignore environment variable!
}

ProjContext::ProjContext(PJ_CONTEXT* ctx)
    : m_ctx(*new ctx_wrapper<PJ_CONTEXT, ProjContext>(ctx, proj_context_destroy))
{
    if (!ctx)
        throw gcnew ArgumentNullException("ctx");

    m_ctx.SetTarget(this);

    proj_context_set_file_finder(m_ctx, my_file_finder, &m_ctx);
    proj_log_func(m_ctx, &m_ctx, my_log_func);
    m_logLevel = (ProjLogLevel)proj_log_level(m_ctx, PJ_LOG_TELL);

    SetupNetworkHandling();
}

ProjContext^ ProjContext::Clone()
{
    auto pc = gcnew ProjContext(proj_context_clone(this));

    pc->m_logLevel = m_logLevel;
    return pc;
}

SharpProj::ProjContext::~ProjContext()
{
    if (!m_disposed)
        EnableNetworkConnections = false;

    ProjContext::!ProjContext();
}

ProjContext::!ProjContext()
{
    FlushChain();
    if (m_disposed)
        return;
    m_disposed = true;
    m_ctx.ReleaseMe();
}


void SharpProj::ProjContext::FlushChain()
{
    void* chain = m_chain;
    try
    {
        free_chain(chain);
    }
    finally
    {
        m_chain = chain;
    }
}

String^ ProjContext::GetMetaData(String^ key)
{
    if (String::IsNullOrEmpty(key))
        throw gcnew ArgumentNullException("key");

    std::string skey = utf8_string(key);

    const char* v = proj_context_get_database_metadata(this, skey.c_str());

    if (!v)
        throw gcnew ArgumentOutOfRangeException("key");

    return Utf8_PtrToString(v);
}

Version^ ProjContext::EpsgVersion::get()
{
    String^ md = GetMetaData("EPSG.VERSION");
    System::Version^ v = nullptr;

    if (md->StartsWith("v") && System::Version::TryParse(md->Substring(1), v))
        return v;
    else
        return nullptr;
}

Version^ ProjContext::EsriVersion::get()
{
    String^ md = GetMetaData("ESRI.DATE");
    System::Version^ v = nullptr;

    if (!String::IsNullOrEmpty(md) && System::Version::TryParse(md->Replace('-', '.'), v))
        return v;

    md = GetMetaData("ESRI.VERSION");
    v = nullptr;

    if (md->StartsWith("ArcGIS Pro") && System::Version::TryParse(md->Substring(10)->Trim(), v))
        return v; // Format 9.1.0 (and later?)
    else if (md->StartsWith("ArcMap") && System::Version::TryParse(md->Substring(6)->Trim(), v))
        return v; // Format for 9.0
    else
        return nullptr;
}

Version^ ProjContext::IgnfVersion::get()
{
    String^ md = GetMetaData("IGNF.VERSION");
    System::Version^ v = nullptr;

    if (md && System::Version::TryParse(md, v))
        return v;
    else
        return nullptr;
}

Version^ ProjContext::ProjDataVersion::get()
{
    String^ md = GetMetaData("PROJ_DATA.VERSION");
    System::Version^ v = nullptr;

    if (md && System::Version::TryParse(md, v))
        return v;
    else
        return nullptr;
}


ProjException^ ProjContext::CreateException(int err, String^ message, System::Exception^ inner)
{
    if (err >= PROJ_ERR_INVALID_OP && err < PROJ_ERR_COORD_TRANSFM)
    {
        if (err == PROJ_ERR_INVALID_OP_MISSING_ARG || err == PROJ_ERR_INVALID_OP_ILLEGAL_ARG_VALUE)
            return gcnew ProjOperationArgumentException(message, inner);
        else
            return gcnew ProjOperationException(message, inner);
    }
    else if (err >= PROJ_ERR_COORD_TRANSFM && err < PROJ_ERR_OTHER)
    {
        return gcnew ProjTransformException(message, inner);
    }
    else
    {
        return gcnew ProjException(message, inner);
    }
}

Exception^ ProjContext::ConstructException(String^ prefix)
{
    int err = proj_context_errno(this);

    String^ msg = (prefix && m_lastError) ? (prefix + ": " + m_lastError) :
        (m_lastError ? m_lastError : prefix);
    m_lastError = nullptr;

    String^ projMsg = Utf8_PtrToString(proj_context_errno_string(this, err));

    if (msg && projMsg && (msg->Contains(projMsg) || projMsg->StartsWith("Unknown error (code ")))
    {
        projMsg = msg;
        msg = nullptr;
    }

    return CreateException(err, projMsg, msg ? CreateException(err, msg, nullptr) : nullptr);
}

bool ProjContext::CanWriteFromResource(String^ file, String^ userDir, String^ resultFile)
{
    auto stream = ProjContext::typeid->Assembly->GetManifestResourceStream(file);

    if (stream)
    {
        try
        {
            DateTime old = DateTime::Now.Date.AddMonths(-1);
            for each (FileInfo ^ fi in (gcnew DirectoryInfo(userDir))->GetFiles("#proj-*"))
            {
                if (fi->LastWriteTime < old && fi->LastAccessTime < old)
                    fi->Delete();
            }
        }
        catch (Exception^)
        {
        }

        FileStream fs(resultFile, FileMode::CreateNew);
        stream->CopyTo(% fs);

        return true;
    }

    return false;
}

System::Collections::Generic::IEnumerable<String^>^ ProjContext::ProjLibDirs::get()
{
    if (!_projLibDirs)
    {
        List<String^>^ dirs = gcnew List<String^>();

        auto asb = ProjContext::typeid->Assembly;

        if (asb && asb->Location)
            dirs->Add(Path::GetDirectoryName(asb->Location));

        try
        {
            if (asb && asb->CodeBase)
            {
                auto p = (gcnew System::Uri(asb->CodeBase))->LocalPath;

                if (p)
                {
                    p = Path::GetDirectoryName(p);

                    if (!dirs->Contains(p))
                        dirs->Add(p);
                }
            }
        }
        catch (Exception^)
        { /* Assembly security restrictions */
        }

        if (AppDomain::CurrentDomain->BaseDirectory)
        {
            auto p = Path::GetDirectoryName(AppDomain::CurrentDomain->BaseDirectory);
            if (!dirs->Contains(p))
                dirs->Add(p);
        }

        try
        {
            PJ_INFO pi = proj_info();
            String^ libs = Utf8_PtrToString(pi.searchpath);

            if (!String::IsNullOrEmpty(libs))
            {
                for each (String ^ p in libs->Split(System::IO::Path::PathSeparator))
                {
                    if (String::IsNullOrWhiteSpace(p))
                        continue;

                    auto pp = Path::GetFullPath(p->Trim());

                    if (Directory::Exists(pp) && !dirs->Contains(pp))
                        dirs->Add(pp);
                }
            }
        }
        catch (Exception^)
        { /* Assembly security restrictions */
        }


        try
        {
            auto libs = Environment::GetEnvironmentVariable("PROJ_DATA");

            if (!String::IsNullOrEmpty(libs))
            {
                for each (String ^ p in libs->Split(System::IO::Path::PathSeparator))
                {
                    if (String::IsNullOrWhiteSpace(p))
                        continue;

                    auto pp = Path::GetFullPath(p->Trim());

                    if (Directory::Exists(pp) && !dirs->Contains(pp))
                        dirs->Add(pp);
                }
            }
        }
        catch (Exception^)
        { /* Assembly security restrictions */
        }

        _projLibDirs = dirs->ToArray();
    }

    return static_cast<System::Collections::Generic::IEnumerable<String^>^>(_projLibDirs);
}

// Update last write time when last write time more than one day old
void ProjContext::TouchFile(String^ file)
{
    try
    {
        if (File::GetLastWriteTime(file) < DateTime::Now.Date.AddDays(-1))
        {
            File::SetLastWriteTime(file, DateTime::Now);
        }
    }
    catch (Exception^)
    {
    }
}

String^ ProjContext::FindFile(String^ file)
{
    String^ testFile;

    for each (String ^ dir in ProjLibDirs)
    {
        if (File::Exists(testFile = Path::Combine(dir, file)))
            return testFile;
    }

    const char* pUserDir = proj_context_get_user_writable_directory(this, false);
    String^ userDir = Utf8_PtrToString(pUserDir);


    // UserDir is already contained in ProjLibDirs, so need to probe for the normal name here
    if (File::Exists(testFile = Path::Combine(userDir, ("#proj" "-" PROJ_VERSION "-") + file)))
    {
        TouchFile(testFile);
        return testFile;
    }
    else if (CanWriteFromResource(file, userDir, testFile /* = Path::Combine(userDir, ("#proj" "-" PROJ_VERSION "-") + file)*/))
        return testFile;
    else if (file == "proj.db" && (EnableNetworkConnections || m_enableNetwork))
    {
        // testFile = Path::Combine(userDir, ("#proj-" PROJ_VERSION "-") + file);
        try
        {
            DownloadProjDB(testFile);
        }
        catch (IOException^)
        {
            testFile = nullptr;
        }
        catch (System::Net::WebException^)
        {
            testFile = nullptr;
        }
        catch (InvalidOperationException^)
        {
            testFile = nullptr;
        }

        if (testFile && File::Exists(testFile))
            return testFile;
    }

    return nullptr;
}

void ProjContext::OnLogMessage(ProjLogLevel level, String^ message)
{
    if (level == ProjLogLevel::Error)
        m_lastError = message;
    else
        m_lastError = nullptr;

    if (level >= LogLevel)
        return;

    OnLog(level, message);
}
