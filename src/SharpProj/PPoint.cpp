#include "pch.h"
#include "PPoint.h"
#include "CoordinateTransform.h"

using namespace SharpProj;

System::String^ PPoint::ToString()
{
	using System::Globalization::CultureInfo;
	String^ v;
	switch (Axis)
	{
	case 1:
		v = String::Format(CultureInfo::InvariantCulture, "X={0}", X);
		break;
	case 2:
		v = String::Format(CultureInfo::InvariantCulture, "X={0}, Y={1}", X, Y);
		break;
	case 3:
	default:
		v = String::Format(CultureInfo::InvariantCulture, "X={0}, Y={1}, Z={2}", X, Y, Z);
	}

	if (HasT)
		return String::Format("{0}, T={1}", v, T);
	else
		return v;
}

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
