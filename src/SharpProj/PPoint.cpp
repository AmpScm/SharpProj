#include "pch.h"
#include "PPoint.h"
#include "CoordinateTransform.h"

using namespace SharpProj;

PPoint SharpProj::PPoint::DegToRad()
{
	PPoint pc = PPoint(
		CoordinateTransform::ToRad(X),
		CoordinateTransform::ToRad(Y),
		Z,
		T);
	pc.Axis = Axis;
	return pc;
}

PPoint SharpProj::PPoint::RadToDeg()
{
	PPoint pc = PPoint(
		CoordinateTransform::ToDeg(X),
		CoordinateTransform::ToDeg(Y),
		Z,
		T);
	pc.Axis = Axis;
	return pc;
}
