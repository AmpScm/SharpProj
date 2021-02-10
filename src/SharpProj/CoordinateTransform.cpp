#include "pch.h"
#include "ProjContext.h"
#include "CoordinateTransform.h"
#include "AnyCoordinateTransform.h"
#include "CoordinateReferenceSystem.h"
#include "CoordinateSystem.h"
#include "CoordinateArea.h"
#include "ProjException.h"

using namespace SharpProj;

SharpProj::CoordinateTransform::~CoordinateTransform()
{
	if ((Object^)m_source)
	{
		delete m_source;
		m_source = nullptr;
	}
	if ((Object^)m_target)
	{
		delete m_target;
		m_target = nullptr;
	}
}

CoordinateTransform^ CoordinateTransform::Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateArea^ area, ProjContext^ ctx)
{
	CoordinateTransformOptions^ opts = gcnew CoordinateTransformOptions();
	opts->Area = area;

	return CoordinateTransform::Create(sourceCrs, targetCrs, opts, ctx);
}

CoordinateTransform^ CoordinateTransform::Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateTransformOptions^ options, ProjContext^ ctx)
{
	if (!sourceCrs)
		throw gcnew ArgumentNullException("sourceCrs");
	else if (!targetCrs)
		throw gcnew ArgumentNullException("targetCrs");

	if (!ctx) // After fromCrs
		ctx = sourceCrs->Context;

	if (!options)
		options = gcnew CoordinateTransformOptions();

	std::string s_auth;
	if (!String::IsNullOrEmpty(options->Authority))
		s_auth = utf8_string(options->Authority);

	auto operation_ctx = proj_create_operation_factory_context(ctx, s_auth.size() ? s_auth.c_str() : nullptr);
	if (!operation_ctx) {
		return nullptr;
	}

	proj_operation_factory_context_set_allow_ballpark_transformations(ctx, operation_ctx, !options->NoBallparkConversions);

	if (options->Accuracy.HasValue)
		proj_operation_factory_context_set_desired_accuracy(ctx, operation_ctx, options->Accuracy.Value);

	if (options && options->Area)
	{
		proj_operation_factory_context_set_area_of_interest(
			ctx,
			operation_ctx,
			options->Area->WestLongitude,
			options->Area->SouthLatitude,
			options->Area->EastLongitude,
			options->Area->NorthLatitude);
	}
	else
	{
		proj_operation_factory_context_set_spatial_criterion(
			ctx, operation_ctx,
			PROJ_SPATIAL_CRITERION_PARTIAL_INTERSECTION);
	}

	proj_operation_factory_context_set_grid_availability_use(
		ctx, operation_ctx,
		proj_context_is_network_enabled(ctx)
		? PROJ_GRID_AVAILABILITY_KNOWN_AVAILABLE
		: (options->NoDiscardIfMissing
			? PROJ_GRID_AVAILABILITY_USED_FOR_SORTING
			: PROJ_GRID_AVAILABILITY_DISCARD_OPERATION_IF_MISSING_GRID));


	proj_operation_factory_context_set_use_proj_alternative_grid_names(
		ctx, operation_ctx,
		!options->UsePrimaryGridNames);

	proj_operation_factory_context_set_allow_use_intermediate_crs(
		ctx, operation_ctx,
		(options->IntermediateCrsUsage == IntermediateCrsUsage::Auto
			? PROJ_INTERMEDIATE_CRS_USE_IF_NO_DIRECT_TRANSFORMATION
			: (options->IntermediateCrsUsage == IntermediateCrsUsage::Never
				? PROJ_INTERMEDIATE_CRS_USE_NEVER
				: PROJ_INTERMEDIATE_CRS_USE_IF_NO_DIRECT_TRANSFORMATION)));

	proj_operation_factory_context_set_discard_superseded(
		ctx, operation_ctx,
		!options->UseSuperseded);

	auto op_list = proj_create_operations(ctx, sourceCrs, targetCrs, operation_ctx);
	proj_operation_factory_context_destroy(operation_ctx);

	if (!op_list) {
		return nullptr;
	}

	auto op_count = proj_list_get_count(op_list);
	if (op_count == 0) {
		proj_list_destroy(op_list);

		throw gcnew ProjException("No operation found matching criteria");
	}

	PJ* P = proj_list_get(ctx, op_list, 0);

	if (P == nullptr || op_count == 1 || (options->Area) ||
		sourceCrs->Type == ProjType::GeocentricCrs ||
		targetCrs->Type == ProjType::GeocentricCrs)
	{
		proj_list_destroy(op_list);
		return ctx->Create<CoordinateTransform^>(P);
	}

	return gcnew AnyCoordinateTransform(ctx, P, op_list);
}

double CoordinateTransform::RoundTrip(bool forward, int transforms, ProjCoordinate coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	return proj_roundtrip(this, forward ? PJ_FWD : PJ_INV, transforms, &coord);
}

Details::CoordinateTransformFactors^ CoordinateTransform::Factors(ProjCoordinate coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	PJ_FACTORS f = proj_factors(this, coord);

	return gcnew Details::CoordinateTransformFactors(this, &f);
}

ProjCoordinate CoordinateTransform::DoTransform(bool forward, ProjCoordinate% coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	coord = proj_trans(this, forward ? PJ_FWD : PJ_INV, coord);

	if (double::IsNaN(coord.v[0]))
		throw Context->ConstructException();

	return FromCoordinate(coord, forward);
}

ProjCoordinate CoordinateTransform::FromCoordinate(const PJ_COORD& coord, bool forward)
{
	int axis = 4;

	CoordinateReferenceSystem^ crs = forward ? TargetCRS : SourceCRS;
	axis = crs ? crs->AxisCount : 4;

	return ProjCoordinate(axis, &coord.v[0]);
}

double CoordinateTransform::EllipsoidDistance(ProjCoordinate coordinate1, ProjCoordinate coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lp_dist(this, coord1, coord2);
}

double CoordinateTransform::EllipsoidDistanceZ(ProjCoordinate coordinate1, ProjCoordinate coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lpz_dist(this, coord1, coord2);
}


ProjCoordinate CoordinateTransform::EllipsoidGeod(ProjCoordinate coordinate1, ProjCoordinate coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	PJ_COORD r = proj_geod(this, coord1, coord2);

	return ProjCoordinate(r);
}


CoordinateReferenceSystem^ CoordinateTransform::SourceCRS::get()
{
	if (!m_source)
	{
		PJ* pj = proj_get_source_crs(Context, this);

		if (!pj)
			return nullptr;

		m_source = Context->Create<CoordinateReferenceSystem^>(pj);
	}

	return m_source;
}


CoordinateReferenceSystem^ CoordinateTransform::TargetCRS::get()
{
	if (!m_target)
	{
		PJ* pj = proj_get_target_crs(Context, this);

		if (!pj)
			return nullptr;

		m_target = Context->Create<CoordinateReferenceSystem^>(pj);
	}

	return m_target;
}

void SharpProj::Details::CoordinateTransformParameter::Ensure()
{
	if (!m_name)
	{
		const char* name;
		const char* auth_name;
		const char* code;
		double value;
		const char* value_string;
		double unit_conv_factor;
		const char* unit_name;
		const char* unit_auth_name;
		const char* unit_code;
		const char* unit_category;


		if (proj_coordoperation_get_param(m_op->Context, m_op, m_index,
			&name, &auth_name, &code, &value, &value_string,
			&unit_conv_factor, &unit_name, &unit_auth_name,
			&unit_code, &unit_category))
		{
			m_name = name ? gcnew String(name) : "";
			m_auth_name = auth_name ? gcnew String(auth_name) : nullptr;
			m_code = code ? gcnew String(code) : nullptr;
			m_value = value;
			m_value_string = value_string ? gcnew String(value_string) : nullptr;
			m_unit_conv_factor = unit_conv_factor;
			m_unit_name = unit_name ? gcnew String(unit_name) : nullptr;
			m_unit_auth_name = unit_auth_name ? gcnew String(unit_auth_name) : nullptr;
			m_unit_code = unit_code ? gcnew String(unit_code) : nullptr;
			m_unit_category = unit_category ? gcnew String(unit_category) : nullptr;
		}
	}
}
