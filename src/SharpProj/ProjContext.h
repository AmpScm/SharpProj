#pragma once


#if 1
#define EMPTY_ARRAY(T) gcnew array<T> {}
#else
#define EMPTY_ARRAY(T) Array::Empty<T>()
#endif

using namespace System;

namespace SharpProj {
	using System::Diagnostics::DebuggerBrowsableAttribute;
	using System::Diagnostics::DebuggerBrowsableState;

	ref class ProjException;
	ref class CoordinateReferenceSystem;

	namespace Proj {
		ref class ProjObject;
		ref class CoordinateReferenceSystemFilter;
		ref class CoordinateReferenceSystemInfo;
	}

	public enum class ProjLogLevel
	{
		None = PJ_LOG_NONE,
		Error = PJ_LOG_ERROR,
		Debug = PJ_LOG_DEBUG,
		Trace = PJ_LOG_TRACE
	};

	/// <summary>
	/// Context objects enable safe multi-threaded usage of SharpProj. Each Proj object is connected to some context
	/// (if not specified, a default related or new context is used). All operations within a context should be
	/// performed in the same thread.
	/// </summary>
	public ref class ProjContext
	{
	private:
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		PJ_CONTEXT* m_ctx;
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		gcroot<WeakReference<ProjContext^>^>* m_ref;
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		void* m_chain;
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		String^ m_lastError;
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		ProjLogLevel m_logLevel;

		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		static array<String^>^ _projLibDirs;

		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		bool m_autoCloseSession;

		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		int m_nRefs;

		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		int m_disposed;

		ProjContext(PJ_CONTEXT* ctx);
		void SetupNetworkHandling();

	public:
		static initonly String^ DefaultEndpointUrl = "https://cdn.proj.org";
		static property bool EnableNetworkConnectionsOnNewContexts;

	internal:
		const char* utf8_string(String^ value);

		const char* utf8_chain(String^ value, void*& chain);
		void free_chain(void*& chain);
		void FlushChain();

	public:
		/// <summary>
		/// Creates a new unrelated context
		/// </summary>
		ProjContext();

	private:
		!ProjContext();
		~ProjContext();
		void NoMoreReferences();

	public:
		/// <summary>
		/// Creates a disconnected copy of this context.
		/// </summary>
		/// <returns></returns>
		ProjContext^ Clone();

		/// <summary>
		/// Enable or disable network access. This allows access to not locally available grid transforms, etc.
		/// </summary>
		property bool EnableNetworkConnections
		{
			bool get()
			{
				return (0 != proj_context_is_network_enabled(this));
			}
			void set(bool value)
			{
				proj_context_set_enable_network(this, value);

				if (value)
				{
					const char* c = proj_context_get_url_endpoint(m_ctx);

					if (!c || !*c)
						proj_context_set_url_endpoint(m_ctx, "https://cdn.proj.org");
				}
			}
		}

		/// <summary>
		/// Keeps a reference to the database open after use
		/// </summary>
		property bool AutoCloseSession
		{
			bool get()
			{
				return m_autoCloseSession;
			}
			void set(bool value)
			{
				m_autoCloseSession = value;
				proj_context_set_autoclose_database(this, value);
			}
		}

		/// <summary>
		/// Gets or sets the URL used to access the proj data
		/// </summary>
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

		/// <summary>
		/// Sets up a specific caching location. By default a standard per user cache in local appdata is used.
		/// </summary>
		/// <param name="enabled"></param>
		/// <param name="path"></param>
		/// <param name="max_mb"></param>
		/// <param name="ttl_seconds"></param>
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

		/// <summary>
		/// Clears the current grid cache. Grid files will be reloaded when required
		/// </summary>
		void ClearGridCache()
		{
			proj_grid_cache_clear(this);
		}

	private:
		bool CanWriteFromResource(String^ file, String^ userDir, String^ resultFile);
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		property System::Collections::Generic::IEnumerable<String^>^ ProjLibDirs
		{
			System::Collections::Generic::IEnumerable<String^>^ get();
		}
		void TouchFile(String^ file);

	internal:
		String^ FindFile(String^ file);
		void OnLogMessage(ProjLogLevel level, String^ message);

	public:
		/// <summary>
		/// Creates a new proj object from its definition. Definiton could be an auth string, a projstring, wellknowntext or even projjson.
		/// </summary>
		/// <param name="definition"></param>
		/// <returns></returns>
		ProjObject^ Create(String^ definition);
		/// <summary>
		/// Creates a new proj object from its definition. Definiton could be an auth string, a projstring, wellknowntext or even projjson.
		/// </summary>
		/// <param name="from"></param>
		/// <returns></returns>
		ProjObject^ Create(...array<String^>^ from);
		/// <summary>
		/// Creates a new proj object from a Well known text definition.
		/// </summary>
		/// <param name="from"></param>
		/// <returns></returns>
		ProjObject^ CreateFromWellKnownText(String^ from);
		/// <summary>
		/// Creates a new proj object from a Well known text definition.
		/// </summary>
		/// <param name="from"></param>
		/// <param name="warnings"></param>
		/// <returns></returns>
		ProjObject^ CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings);
		/// <summary>
		/// Similar to <see cref="Create(String)"/> but follows more strict Proj4 compatibility
		/// </summary>
		/// <param name="definition"></param>
		/// <returns></returns>
		ProjObject^ CreateProj4Compatible(String^ definition);

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
		/// <summary>
		/// Gets or sets the log level
		/// </summary>
		property ProjLogLevel LogLevel
		{
			ProjLogLevel get()
			{
				return m_logLevel;
			}
			void set(ProjLogLevel value)
			{
				m_logLevel = value;
				proj_log_level(this, (PJ_LOG_LEVEL)value);
			}
		}

		event System::Action<ProjLogLevel, String^>^ Log;

	protected:
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
			System::Version^ get();
		}

		/// <summary>ESRI Version. As stored in Proj Database</summary>
		property System::Version^ EsriVersion
		{
			System::Version^ get();
		}

		/// <summary>IGNF Version. As stored in Proj Database</summary>
		property System::Version^ IgnfVersion
		{
			System::Version^ get();
		}

		/// <summary>
		/// Gets all <see cref="CoordinateReferenceSystem"/>s matching the filter
		/// </summary>
		/// <param name="filter"></param>
		/// <returns></returns>
		System::Collections::ObjectModel::ReadOnlyCollection<CoordinateReferenceSystemInfo^>^ GetCoordinateReferenceSystems(CoordinateReferenceSystemFilter^ filter);
		System::Collections::ObjectModel::ReadOnlyCollection<CoordinateReferenceSystemInfo^>^ GetCoordinateReferenceSystems();

	protected:
		virtual void OnLog(ProjLogLevel level, String^ message)
		{
			Log(level, message);
		}

	private:
		static String^ EnvCombine(String^ envVar, String^ file);

	internal:
		static void DownloadProjDB(String^ toPath);
		static operator PJ_CONTEXT* (ProjContext^ me)
		{
			if ((Object^)me == nullptr)
				return nullptr;
			else if (me->m_disposed || !me->m_ctx)
				throw gcnew System::ObjectDisposedException("Context already disposed");

			return me->m_ctx;
		}

		System::Exception^ ConstructException(String^ prefix);
		System::Exception^ ConstructException()
		{
			return ConstructException(nullptr);
		}
		ProjException^ CreateException(int err, String^ message, System::Exception^ inner);

	private:
		void Release()
		{
			int n = System::Threading::Interlocked::Decrement(m_nRefs);
			if (!n)
				NoMoreReferences();
		}
		ProjContext^ AddRef()
		{
			System::Threading::Interlocked::Increment(m_nRefs);
			return this;
		}

	internal:
		ref class CtxHolder sealed
		{
		private:
			ProjContext^ _pc;

		public:
			CtxHolder(ProjContext^ pc)
			{
				_pc = pc->AddRef();
			}

		private:
			!CtxHolder()
			{
				Release();
			}

			~CtxHolder()
			{
				Release();
			}
		public:
			ProjContext^ Get()
			{
				return _pc;
			}

			void Release()
			{
				auto pc = _pc;
				_pc = nullptr;

				if ((Object^)pc)
					pc->Release();
			}

			static operator ProjContext ^ (CtxHolder^ f)
			{
				return f->_pc;
			}
		};
	};
}
