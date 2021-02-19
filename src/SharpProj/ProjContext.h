#pragma once

using namespace System;

namespace SharpProj {
	namespace Proj {
		ref class ProjObject;
	}

	public enum class ProjLogLevel
	{
		None = PJ_LOG_NONE,
		Error = PJ_LOG_ERROR,
		Debug = PJ_LOG_DEBUG,
		Trace = PJ_LOG_TRACE
	};

	public ref class ProjContext
	{
	private:
		PJ_CONTEXT* m_ctx;
		gcroot<WeakReference<ProjContext^>^>* m_ref;
		void* m_chain;

		ProjContext(PJ_CONTEXT *ctx)
		{
			m_ctx = ctx;
		}

		void SetupNetworkHandling();

	public:
		static initonly String^ DefaultEndpointUrl = "https://cdn.proj.org";
		static property bool EnableNetworkConnectionsOnNewContexts;

	internal:
		String^ m_lastError;

		const char* utf8_string(String^ value);

		const char* utf8_chain(String^ value, void*& chain);
		void free_chain(void*& chain);

	public:
		ProjContext();

		~ProjContext();

		ProjContext^ Clone()
		{
			return gcnew ProjContext(proj_context_clone(this));
		}

		property bool AllowNetworkConnections
		{
			bool get()
			{
				return (0 != proj_context_is_network_enabled(this));
			}
			void set(bool value)
			{
				proj_context_set_enable_network(this, value);
			}
		}

		property String^ EndpointUrl
		{
			String^ get()
			{
				const char* c = proj_context_get_url_endpoint(this);
				return (c && *c) ? Utf8_PtrToString(c) : nullptr;
			}
			void set(String^ value)
			{
				std::string url = utf8_string(value);

				proj_context_set_url_endpoint(this, url.c_str());
			}
		}

		void SetGridCache(bool enabled, String^ path, int max_mb, int ttl_seconds)
		{
			proj_grid_cache_set_enable(this, enabled);
			if (enabled && path)
			{
				std::string p = utf8_string(path);
				proj_grid_cache_set_filename(this, p.c_str());
			}
			proj_grid_cache_set_max_size(this, max_mb > 0 ? max_mb : -1);
			proj_grid_cache_set_ttl(this, ttl_seconds > 0 ? ttl_seconds : -1);
		}

		void ClearGridCache()
		{
			proj_grid_cache_clear(this);
		}

	protected public:
		void OnFindFile(String^ file, [Out] String^% foundFile);
		void OnLogMessage(ProjLogLevel level, String^ message);

	public:
		ProjObject^ Create(String^ definition);
		ProjObject^ Create(...array<String^>^ from);

	internal:
		ProjObject^ Create(PJ* pj);

		generic<typename T> where T : ProjObject
			T Create(PJ* pj);

		void ClearError()
		{
			m_lastError = nullptr;
		}

		void ClearError(PJ* pj)
		{
			m_lastError = nullptr;
			if (pj)
				proj_errno_reset(pj);			
		}

	public:
		property ProjLogLevel LogLevel
		{
			ProjLogLevel get()
			{
				return (ProjLogLevel)proj_log_level(this, PJ_LOG_TELL);
			}
			void set(ProjLogLevel value)
			{
				proj_log_level(this, (PJ_LOG_LEVEL)value);
			}
		}

		event System::Action<ProjLogLevel, String^>^ Log;

		String^ GetMetaData(String^ key);

	public:
		/// <summary>PROJ Version, wrapped by SharpProj</summary>
		property System::Version^ Version
		{
			System::Version^ get()
			{
				return gcnew System::Version(PROJ_VERSION_MAJOR, PROJ_VERSION_MINOR, PROJ_VERSION_PATCH);
			}
		}

		/// <summary>EPSG Version. As stored in Proj Database</summary>
		property System::Version^ EpsgVersion
		{
			System::Version ^ get();
		}

	protected:
		virtual void OnLog(ProjLogLevel level, String^ message)
		{
			Log(level, message);
		}

	private:
		static String^ EnvCombine(String^ envVar, String^ file);

	public:
		static void DownloadProjDB(String^ toPath);
	internal:
		static operator PJ_CONTEXT* (ProjContext^ me)
		{
			if ((Object^)me == nullptr)
				return nullptr;
			else if (!me->m_ctx)
				throw gcnew System::ObjectDisposedException("Context already disposed");

			return me->m_ctx;
		}

		System::Exception^ ConstructException();
	};
}
