#include "pch.h"
#include "ProjCoordinate.h"
#include "CoordinateOperation.h"

using namespace SharpProj;

ProjCoordinate SharpProj::ProjCoordinate::Transform(CoordinateOperation^ operation)
{
	if (!operation)
		throw gcnew ArgumentNullException("operation");
	return operation->Transform(*this);
}
