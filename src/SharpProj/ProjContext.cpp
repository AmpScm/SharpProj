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
	int slen = v.length() + 1;
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
	gcroot<WeakReference<ProjContext^>^>& ref = *(gcroot<WeakReference<ProjContext^>^>*)user_data;

	ProjContext^ pc;
	if (ref->TryGetTarget(pc))
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
	gcroot<WeakReference<ProjContext^>^>& ref = *(gcroot<WeakReference<ProjContext^>^>*)user_data;

	ProjContext^ pc;
	if (ref->TryGetTarget(pc))
	{
		String^ msg = Utf8_PtrToString(message);
		
		pc->OnLogMessage((ProjLogLevel)level, msg);
	}
}



ProjContext::ProjContext()
	: ProjContext(proj_context_create())
{
	if (EnableNetworkConnectionsOnNewContexts)
		EnableNetworkConnections = true; // Otherwise follow environment variable

	proj_log_level(m_ctx, PJ_LOG_ERROR); // Ignore environment variable!
	proj_context_use_proj4_init_rules(m_ctx, false); // Ignore environment variable!
}

ProjContext::ProjContext(PJ_CONTEXT* ctx)
{
	if (!ctx)
		throw gcnew ArgumentNullException("ctx");

	m_ctx = ctx;

	WeakReference<ProjContext^>^ wr = gcnew WeakReference<ProjContext^>(this);
	m_ref = new gcroot<WeakReference<ProjContext^>^>(wr);

	proj_context_set_file_finder(m_ctx, my_file_finder, m_ref);
	proj_log_func(m_ctx, m_ref, my_log_func);

	SetupNetworkHandling();
}

ProjContext^ ProjContext::Clone()
{
	return gcnew ProjContext(proj_context_clone(this));
}

SharpProj::ProjContext::~ProjContext()
{
	if (m_ctx)
	{
		proj_context_destroy(m_ctx);
		m_ctx = nullptr;
	}
	if (m_ref)
	{
		delete m_ref;
		m_ref = nullptr;
	}

	FlushChain();
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
	String^ md = GetMetaData("ESRI.VERSION");
	System::Version^ v = nullptr;

	if (md->StartsWith("ArcMap") && System::Version::TryParse(md->Substring(6)->Trim(), v))
		return v;
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


ProjException^ ProjContext::CreateException(int err, String^ message, System::Exception^ inner)
{
	if (err >= PROJ_ERR_INVALID_OP && err < PROJ_ERR_COORD_TRANSFM)
	{
		if (inner)
			return gcnew ProjOperationException(message, inner);
		else
			return gcnew ProjOperationException(message);
	}
	else if (err >= PROJ_ERR_COORD_TRANSFM && err < PROJ_ERR_OTHER)
	{
		if (inner)
			return gcnew ProjTransformException(message, inner);
		else
			return gcnew ProjTransformException(message);
	}
	else
	{
		if (inner)
			return gcnew ProjException(message, inner);
		else
			return gcnew ProjException(message);
	}
}

Exception^ ProjContext::ConstructException(String ^prefix)
{
	int err = proj_context_errno(this);

	String^ msg = (prefix && m_lastError) ? (prefix + ": " + m_lastError) :
		(m_lastError ? m_lastError : prefix);

	if (msg)
	{
		m_lastError = nullptr;
		return CreateException(err, Utf8_PtrToString(proj_context_errno_string(this, err)),
			CreateException(err, msg, nullptr));
	}
	else
	{
		return CreateException(err, Utf8_PtrToString(proj_context_errno_string(this, err)), nullptr);
	}
}

String^ ProjContext::EnvCombine(String^ envVar, String^ file)
{
	try
	{
		String^ ev = Environment::GetEnvironmentVariable("PROJ_LIB");

		if (String::IsNullOrEmpty(ev))
			return file;

		return Path::GetFullPath(Path::Combine(ev, file));
	}
	catch(IOException^)
	{
		return file;
	}
	catch (ArgumentException^)
	{
		return file;
	}
}

bool ProjContext::CanWriteFromResource(String^ file, String^ resultFile)
{
	auto stream = ProjContext::typeid->Assembly->GetManifestResourceStream(file);

	if (stream)
	{
		FileStream fs(resultFile, FileMode::CreateNew);
		stream->CopyTo(%fs);

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
					p = Path::GetDirectoryName(p);

				if (!dirs->Contains(p))
					dirs->Add(p);
			}
		}
		catch (Exception^)
		{ /* Assembly security restrictions */
		}

		if (AppDomain::CurrentDomain->BaseDirectory && !dirs->Contains(AppDomain::CurrentDomain->BaseDirectory))
			dirs->Add(AppDomain::CurrentDomain->BaseDirectory);

		try
		{
			String^ libs = System::Environment::GetEnvironmentVariable("PROJ_LIB");

			if (!String::IsNullOrEmpty(libs))
			{
				for each (String ^ p in libs->Split(System::IO::Path::PathSeparator))
				{
					if (String::IsNullOrWhiteSpace(p))
						continue;

					auto pp = Path::GetFullPath(p);

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

	if (File::Exists(testFile = Path::Combine(userDir, file)))
		return testFile;
	else if (File::Exists(testFile = Path::Combine(userDir, ("proj" "-" PROJ_VERSION "-") + file)))
		return testFile;
	else if (CanWriteFromResource(file, testFile /* = Path::Combine(userDir, ("proj" "-" PROJ_VERSION "-") + file)*/))
		return testFile;
	else if (file == "proj.db" && EnableNetworkConnections)
	{
		testFile = Path::Combine(userDir, ("proj" PROJ_VERSION "-") + file);
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

	if (!m_lastError)
		m_lastError = String::Format("File '{0}' not found", file);

	return nullptr;
}

void ProjContext::OnLogMessage(ProjLogLevel level, String^ message)
{
	if (level == ProjLogLevel::Error)
		m_lastError = message;
	else
		m_lastError = nullptr;

#ifdef _DEBUG
	if (level <= ProjLogLevel::Error)
		System::Diagnostics::Debug::WriteLine(message);
#endif

	OnLog(level, message);
}
