#include "pch.h"
#include "ProjContext.h"
#include "CoordinateOperation.h"
#include "CoordinateOperationList.h"
#include "CoordinateReferenceSystem.h"
#include "ProjArea.h"
#include "ProjException.h"
using namespace ProjSharp;

CoordinateOperation^ CoordinateOperation::Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, ProjArea ^area, ProjContext^ ctx)
{
	if (!sourceCrs)
		throw gcnew ArgumentNullException("sourceCrs");
	else if (!targetCrs)
		throw gcnew ArgumentNullException("targetCrs");
	else if (!ctx) // After fromCrs
		ctx = sourceCrs->Context;

    auto operation_ctx = proj_create_operation_factory_context(ctx, nullptr);
    if (!operation_ctx) {
        return nullptr;
    }

    if (area) {
        proj_operation_factory_context_set_area_of_interest(
            ctx,
            operation_ctx,
            area->WestLongitude,
            area->SouthLatitude,
            area->EastLongitude,
            area->NorthLatitude);
    }

    proj_operation_factory_context_set_spatial_criterion(
        ctx, operation_ctx, PROJ_SPATIAL_CRITERION_PARTIAL_INTERSECTION);
    proj_operation_factory_context_set_grid_availability_use(
        ctx, operation_ctx,
        proj_context_is_network_enabled(ctx) ?
        PROJ_GRID_AVAILABILITY_KNOWN_AVAILABLE :
        PROJ_GRID_AVAILABILITY_DISCARD_OPERATION_IF_MISSING_GRID);

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

    if (P == nullptr || op_count == 1 || (area) ||
        sourceCrs->Type == ProjType::GeocentricCrs ||
        targetCrs->Type == ProjType::GeocentricCrs) {
        proj_list_destroy(op_list);
        return static_cast<CoordinateOperation^>(ctx->Create(P));
    }

    return gcnew CoordinateOperationList(ctx, P, op_list);
}

double CoordinateOperation::RoundTrip(bool forward, int transforms, array<double>^ coordinate)
{
    PJ_COORD coord;
    SetCoordinate(coord, coordinate);

    return proj_roundtrip(this, forward ? PJ_FWD : PJ_INV, transforms, &coord);
}

CoordinateOperationFactors^ CoordinateOperation::Factors(array<double>^ coordinate)
{
    PJ_COORD coord;
    SetCoordinate(coord, coordinate);

    PJ_FACTORS f = proj_factors(this, coord);

    return gcnew CoordinateOperationFactors(this, &f);
}


array<double>^ CoordinateOperation::DoTransform(bool forward, array<double>^ coordinate)
{
    if (!coordinate)
        throw gcnew ArgumentNullException("coordinate");

	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	coord = proj_trans(this, forward ? PJ_FWD : PJ_INV, coord);

	return FromCoordinate(coord, coordinate->Length);
}

double CoordinateOperation::EllipsoidDistance(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lp_dist(this, coord1, coord2);
}

double CoordinateOperation::EllipsoidDistanceZ(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lpz_dist(this, coord1, coord2);
}


array<double>^ CoordinateOperation::EllipsoidGeod(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	PJ_COORD r = proj_geod(this, coord1, coord2);

	return FromCoordinate(r, 3);
}


CoordinateReferenceSystem^ CoordinateOperation::GetSourceCoordinateReferenceSystem([Optional] ProjContext^ context)
{
    if (!context)
        context = Context;

    PJ* pj = proj_get_source_crs(context, this);

    if (!pj)
        return nullptr;

    return static_cast<CoordinateReferenceSystem^>(context->Create(pj));
}


CoordinateReferenceSystem^ CoordinateOperation::GetTargetCoordinateReferenceSystem([Optional] ProjContext^ context)
{
    if (!context)
        context = Context;

    PJ* pj = proj_get_target_crs(context, this);

    if (!pj)
        return nullptr;

    return static_cast<CoordinateReferenceSystem^>(context->Create(pj));
}
