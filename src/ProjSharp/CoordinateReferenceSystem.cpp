#include "pch.h"
#include "ProjContext.h"
#include "ProjException.h"
#include "CoordinateReferenceSystem.h"
#include "DatumEnsamble.h"
#include "CoordinateOperation.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "PrimeMeridian.h"
#include "ProjDatum.h"


using namespace ProjSharp;


CoordinateReferenceSystem^ CoordinateReferenceSystem::Create(ProjContext^ ctx, String^ from)
{
	if (!ctx)
		throw gcnew ArgumentNullException("ctx");
	else if (String::IsNullOrWhiteSpace(from))
		throw gcnew ArgumentNullException("from");

	std::string fromStr = utf8_string(from);
	PJ* pj = proj_create(ctx, fromStr.c_str());

	if (!pj)
		throw ctx->ConstructException();


	if (!proj_is_crs(pj))
	{
		proj_destroy(pj);
		throw gcnew ProjException(String::Format("'{0}' doesn't describe a coordinate system", from));
	}

	return static_cast<CoordinateReferenceSystem^>(ctx->Create(pj));
}


CoordinateReferenceSystem^ CoordinateReferenceSystem::Create(ProjContext^ ctx, ...array<String^>^ from)
{
	if (!ctx)
		throw gcnew ArgumentNullException("ctx");
	else if (!from)
		throw gcnew ArgumentNullException("from");

	char** lst = new char*[from->Length+1];
	for(int i = 0; i < from->Length; i++)
	{
		std::string fromStr = utf8_string(from[i]);
		lst[i] = _strdup(fromStr.c_str());
	}
	lst[from->Length] = 0;
	
	try
	{
		PJ* pj = proj_create_argv(ctx, from->Length, lst);


		if (!pj)
			throw ctx->ConstructException();

		return static_cast<CoordinateReferenceSystem^>(ctx->Create(pj));
	}
	finally
	{
		for (int i = 0; i < from->Length; i++)
		{
			free(lst[i]);
		}
		delete[] lst;
	}
}

double ProjObject::Distance2D(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lp_dist(this, coord1, coord2);
}

double ProjObject::Distance3D(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lpz_dist(this, coord1, coord2);
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::GetGeodeticCoordinateReferenceSystem(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_crs_get_geodetic_crs(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<CoordinateReferenceSystem^>(context->Create(pj));
}

ProjDatum^ CoordinateReferenceSystem::GetHorizontalDatum(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError();
	PJ* pj = proj_crs_get_horizontal_datum(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<ProjDatum^>(context->Create(pj));
}

ProjDatum^ CoordinateReferenceSystem::GetDatum(ProjContext^ context)
{
	if (!context)
		context = Context;


	context->ClearError();
	PJ* pj = proj_crs_get_datum(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<ProjDatum^>(context->Create(pj));
}

DatumEnsamble^ CoordinateReferenceSystem::GetDatumEnsamble(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError();
	proj_errno_reset(this);
	PJ* pj = proj_crs_get_datum_ensemble(context, this);

	if (!pj)
	{
		if (proj_context_errno(context) == 0)
			return nullptr;
		
		throw context->ConstructException();
	}

	return static_cast<DatumEnsamble^>(context->Create(pj));
}

ProjDatum^ CoordinateReferenceSystem::GetDatumForced(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError();
	PJ* pj = proj_crs_get_datum_forced(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<ProjDatum^>(context->Create(pj));
}

CoordinateSystem^ CoordinateReferenceSystem::GetCoordinateSystem(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError();
	PJ* pj = proj_crs_get_coordinate_system(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<CoordinateSystem^>(context->Create(pj));
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::GetNormalized(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_normalize_for_visualization(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<CoordinateReferenceSystem^>(context->Create(pj));
}

Ellipsoid^ CoordinateReferenceSystem::GetEllipsoid(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_get_ellipsoid(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<Ellipsoid^>(context->Create(pj));
}

PrimeMeridian^ CoordinateReferenceSystem::GetPrimeMeridian(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_get_prime_meridian(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<PrimeMeridian^>(context->Create(pj));
}

CoordinateOperation^ CoordinateReferenceSystem::GetCoordinateOperation(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_crs_get_coordoperation(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<CoordinateOperation^>(context->Create(pj));
}
