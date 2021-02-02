#include "pch.h"
#include "Ellipsoid.h"

using namespace ProjSharp;

double Ellipsoid::EllipsoidDistance(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lp_dist(this, coord1, coord2);
}

double Ellipsoid::EllipsoidDistanceZ(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lpz_dist(this, coord1, coord2);
}


array<double>^ Ellipsoid::EllipsoidGeod(array<double>^ coordinate1, array<double>^ coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	PJ_COORD r = proj_geod(this, coord1, coord2);

	return FromCoordinate(r, 3);
}
