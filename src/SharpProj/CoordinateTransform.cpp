#include "pch.h"
#include <geodesic.h>

#include "ProjContext.h"
#include "CoordinateTransform.h"
#include "ChooseCoordinateTransform.h"
#include "CoordinateReferenceSystem.h"
#include "CoordinateSystem.h"
#include "CoordinateArea.h"
#include "ProjException.h"
#include "Ellipsoid.h"
#include "GridUsage.h"

CoordinateTransform::CoordinateTransform(ProjContext^ ctx, PJ* pj)
	: ProjObject(ctx, pj)
{


}

CoordinateTransform::~CoordinateTransform()
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
	if (m_pgeod)
	{
		delete m_pgeod;
		m_pgeod = nullptr;
	}
}

ProjObject^ SharpProj::CoordinateTransform::DoClone(ProjContext^ ctx)
{
	auto t = static_cast<CoordinateTransform^>(__super::DoClone(ctx));

	t->m_methodName = m_methodName;
	t->m_distanceFlags = m_distanceFlags;

	if (m_pgeod && !t->m_pgeod)
	{
		t->m_pgeod = new struct geod_geodesic;
		*t->m_pgeod = *m_pgeod;
	}
	return t;
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

	if (options->Accuracy.HasValue && options->Accuracy.Value >= 0.0)
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

	proj_operation_factory_context_set_spatial_criterion(
		ctx, operation_ctx,
		(options && options->StrictContains)
		? PROJ_SPATIAL_CRITERION_STRICT_CONTAINMENT
		: PROJ_SPATIAL_CRITERION_PARTIAL_INTERSECTION);

	proj_operation_factory_context_set_grid_availability_use(
		ctx, operation_ctx,
		ctx->EnableNetworkConnections
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
		throw gcnew ProjException("Failed to obtain operations");
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

		if (!P)
			throw ctx->ConstructException();

		return ctx->Create<CoordinateTransform^>(P);
	}

	return gcnew ChooseCoordinateTransform(ctx, P, op_list);
}

CoordinateTransform^ CoordinateTransform::Create(String^ from, ProjContext^ ctx)
{
	ProjObject^ po = ProjObject::Create(from, ctx);

	CoordinateTransform^ ct = dynamic_cast<CoordinateTransform^>(po);
	if (ct)
		return ct;

	throw gcnew ProjException("Proj Object is no CoordinateTransform");
}

CoordinateTransform^ CoordinateTransform::Create(array<String^>^ definition, ProjContext^ ctx)
{
	ProjObject^ po = ProjObject::Create(definition, ctx);

	CoordinateTransform^ ct = dynamic_cast<CoordinateTransform^>(po);
	if (ct)
		return ct;

	throw gcnew ProjException("Proj Object is no CoordinateTransform");
}

CoordinateTransform^ CoordinateTransform::CreateFromDatabase(String^ authority, String^ code, ProjContext^ ctx)
{
	if (String::IsNullOrWhiteSpace(authority))
		throw gcnew ArgumentNullException("authority");
	else if (String::IsNullOrWhiteSpace(code))
		throw gcnew ArgumentNullException("code");

	if (!ctx)
		ctx = gcnew ProjContext();

	std::string authStr = utf8_string(authority);
	std::string codeStr = utf8_string(code);
	PJ* pj = proj_create_from_database(ctx, authStr.c_str(), codeStr.c_str(), PJ_CATEGORY_COORDINATE_OPERATION, false, nullptr);

	if (pj)
		return ctx->Create<CoordinateTransform^>(pj);
	else
		throw ctx->ConstructException();
}


double CoordinateTransform::RoundTrip(bool forward, int transforms, PPoint coordinate)
{
	if (transforms < 1)
		throw gcnew ArgumentOutOfRangeException("transforms", "n should be >= 1");

	PPoint coord = coordinate; // Holds value after each roundtrip

	for (int i = 0; i < transforms; i++)
	{
		coord = DoTransform(!forward, DoTransform(forward, coord)); // Do what SharpProj does for ChooseCoordinateTransorm, etc.
	}

	// checking for angular/degree *input* since we do a roundtrip, and end where we begin
	if (proj_angular_input(this, forward ? PJ_FWD : PJ_INV) || proj_degree_input(this, forward ? PJ_FWD : PJ_INV))
		return GeoDistanceZ(coordinate, coord);
	else
	{
		PJ_COORD pj_org, pj_coord;

		SetCoordinate(pj_org, coordinate);
		SetCoordinate(pj_coord, coord);

		return proj_xyz_dist(pj_org, pj_coord);
	}
}

Proj::CoordinateTransformFactors^ CoordinateTransform::Factors(PPoint coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	PJ_FACTORS f = proj_factors(this, coord);

	return gcnew Proj::CoordinateTransformFactors(this, &f);
}

PPoint CoordinateTransform::DoTransform(bool forward, PPoint% coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	coord = proj_trans(this, forward ? PJ_FWD : PJ_INV, coord);

	if (double::IsNaN(coord.v[0]))
		throw Context->ConstructException("Transform failed; Check Coordinates");

	return FromCoordinate(coord, forward);
}

void CoordinateTransform::DoTransform(bool forward,
	double* xVals, int xStep, int xCount,
	double* yVals, int yStep, int yCount,
	double* zVals, int zStep, int zCount,
	double* tVals, int tStep, int tCount)
{
	proj_trans_generic(this, forward ? PJ_FWD : PJ_INV,
		xVals, xStep * sizeof(double), xCount,
		yVals, yStep * sizeof(double), yCount,
		zVals, zStep * sizeof(double), zCount,
		tVals, tStep * sizeof(double), tCount);
}

void CoordinateTransform::Apply(
	double* xVals, int xStep, int xCount,
	double* yVals, int yStep, int yCount,
	double* zVals, int zStep, int zCount,
	double* tVals, int tStep, int tCount)
{
	DoTransform(true,
		xVals, xStep, xCount,
		yVals, yStep, yCount,
		zVals, zStep, zCount,
		tVals, tStep, tCount);
}

void CoordinateTransform::ApplyReversed(
	double* xVals, int xStep, int xCount,
	double* yVals, int yStep, int yCount,
	double* zVals, int zStep, int zCount,
	double* tVals, int tStep, int tCount)
{
	DoTransform(false,
		xVals, xStep, xCount,
		yVals, yStep, yCount,
		zVals, zStep, zCount,
		tVals, tStep, tCount);
}


PPoint CoordinateTransform::FromCoordinate(const PJ_COORD& coord, bool forward)
{
	CoordinateReferenceSystem^ crs = forward ? TargetCRS : SourceCRS;
	if (crs)
		return PPoint(crs->AxisCount, coord);
	else
		return PPoint(coord);
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

void SharpProj::Proj::CoordinateTransformParameter::Ensure()
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
			m_name = Utf8_PtrToString(name);
			m_auth_name = Utf8_PtrToString(auth_name);
			m_code = Utf8_PtrToString(code);
			m_value = value;
			m_value_string = Utf8_PtrToString(value_string);
			m_unit_conv_factor = unit_conv_factor;
			m_unit_name = Utf8_PtrToString(unit_name);
			m_unit_auth_name = Utf8_PtrToString(unit_auth_name);
			m_unit_code = Utf8_PtrToString(unit_code);
			m_unit_category = Utf8_PtrToString(unit_category);
		}
	}
}

enum DistanceFlags
{
	None = 0,
	Setup = 1,
	ApplyTransform = 2,
	SwapXY = 4,
	ApplyRad = 8
};

void CoordinateTransform::SetupDistance()
{
	int d = DistanceFlags::Setup;

	d |= DistanceFlags::ApplyTransform;

	auto axis = this->TargetCRS->Axis;

	if (axis && axis->Count)
	{
		String^ abbr = axis[0]->Abbreviation;

		if (abbr == "Lat")
			d |= DistanceFlags::SwapXY | DistanceFlags::ApplyRad;
		else if (abbr == "Lon")
			d |= DistanceFlags::ApplyRad;
	}

	m_distanceFlags = d;

	if (!m_pgeod && TargetCRS)
	{
		auto el = TargetCRS->Ellipsoid;
		if (el)
		{
			m_pgeod = new struct geod_geodesic;

			geod_init(m_pgeod, el->SemiMajorMetre, 1.0 / el->InverseFlattening);
		}
	}
}

double CoordinateTransform::GeoDistance(PPoint p1, PPoint p2)
{
	return GeoDistance(gcnew array<PPoint>{p1, p2});
}

double CoordinateTransform::GeoDistance(System::Collections::Generic::IEnumerable<PPoint>^ points)
{
	if (!points)
		throw gcnew ArgumentNullException("points");

	EnsureDistance();

	if (!m_pgeod)
		return double::PositiveInfinity; // Like distance methods

	bool applyTransform = (m_distanceFlags & DistanceFlags::ApplyTransform);
	bool swapXY = (m_distanceFlags & DistanceFlags::SwapXY);
	bool applyToRad = (m_distanceFlags & DistanceFlags::ApplyRad);

	double ll1[2] = {};
	double ll2[2] = {};
	bool first = true;

	double size = 0;

	for each (PPoint p in points)
	{
		if (applyTransform)
			p = Apply(p);

		ll2[0] = swapXY ? p.X : p.Y;
		ll2[1] = swapXY ? p.Y : p.X;

		if (!applyToRad)
		{
			ll2[0] = ToDeg(ll2[0]);
			ll2[1] = ToDeg(ll2[1]);
		}

		if (first)
			first = false;
		else
		{
			double s12, azi1, azi2;
			/* Note: the geodesic code takes arguments in degrees */

			geod_inverse(m_pgeod, ll1[0], ll1[1], ll2[0], ll2[1], &s12, &azi1, &azi2);

			size += s12;
		}

		memcpy(&ll1, &ll2, sizeof(ll1));
	}

	return size;
}

double CoordinateTransform::GeoDistanceZ(PPoint p1, PPoint p2)
{
	return GeoDistanceZ(gcnew array<PPoint>{ p1, p2 });
}

double CoordinateTransform::GeoDistanceZ(System::Collections::Generic::IEnumerable<PPoint>^ points)
{
	if (!points)
		throw gcnew ArgumentNullException("points");

	EnsureDistance();

	if (!m_pgeod) // Can be null
		return double::PositiveInfinity; // Like distance methods

	bool applyTransform = (m_distanceFlags & DistanceFlags::ApplyTransform);
	bool swapXY = (m_distanceFlags & DistanceFlags::SwapXY);
	bool applyToRad = (m_distanceFlags & DistanceFlags::ApplyRad);

	double ll1[3] = {};
	double ll2[3] = {};
	bool first = true;

	double size = 0;

	for each (PPoint p in points)
	{
		if (applyTransform)
			p = Apply(p);

		ll2[0] = swapXY ? p.X : p.Y;
		ll2[1] = swapXY ? p.Y : p.X;
		ll2[2] = p.Z;

		if (!applyToRad)
		{
			ll2[0] = ToDeg(ll2[0]);
			ll2[1] = ToDeg(ll2[1]);
		}

		if (first)
			first = false;
		else
		{
			double s12, azi1, azi2;
			/* Note: the geodesic code takes arguments in degrees */

			geod_inverse(m_pgeod, ll1[0], ll1[1], ll2[0], ll2[1], &s12, &azi1, &azi2);

			size += hypot(s12, ll1[2] - ll2[2]);
		}

		memcpy(&ll1, &ll2, sizeof(ll1));
	}

	return size;
}


PPoint CoordinateTransform::Geod(PPoint p1, PPoint p2)
{
	EnsureDistance();


	if (m_distanceFlags & DistanceFlags::ApplyTransform)
	{
		p1 = Apply(p1);
		p2 = Apply(p2);
	}
	if (m_distanceFlags & DistanceFlags::ApplyRad)
	{
		p1 = p1.DegToRad();
		p2 = p2.DegToRad();
	}

	PJ_COORD coord1 = {}, coord2 = {};
	coord1.xyz.x = p1.X;
	coord1.xyz.y = p1.Y;
	coord1.xyz.z = p1.Z;
	coord2.xyz.x = p2.X;
	coord2.xyz.y = p2.Y;
	coord2.xyz.z = p2.Z;

	if (m_distanceFlags & DistanceFlags::SwapXY)
	{
		std::swap(coord1.xy.x, coord1.xy.y);
		std::swap(coord2.xy.x, coord2.xy.y);
	}

	PJ_COORD r = proj_geod(this, coord1, coord2);

	return PPoint(r);
}

double CoordinateTransform::GeoArea(System::Collections::Generic::IEnumerable<PPoint>^ points)
{
	if (!points)
		throw gcnew ArgumentNullException("points");

	EnsureDistance();

	if (!m_pgeod) // Can be null
		return double::PositiveInfinity; // Like distance methods

	bool applyTransform = (m_distanceFlags & DistanceFlags::ApplyTransform);
	bool swapXY = (m_distanceFlags & DistanceFlags::SwapXY);
	bool applyToRad = (m_distanceFlags & DistanceFlags::ApplyRad);

	struct geod_polygon poly;
	geod_polygon_init(&poly, false);

	for each (PPoint p in points)
	{
		if (applyTransform)
			p = Apply(p);

		if (!applyToRad)
			p = p.RadToDeg();

		geod_polygon_addpoint(m_pgeod, &poly,
			swapXY ? p.X : p.Y,
			swapXY ? p.Y : p.X);
	}

	double poly_area;
	double perim_area;
	geod_polygon_compute(m_pgeod, &poly, true /* clockwise = positive */, true /* sign */, &poly_area, &perim_area);

	return poly_area;
}

ReadOnlyCollection<GridUsage^>^ CoordinateTransform::GridUsages::get()
{
	if (!m_gridUsages)
	{
		int n = proj_coordoperation_get_grid_used_count(Context, this);

		array<GridUsage^>^ usages;

		if (n == 0)
			usages = Array::Empty<GridUsage^>();
		else
		{
			usages = gcnew array<GridUsage^>(n);

			for (int i = 0; i < n; i++)
				usages[i] = gcnew GridUsage(this, i);
		}
		m_gridUsages = Array::AsReadOnly(usages);
	}
	return m_gridUsages;
}
