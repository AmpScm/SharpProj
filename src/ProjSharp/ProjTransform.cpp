#include "pch.h"
#include "ProjContext.h"
#include "ProjTransform.h"
#include "ProjCrs.h"
using namespace ProjSharp;

ProjTransform^ ProjTransform::Create(ProjCrs^ sourceCrs, ProjCrs^ targetCrs)
{
	return Create(sourceCrs ? sourceCrs->Context : nullptr, sourceCrs, targetCrs, (ProjArea ^)nullptr);
}

ProjTransform^ ProjTransform::Create(ProjContext ^ctx, ProjCrs^ sourceCrs, ProjCrs^ targetCrs, ProjArea ^area)
{
	if (!sourceCrs)
		throw gcnew ArgumentNullException("sourceCrs");
	else if (!targetCrs)
		throw gcnew ArgumentNullException("targetCrs");
	else if (!ctx) // After fromCrs
		throw gcnew ArgumentNullException("ctx");

	PJ_AREA* p_area = proj_area_create();
	try
	{
		PJ* p = proj_create_crs_to_crs_from_pj(ctx, sourceCrs, targetCrs, p_area, nullptr /* NULL required for now */);

		if (!p)
			throw ctx->ConstructException();

		return gcnew ProjTransform(ctx, p);
	}
	finally
	{
		proj_area_destroy(p_area);
	}
}
