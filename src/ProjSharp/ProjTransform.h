#pragma once
#include "ProjBase.h"

namespace ProjSharp {
	ref class ProjCrs;
	ref class ProjArea;

	public ref class ProjTransform : ProjBase
	{
	private:
		ProjTransform(ProjContext^ ctx, PJ* pj)
			: ProjBase(ctx, pj)
		{


		}


	public:
		static ProjTransform^ Create(ProjContext ^ctx, ProjCrs^ sourceCrs, ProjCrs^ targetCrs, ProjArea ^area);
		static ProjTransform^ Create(ProjCrs^ sourceCrs, ProjCrs^ targetCrs);
	};

}