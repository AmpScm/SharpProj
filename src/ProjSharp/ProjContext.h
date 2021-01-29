#pragma once

using namespace System;

namespace ProjSharp {
	ref class ProjObject;

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

		ProjContext(PJ_CONTEXT *ctx)
		{
			m_ctx = ctx;
		}

	internal:
		String^ m_lastError;

	public:
		ProjContext();

		~ProjContext()
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
		}

		ProjContext^ Clone()
		{
			return gcnew ProjContext(proj_context_clone(this));
		}

	protected public:
		void OnFindFile(String^ file, [Out] String^% foundFile);
		void OnLogMessage(ProjLogLevel level, String^ message);

	public:
		ProjObject^ Create(String^ definition);
		ProjObject^ Create(...array<String^>^ from);

	internal:
		ProjObject^ Create(PJ* pj);
		void ClearError()
		{
			m_lastError = nullptr;
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
