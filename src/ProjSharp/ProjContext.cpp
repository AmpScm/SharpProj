#include "pch.h"

#include <locale>
#include <codecvt>
#include "ProjContext.h"
#include "ProjException.h"

using namespace ProjSharp;
using namespace System::IO;
using System::Net::WebRequest;
using System::Net::WebResponse;
using System::Net::HttpWebRequest;
using System::Net::HttpWebResponse;
using System::Net::HttpStatusCode;

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

struct my_network_data
{
	gcroot<ProjContext^> ctx;
	gcroot<String^> url;
	gcroot<System::Net::WebResponse^> rp;
	long long offset;
	long long len;
	void* chain;
};


static PROJ_NETWORK_HANDLE* my_network_open(
	PJ_CONTEXT* ctx,
	const char* url,
	unsigned long long offset,
	size_t size_to_read,
	void* buffer,
	size_t* out_size_read,
	size_t error_string_max_size,
	char* out_error_string,
	void* user_data)
{
	gcroot<WeakReference<ProjContext^>^>& ref = *(gcroot<WeakReference<ProjContext^>^>*)user_data;
	ProjContext^ pc;
	if (!ref->TryGetTarget(pc))
		return nullptr;

	if (error_string_max_size > 0 && out_error_string)
		out_error_string[0] = '\0';


	String^ sUrl = gcnew String(url);
	WebRequest^ rq = WebRequest::Create(sUrl);
	HttpWebRequest^ hrq = dynamic_cast<HttpWebRequest^>(rq);

	if (hrq != nullptr)
	{
		hrq->AddRange((long long)offset, (long long)(offset + size_to_read));
	}
	else
		rq->Headers->Add(String::Format("Range: bytes={0}-{1}", offset, offset + size_to_read-1));
	
	WebResponse^ rp;
	bool in_error = true;
	
	try
	{
		rp = rq->GetResponse();
		in_error = false;
	}
	catch(System::Net::WebException ^wx)
	{
		pc->OnLogMessage(ProjLogLevel::Debug, wx->ToString());
		rp = wx->Response;
	}

	HttpWebResponse^ hrp = dynamic_cast<HttpWebResponse^>(rp);

	if (hrp && hrp->StatusCode == HttpStatusCode::PartialContent)
	{
		System::IO::Stream^ s = hrp->GetResponseStream();
		array<unsigned char>^ buf = gcnew array<unsigned char>(size_to_read);

		int r = s->Read(buf, 0, size_to_read);

		while (r > 0 && r < size_to_read)
		{
			int n = s->Read(buf, r, size_to_read - r);

			if (n > 0)
				r += n;
			else
				break;
		}

		pin_ptr<unsigned char> pBuf = &buf[0];
		memcpy(buffer, pBuf, r);
		*out_size_read = r;

	}
	else if (!in_error)
	{
		strncpy_s(out_error_string, error_string_max_size, "No partial web response", error_string_max_size);
		return nullptr;
	}
	else
	{
		strncpy_s(out_error_string, error_string_max_size, "Http error", error_string_max_size);
		return nullptr;
	}

	my_network_data* d = new my_network_data();
	d->ctx = pc;
	d->url = sUrl;
	d->rp = rp;
	d->offset = offset;
	d->len = size_to_read;
	d->chain = nullptr;
	return (PROJ_NETWORK_HANDLE *)(void*)d;
}

static void my_network_close(
	PJ_CONTEXT* ctx,
	PROJ_NETWORK_HANDLE* handle,
	void* user_data)
{
	my_network_data* d = (my_network_data*)handle;

	d->ctx->free_chain(d->chain);

	delete d;
}

const char* my_network_get_header_value(
	PJ_CONTEXT* ctx,
	PROJ_NETWORK_HANDLE* handle,
	const char* header_name,
	void* user_data)
{
	my_network_data* d = (my_network_data*)handle;

	String^ h = d->rp->Headers->Get(gcnew String(header_name));

	if (h)
	{
		return d->ctx->utf8_chain(h, d->chain);
	}

	return nullptr;
}

size_t my_network_read_range(
	PJ_CONTEXT* ctx,
	PROJ_NETWORK_HANDLE* handle,
	unsigned long long offset,
	size_t size_to_read,
	void* buffer,
	size_t error_string_max_size,
	char* out_error_string,
	void* user_data)
{
	my_network_data* d = (my_network_data*)handle;

	if (error_string_max_size > 0 && out_error_string)
		out_error_string[0] = '\0';

	delete d->rp;
	d->rp = nullptr;

	WebRequest^ rq = WebRequest::Create(d->url);
	HttpWebRequest^ hrq = dynamic_cast<HttpWebRequest^>(rq);

	if (hrq != nullptr)
	{
		hrq->AddRange((long long)offset, (long long)(offset + size_to_read));
	}
	else
		rq->Headers->Add(String::Format("Range: bytes={0}-{1}", offset, offset + size_to_read - 1));

	WebResponse^ rp;
	bool in_error = true;

	try
	{
		rp = rq->GetResponse();
		in_error = false;
	}
	catch (System::Net::WebException^ wx)
	{
		d->ctx->OnLogMessage(ProjLogLevel::Debug, wx->ToString());
		rp = wx->Response;
	}

	HttpWebResponse^ hrp = dynamic_cast<HttpWebResponse^>(rp);

	if (hrp && hrp->StatusCode == HttpStatusCode::PartialContent)
	{
		d->rp = rp;
		System::IO::Stream^ s = hrp->GetResponseStream();
		array<unsigned char>^ buf = gcnew array<unsigned char>(size_to_read);

		int r = s->Read(buf, 0, size_to_read);

		while (r > 0 && r < size_to_read)
		{
			int n = s->Read(buf, r, size_to_read - r);

			if (n > 0)
				r += n;
			else
				break;
		}

		pin_ptr<unsigned char> pBuf = &buf[0];
		memcpy(buffer, pBuf, r);

		return r;
	}
	else if (!in_error)
	{
		strncpy_s(out_error_string, error_string_max_size, "No partial web response", error_string_max_size);
		return 0;
	}
	else
	{
		strncpy_s(out_error_string, error_string_max_size, "Http error", error_string_max_size);
		return 0;
	}
	return 0;
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

		// Somehow curl is not enabled in the vcpkg build. Let's use the .Net HTTP stack
		proj_context_set_network_callbacks(
			m_ctx,
			my_network_open,
			my_network_close,
			my_network_get_header_value,
			my_network_read_range,
			m_ref);

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
