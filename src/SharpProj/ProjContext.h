#pragma once


#if 1
#define EMPTY_ARRAY(T) gcnew array<T> {}
#else
#define EMPTY_ARRAY(T) Array::Empty<T>()
#endif

using namespace System;

namespace SharpProj {
    ref class ProjException;
    ref class CoordinateReferenceSystem;

    namespace Proj {
        ref class ProjObject;
        ref class CoordinateReferenceSystemFilter;
        ref class CoordinateReferenceSystemInfo;
        ref class CelestialBodyInfo;
    }

    public enum class ProjLogLevel
    {
        None = PJ_LOG_NONE,
        Error = PJ_LOG_ERROR,
        Debug = PJ_LOG_DEBUG,
        Trace = PJ_LOG_TRACE
    };

    template<typename TCtx, typename TNetCtx> class ctx_wrapper
    {
        TCtx* m_handle;
        int m_cnt;
        bool m_meDisposed;
        TCtx* (*m_close_handle)(TCtx* h);
        gcroot<WeakReference<TNetCtx^>^> m_netRef;

        const int CTX_Pressure = 2 * 1024 * 1024; // Guess 2 MB memory pressure outside .Net

    public:
        ctx_wrapper(TCtx* value, TCtx* (*close_handle)(TCtx* h))
        {
            m_handle = value;
            m_close_handle = close_handle;
            m_cnt = 1;
            m_meDisposed = false;
            m_netRef = gcnew WeakReference<TNetCtx^>(nullptr);
            GC::AddMemoryPressure(CTX_Pressure);
        }

    private:
        ~ctx_wrapper()
        {
            if (!m_handle)
                return;

            auto h = m_handle;
            m_handle = nullptr;
            m_close_handle(h);

            GC::RemoveMemoryPressure(CTX_Pressure);
        }
    public:
        __inline operator TCtx* () const
        {
            return this ? m_handle : nullptr;
        }

        __inline void AddRef()
        {
            System::Threading::Interlocked::Increment(m_cnt);
        }

        __inline void Release()
        {
            if (!System::Threading::Interlocked::Decrement(m_cnt))
                delete this;
        }

        __inline void ReleaseMe()
        {
            if (m_meDisposed)
                throw gcnew InvalidOperationException();
            m_meDisposed = true;
            Release();
        }

    public:
        bool TryGetTarget(TNetCtx^% target) const
        {
            if (m_meDisposed)
            {
                target = nullptr;
                return false;
            }

            return m_netRef->TryGetTarget(target);
        }
    
        void SetTarget(TNetCtx^ target)
        {
            m_netRef->SetTarget(target);
        }
    };

    template<typename TCtx, typename TNetCtx, typename TItem> class item_wrapper
    {
        TItem* m_handle;
        int m_cnt;
        ctx_wrapper<TCtx, TNetCtx>* m_ctx;
        TItem* (*m_close_handle)(TItem* h);

    public:
        item_wrapper(ctx_wrapper<TCtx, TNetCtx>* ctx, TItem* value, TItem* (*close_handle)(TItem* h))
        {
            m_ctx = ctx;
            m_handle = value;
            m_close_handle = close_handle;
            m_cnt = 1;
            m_ctx->AddRef();
        }

    private:
        ~item_wrapper()
        {
            if (!m_handle)
                return;

            auto h = m_handle;
            m_handle = nullptr;
            m_close_handle(h);
            m_ctx->Release();
            m_ctx = nullptr;
        }

    public:
        __inline void AddRef()
        {
            System::Threading::Interlocked::Increment(m_cnt);
        }

        __inline void Release()
        {
            if (!System::Threading::Interlocked::Decrement(m_cnt))
                delete this;
        }
    public:
        __inline operator TItem* () const
        {
            return this ? m_handle : nullptr;
        }
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
        ctx_wrapper<PJ_CONTEXT, ProjContext>& m_ctx;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        void* m_chain;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        String^ m_lastError;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        ProjLogLevel m_logLevel;

        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        static array<String^>^ _projLibDirs;

        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        bool m_disposed;
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
        [Obsolete("Unused since Proj 8.1")]
        [EditorBrowsableAttribute(EditorBrowsableState::Never)]
        property bool AutoCloseSession
        {
            bool get() { return false; }
            void set(bool value) { UNUSED_ALWAYS(value); }
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
        static property System::Version^ ProjVersion
        {
            System::Version^ get()
            {
                return gcnew System::Version(PROJ_VERSION_MAJOR, PROJ_VERSION_MINOR, PROJ_VERSION_PATCH);
            }
        }

        /// <summary>PROJ Version, wrapped by SharpProj</summary>
        property System::Version^ Version
        {
            System::Version^ get()
            {
                return ProjVersion;
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

        /// <summary>Version of the PROJ-data package with which this database is the most compatible.</summary>
        property System::Version^ ProjDataVersion
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

        System::Collections::ObjectModel::ReadOnlyCollection<CelestialBodyInfo^>^ GetCelestialBodies();

    protected:
        virtual void OnLog(ProjLogLevel level, String^ message)
        {
            Log(level, message);
        }

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

        static operator ctx_wrapper<PJ_CONTEXT, ProjContext>* (ProjContext^ me)
        {
            if ((Object^)me == nullptr)
                return nullptr;
            else if (me->m_disposed || !me->m_ctx)
                throw gcnew System::ObjectDisposedException("Context already disposed");

            return &me->m_ctx;
        }

        static bool operator ! (ProjContext^ me)
        {
            if ((Object^)me == nullptr)
                return true;
            else if (me->m_disposed || !me->m_ctx)
                return true;

            return false;
        }

        System::Exception^ ConstructException(String^ prefix);
        System::Exception^ ConstructException()
        {
            return ConstructException(nullptr);
        }
        ProjException^ CreateException(int err, String^ message, System::Exception^ inner);
    };
}
