#pragma once
namespace ProjSharp {
	[System::Diagnostics::DebuggerDisplayAttribute("{Description}")]
	public ref class ProjBase
	{
	protected:
		ProjContext^ m_ctx;
		PJ* m_pj;
		String^ m_infoId;
		String^ m_infoDescription;
		String^ m_infoDefinition;

	private:
		~ProjBase()
		{
			if (m_pj)
			{
				proj_destroy(m_pj);
			}
		}

	internal:
		ProjBase(ProjContext^ ctx, PJ* pj)
		{
			m_ctx = ctx;
			m_pj = pj;
		}

		static operator PJ* (ProjBase^ pj)
		{
			if ((Object^)pj == nullptr)
				return nullptr;				
			else if(pj->m_pj == nullptr)
				throw gcnew ObjectDisposedException("PJ disposed");

			return pj->m_pj;
		}

	public:
		virtual String^ ToString() override
		{
			return Description;
		}

	public:		
		property String^ Id
		{
			String^ get()
			{
				if (!m_infoId)
				{
					PJ_PROJ_INFO info = proj_pj_info(this);
					m_infoId = gcnew System::String(info.id);
				}
				return m_infoId;
			}
		}

		property String^ Description
		{
			String^ get()
			{
				if (!m_infoDescription)
				{
					PJ_PROJ_INFO info = proj_pj_info(this);
					m_infoDescription = gcnew System::String(info.description);
				}
				return m_infoDescription;
			}
		}

		property String^ Definition
		{
			String^ get()
			{
				if (!m_infoDefinition)
				{
					PJ_PROJ_INFO info = proj_pj_info(this);
					m_infoDefinition = gcnew System::String(info.definition);
				}
				return m_infoDefinition;
			}
		}

		property bool HasInverse
		{
			bool get()
			{
				PJ_PROJ_INFO info = proj_pj_info(this);
				return info.has_inverse;
			}
		}

		property double Accuraracy
		{
			double get()
			{
				PJ_PROJ_INFO info = proj_pj_info(this);
				return info.accuracy;
			}
		}
	};

}