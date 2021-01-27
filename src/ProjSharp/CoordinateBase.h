#pragma once
#include "ProjContext.h"

namespace ProjSharp {

	[System::Diagnostics::DebuggerDisplayAttribute("{Description}")]
	public ref class CoordinateBase
	{
	protected:
		ProjContext^ m_ctx;
		PJ* m_pj;
		String^ m_infoId;
		String^ m_infoDescription;
		String^ m_infoDefinition;

	private:
		~CoordinateBase()
		{
			if (m_pj)
			{
				proj_destroy(m_pj);
			}
		}

	internal:
		CoordinateBase(ProjContext^ ctx, PJ* pj)
		{
			m_ctx = ctx;
			m_pj = pj;
		}

		static operator PJ* (CoordinateBase^ pj)
		{
			if ((Object^)pj == nullptr)
				return nullptr;
			else if (pj->m_pj == nullptr)
				throw gcnew ObjectDisposedException("PJ disposed");

			return pj->m_pj;
		}

	public:
		virtual String^ ToString() override
		{
			return Description;
		}

	public:
		property ProjContext^ Context
		{
			ProjContext^ get()
			{
				return m_ctx;
			}
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

	private protected:
		void SetCoordinate(PJ_COORD& coord, array<double>^ coordinate)
		{
			coord.v[0] = coordinate[0];
			coord.v[1] = coordinate[1];
			if (coordinate->Length > 2)
			{
				coord.v[2] = coordinate[2];
				if (coordinate->Length > 3)
					coord.v[3] = coordinate[3];
			}
		}

		array<double>^ FromCoordinate(const PJ_COORD& coord, int len)
		{
			array<double>^ r = gcnew array<double>(len);
			r[0] = coord.v[0];
			r[1] = coord.v[1];
			if (r->Length > 2)
			{
				r[2] = coord.v[2];
				if (r->Length > 3)
					r[3] = coord.v[3];
			}

			return r;
		}
	};
}