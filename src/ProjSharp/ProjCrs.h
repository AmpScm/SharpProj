#pragma once
#include "ProjBase.h"

namespace ProjSharp {
	public ref class ProjCrs : ProjBase
	{
	private:
		ProjContext^ m_ctx;

		~ProjCrs()
		{

		}

	private:
		ProjCrs(ProjContext^ ctx, PJ* pj)
			: ProjBase(ctx, pj)
		{

		}

	public:
		static ProjCrs^ Create(ProjContext^ ctx, String^ from);
		static ProjCrs^ Create(ProjContext^ ctx, ...array<String^>^ from);

	};
}