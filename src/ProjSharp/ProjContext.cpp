#include "pch.h"

#include <locale>
#include <codecvt>
#include "ProjContext.h"
#include "ProjException.h"

using namespace ProjSharp;
using namespace System::IO;

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
		String^ origFile = gcnew String(file);
		String^ newFile;
		pc->OnFindFile(origFile, newFile);

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
		String^ msg = gcnew String(message);

		if (level == PJ_LOG_ERROR)
			pc->m_lastError = msg;
		else
			pc->m_lastError = nullptr;

#ifdef _DEBUG
		System::Diagnostics::Debug::WriteLine(msg);
#endif

		pc->OnLogMessage((ProjLogLevel)level, msg);
	}
}



ProjContext::ProjContext()
{
	m_ctx = proj_context_create();

	if (m_ctx)
	{
		WeakReference<ProjContext^>^ wr = gcnew WeakReference<ProjContext^>(this);
		m_ref = new gcroot<WeakReference<ProjContext^>^>(wr);

		proj_context_set_file_finder(m_ctx, my_file_finder, m_ref);
		proj_log_func(m_ctx, m_ref, my_log_func);
		proj_log_level(m_ctx, PJ_LOG_ERROR);

		SetupNetworkHandling();
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

	return gcnew String(v);
}
Version^ ProjContext::EpsgVersion::get()
{
	try
	{
		String^ md = GetMetaData("EPSG.VERSION");

		if (md->StartsWith("v"))
			return gcnew Version(md->Substring(1));
	}
	catch (ArgumentException^)
	{
		return nullptr;
	}
}


Exception^ ProjContext::ConstructException()
{
	int err = proj_context_errno(this);


	String^ msg = m_lastError;

	if (msg)
	{
		return gcnew ProjException(gcnew String(proj_errno_string(err)),
				gcnew ProjException(msg));
	}
	else
	{
		return gcnew ProjException(gcnew String(proj_errno_string(err)));
	}
}

void ProjContext::OnFindFile(String^ file, [Out] String^% foundFile)
{
	if (File::Exists(file))
		foundFile = Path::GetFullPath(file);
	else if (File::Exists(file = Path::Combine("..", file)))
		foundFile = Path::GetFullPath(file);
	else if (File::Exists(file = Path::Combine("..", file)))
		foundFile = Path::GetFullPath(file);
}

void ProjContext::OnLogMessage(ProjLogLevel level, String^ message)
{
	OnLog(level, message);
}
