#include "pch.h"
#include "ProjObject.h"
#include "ProjException.h"
#include "CoordinateOperation.h"
#include "CoordinateReferenceSystem.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "GeographicCoordinateReferenceSystem.h"
#include "PrimeMeridian.h"
#include "ReferenceFrame.h"
#include "DatumEnsamble.h"
#include "ProjDatum.h"
#include "Proj.h"

using namespace ProjSharp;

ProjObject^ ProjContext::Create(String^ definition)
{
	if (String::IsNullOrWhiteSpace(definition))
		throw gcnew ArgumentNullException("definition");

	std::string fromStr = utf8_string(definition);
	PJ* pj = proj_create(this, fromStr.c_str());

	if (!pj)
		throw ConstructException();

	return Create(pj);
}

ProjObject^ ProjContext::Create(...array<String^>^ from)
{
	char** lst = new char* [from->Length + 1];
	for (int i = 0; i < from->Length; i++)
	{
		std::string fromStr = utf8_string(from[i]);
		lst[i] = _strdup(fromStr.c_str());
	}
	lst[from->Length] = 0;

	try
	{
		PJ* pj = proj_create_argv(this, from->Length, lst);

		if (!pj)
			throw ConstructException();

		return Create(pj);
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

ProjObject^ ProjContext::Create(PJ* pj)
{
	if (!pj)
		throw gcnew ArgumentNullException("pj");

	switch ((ProjType)proj_get_type(pj))
	{
	case ProjType::Ellipsoid:
		return gcnew Ellipsoid(this, pj);

	case ProjType::PrimeMeridian:
		return gcnew PrimeMeridian(this, pj);

	case ProjType::GeodeticReferenceFrame:
	case ProjType::DynamicGeodeticReferenceFrame:
	case ProjType::VerticalReferenceFrame:
	case ProjType::DynamicVerticalReferenceFrame:
		return gcnew ReferenceFrame(this, pj);

	case ProjType::DatumEnsamble:
		return gcnew DatumEnsamble(this, pj);

	case ProjType::GeographicCrs: // Never used. Only inherited
	case ProjType::Geographic2DCrs:
	case ProjType::Geographic3DCrs:
		return gcnew GeographicCoordinateReferenceSystem(this, pj);

	case ProjType::CRS: // abstract
	case ProjType::GeodeticCrs:
	case ProjType::GeocentricCrs:

	case ProjType::VerticalCrs:
	case ProjType::ProjectedCrs:
	case ProjType::CompoundCrs:
	case ProjType::TemporalCrs:
	case ProjType::EngineeringCrs:
	case ProjType::BoundCrs:
	case ProjType::OtherCrs:
		return gcnew CoordinateReferenceSystem(this, pj);

	case ProjType::Conversion:
	case ProjType::Transformation:
	case ProjType::ConcatenatedOperation:
	case ProjType::OtherCoordinateOperation:
		return gcnew CoordinateOperation(this, pj);


	case ProjType::TemporalDatum:
	case ProjType::EngineeringDatum:
	case ProjType::ParametricDatum:
		return gcnew ProjDatum(this, pj);

	default:
		CoordinateSystemType cst = (CoordinateSystemType)proj_cs_get_type(this, pj);

		if (cst != CoordinateSystemType::Unknown)
			return gcnew CoordinateSystem(this, pj);

		ClearError();
		return gcnew ProjObject(this, pj);
	}
}

ProjObject^ ProjObject::Create(String^ definition, [Optional]ProjContext^ ctx)
{
	if (!ctx)
		ctx = gcnew ProjContext();

	return ctx->Create(definition);
}

ProjObject^ ProjObject::Create(array<String^>^ from, [Optional]ProjContext^ ctx)
{
	if (!ctx)
		ctx = gcnew ProjContext();

	return ctx->Create(from);
}
