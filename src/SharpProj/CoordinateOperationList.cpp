#include "pch.h"
#include "CoordinateOperationList.h"
//#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
//#include <proj_api.h>

using namespace SharpProj;

using System::Collections::Generic::IEnumerable;

int CoordinateOperationList::SuggestedOperation(ProjCoordinate coordinate)
{
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	return proj_get_suggested_operation(Context, m_list, PJ_FWD, coord);
}

ProjCoordinate CoordinateOperationList::DoTransform(bool forward, ProjCoordinate% coordinate)
{
	PJ_DIRECTION dir = forward ? PJ_FWD : PJ_INV;
	PJ_COORD coord;
	SetCoordinate(coord, coordinate);

	constexpr int N_MAX_RETRY = 2;
	int iExcluded[N_MAX_RETRY] = { -1, -1 };

	const int nOperations = Count;

	// We may need several attempts. For example the point at
	// lon=-111.5 lat=45.26 falls into the bounding box of the Canadian
	// ntv2_0.gsb grid, except that it is not in any of the subgrids, being
	// in the US. We thus need another retry that will select the conus
	// grid.
	for (int iRetry = 0; iRetry <= N_MAX_RETRY; iRetry++)
	{
		// Do a first pass and select the operations that match the area of use
		// and has the best accuracy.
		int iBest = proj_get_suggested_operation(Context, m_list, dir, coord);
		if (iBest < 0) {
			break;
		}

		CoordinateOperation^ c = this[iBest];

		if (!ReferenceEquals(c, m_last))
		{
			if (Context->LogLevel >= ProjLogLevel::Debug)
			{
				Context->OnLogMessage(ProjLogLevel::Debug, "Using coordinate operation " + c->Description);
			}
			m_last = c;
		}
		PJ_COORD res = proj_trans(c, dir, coord);

		if (proj_errno(c) == -62 /*PJD_ERR_NETWORK_ERROR*/) {
			return FromCoordinate(res);
		}
		if (res.xyzt.x != HUGE_VAL) {
			return FromCoordinate(res);
		}
		Context->OnLogMessage(ProjLogLevel::Debug, "Did not result in valid result. Attempting a retry with another operation.");
		if (iRetry == N_MAX_RETRY) {
			break;
		}
		iExcluded[iRetry] = iBest;
	}

	// In case we did not find an operation whose area of use is compatible
	// with the input coordinate, then goes through again the list, and
	// use the first operation that does not require grids.
	for (int i = 0; i < nOperations; i++)
	{
		CoordinateOperation^ c = this[i];

		if (c->GridUsageCount == 0)
		{
			if (!ReferenceEquals(c, m_last))
			{
				if (Context->LogLevel >= ProjLogLevel::Debug)
				{
					Context->OnLogMessage(ProjLogLevel::Debug, "Using coordinate operation " + c->Description);
				}
				m_last = c;
			}

			PJ_COORD res = proj_trans(c, dir, coord);
			return FromCoordinate(res);
		}
	}

	throw gcnew InvalidOperationException("No usable transform found");
}
