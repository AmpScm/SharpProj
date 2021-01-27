#include "pch.h"
#include "ProjContext.h"
#include "CoordinateSystem.h"

using namespace ProjSharp;


CoordinateSystem^ CoordinateSystem::Create(ProjContext^ ctx, String^ from)
{
	if (!ctx)
		throw gcnew ArgumentNullException("ctx");
	else if (String::IsNullOrWhiteSpace(from))
		throw gcnew ArgumentNullException("from");

	std::string fromStr = utf8_string(from);
	PJ* pj = proj_create(ctx, fromStr.c_str());


	if (!pj)
		throw ctx->ConstructException();

	return gcnew CoordinateSystem(ctx, pj);
}


CoordinateSystem^ CoordinateSystem::Create(ProjContext^ ctx, ...array<String^>^ from)
{
	if (!ctx)
		throw gcnew ArgumentNullException("ctx");
	else if (!from)
		throw gcnew ArgumentNullException("from");

	char** lst = new char*[from->Length+1];
	for(int i = 0; i < from->Length; i++)
	{
		std::string fromStr = utf8_string(from[i]);
		lst[i] = strdup(fromStr.c_str());
	}
	lst[from->Length] = 0;
	
	try
	{
		PJ* pj = proj_create_argv(ctx, from->Length, lst);


		if (!pj)
			throw ctx->ConstructException();

		return gcnew CoordinateSystem(ctx, pj);
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

double CoordinateSystem::Distance2D(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lp_dist(this, coord1, coord2);
}

double CoordinateSystem::Distance3D(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lpz_dist(this, coord1, coord2);
}


GeometricCoordinateSystem^ CoordinateSystem::CreateGeometricCoordinateSystem(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_crs_get_geodetic_crs(context, this);

	if (!pj)
		throw context->ConstructException();

	return gcnew GeometricCoordinateSystem(context, pj);
}

CoordinateSystem^ CoordinateSystem::CreateNormalized(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_normalize_for_visualization(context, this);

	if (!pj)
		throw context->ConstructException();

	return gcnew GeometricCoordinateSystem(context, pj);
}
