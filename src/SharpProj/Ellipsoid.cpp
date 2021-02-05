#include "pch.h"
#include "Ellipsoid.h"

using namespace SharpProj;

double Ellipsoid::EllipsoidDistance(ProjCoordinate coordinate1, ProjCoordinate coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lp_dist(this, coord1, coord2);
}

double Ellipsoid::EllipsoidDistanceZ(ProjCoordinate coordinate1, ProjCoordinate coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	return proj_lpz_dist(this, coord1, coord2);
}


ProjCoordinate Ellipsoid::EllipsoidGeod(ProjCoordinate coordinate1, ProjCoordinate coordinate2)
{
	PJ_COORD coord1, coord2;
	SetCoordinate(coord1, coordinate1);
	SetCoordinate(coord2, coordinate2);

	PJ_COORD r = proj_geod(this, coord1, coord2);

	return FromCoordinate(r);
}
