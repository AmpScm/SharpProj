#include "pch.h"
#include "ProjContext.h"
#include "CoordinateTransform.h"
#include "CoordinateSystem.h"
#include "ProjArea.h"
using namespace ProjSharp;

CoordinateTransform^ CoordinateTransform::Create(CoordinateSystem^ sourceCrs, CoordinateSystem^ targetCrs)
{
	return CoordinateTransform::Create((sourceCrs != nullptr) ? sourceCrs->Context : nullptr, sourceCrs, targetCrs, (ProjArea ^)nullptr);
}

CoordinateTransform^ CoordinateTransform::Create(ProjContext ^ctx, CoordinateSystem^ sourceCrs, CoordinateSystem^ targetCrs, ProjArea ^area)
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
		if (area)
			proj_area_set_bbox(p_area, area->WestLongitude, area->SouthLatitude, area->EastLongitude, area->NorthLatitude);

		PJ* p = proj_create_crs_to_crs_from_pj(ctx, sourceCrs, targetCrs, p_area, nullptr /* NULL required for now */);

		if (!p)
			throw ctx->ConstructException();

		return gcnew CoordinateTransform(ctx, p);
	}
	finally
	{
		proj_area_destroy(p_area);
	}
}


array<double>^ CoordinateTransform::Transform(array<double>^ coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	coord = proj_trans(this, PJ_FWD, coord);

	return FromCoordinate(coord, coordinate->Length);
}

array<double>^ CoordinateTransform::InverseTransform(array<double>^ coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	coord = proj_trans(this, PJ_INV, coord);

	return FromCoordinate(coord, coordinate->Length);
}
