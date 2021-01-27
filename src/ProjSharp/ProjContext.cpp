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

static const char* my_file_finder(PJ_CONTEXT* ctx, const char* file, void* user_data)
{
	gcroot<WeakReference<ProjContext^>^>* ref = (gcroot<WeakReference<ProjContext^>^>*)user_data;

	ProjContext^ pc;
	if ((*ref)->TryGetTarget(pc))
	{
		String^ origFile = gcnew String(file);
		String^ newFile;
		pc->OnFindFile(origFile, newFile);

		if (newFile && !origFile->Equals(newFile))
		{
			std::string sNewFile = utf8_string(newFile);
			return _strdup(sNewFile.c_str());
		}
		else
			return file;
	}
	return file;
}

static void my_log_func(void* user_data, int level, const char* message)
{
	gcroot<WeakReference<ProjContext^>^>* ref = (gcroot<WeakReference<ProjContext^>^>*)user_data;

	ProjContext^ pc;
	if ((*ref)->TryGetTarget(pc))
	{
		String^ msg = gcnew String(message);

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
	}
}

Exception^ ProjContext::ConstructException()
{
	int err = proj_context_errno(this);

	return gcnew ProjException(gcnew String(proj_errno_string(err)));
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

}