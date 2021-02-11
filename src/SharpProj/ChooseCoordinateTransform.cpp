#include "pch.h"
#include "ChooseCoordinateTransform.h"
#include "ProjException.h"

using namespace SharpProj;

using System::Collections::Generic::IEnumerable;

int ChooseCoordinateTransform::SuggestedOperation(PPoint coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	return proj_get_suggested_operation(Context, m_list, PJ_FWD, coord);
}

PPoint ChooseCoordinateTransform::DoTransform(bool forward, PPoint% coordinate)
{
	PJ_DIRECTION dir = forward ? PJ_FWD : PJ_INV;
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	constexpr int N_MAX_RETRY = 2;

	const int nOperations = Count;

	// We may need several attempts. For example the point at
	// lon=-111.5 lat=45.26 falls into the bounding box of the Canadian
	// ntv2_0.gsb grid, except that it is not in any of the subgrids, being
	// in the US. We thus need another retry that will select the conus
	// grid.

	// Do a first pass and select the operations that match the area of use
	// and has the best accuracy.
	int iBest = proj_get_suggested_operation(Context, m_list, dir, coord);

	if (iBest >= 0)
	{
		CoordinateTransform^ c = this[iBest];

		if (!ReferenceEquals(c, m_last))
		{
			if (Context->LogLevel >= ProjLogLevel::Debug)
			{
				Context->OnLogMessage(ProjLogLevel::Debug, "Using coordinate operation " + c->Name);
			}
			m_last = c;
		}
		c->Context->ClearError(c);
		PJ_COORD res = proj_trans(c, dir, coord);

		if (proj_errno(c) == -62 /*PJD_ERR_NETWORK_ERROR*/) 
		{
			throw c->Context->ConstructException();
		}
		else if (res.xyzt.x != HUGE_VAL) 
		{
			// Success
			return c->FromCoordinate(res, forward);
		}

		Context->OnLogMessage(ProjLogLevel::Debug, "Did not result in valid result. Attempting a retry with another operation.");
	}

	// If the best operation fails, just try them all in turn
	for (int i = 0; i < nOperations; i++)
	{
		if (i == iBest)
			continue; // Don't retry same op

		CoordinateTransform^ c = this[i];

		if (!ReferenceEquals(c, m_last))
		{
			if (Context->LogLevel >= ProjLogLevel::Debug)
			{
				Context->OnLogMessage(ProjLogLevel::Debug, "Using coordinate operation " + c->Name);
			}
			m_last = c;
		}

		PJ_COORD res = proj_trans(c, dir, coord);
		if (res.xyzt.x != HUGE_VAL)
		{
			// Success
			return c->FromCoordinate(res, forward);
		}

	}

	throw gcnew ProjException("No usable transform found");
}
