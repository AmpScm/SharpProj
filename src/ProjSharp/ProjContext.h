#pragma once

using namespace System;

namespace ProjSharp {
	public ref class ProjContext
	{
	private:
		PJ_CONTEXT* m_ctx;
		gcroot<WeakReference<ProjContext^>^>* m_ref;

		ProjContext(PJ_CONTEXT *ctx)
		{
			m_ctx = ctx;
		}

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

	internal:
		static operator PJ_CONTEXT* (ProjContext^ me)
		{
			if (!me->m_ctx)
				throw gcnew System::ObjectDisposedException("Context already disposed");

			return me->m_ctx;
		}

		System::Exception^ ConstructException();
	};
}
