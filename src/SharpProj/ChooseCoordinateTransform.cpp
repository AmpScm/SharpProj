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

		if (proj_errno(c) == PROJ_ERR_OTHER_NETWORK_ERROR)
		{
			throw c->Context->ConstructException("Choose transform failed");
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

void ChooseCoordinateTransform::DoTransform(bool forward,
	double* xVals, int xStep, int xCount,
	double* yVals, int yStep, int yCount,
	double* zVals, int zStep, int zCount,
	double* tVals, int tStep, int tCount)
{
	PPoint p;
    int i;
    int nmin;
    double null_broadcast = 0;
	double invalid_time = HUGE_VAL;

    /* ignore lengths of null arrays */
    if (!xVals || xCount < 0 || xStep < 0) xCount = 0;
    if (!yVals || yCount < 0 || yStep < 0) yCount = 0;
    if (!zVals || zCount < 0 || zStep < 0) zCount = 0;
    if (!tVals || tCount < 0 || tStep < 0) tCount = 0;

    /* and make the nullities point to some real world memory for broadcasting nulls */
    if (0 == xCount) xVals = &null_broadcast;
    if (0 == yCount) yVals = &null_broadcast;
    if (0 == zCount) zVals = &null_broadcast;
    if (0 == tCount) tVals = &invalid_time;

    /* nothing to do? */
    if (0 == xCount + yCount + zCount + tCount)
        return;

    /* arrays of length 1 are constants, which we broadcast along the longer arrays */
    /* so we need to find the length of the shortest non-unity array to figure out  */
    /* how many coordinate pairs we must transform */
    nmin = (xCount > 1) ? xCount : (yCount > 1) ? yCount : (zCount > 1) ? zCount : (tCount > 1) ? tCount : 1;
    if ((xCount > 1) && (xCount < nmin))  nmin = xCount;
    if ((yCount > 1) && (yCount < nmin))  nmin = yCount;
    if ((zCount > 1) && (zCount < nmin))  nmin = zCount;
    if ((tCount > 1) && (tCount < nmin))  nmin = tCount;  

    /* Arrays of length==0 are broadcast as the constant 0               */
    /* Arrays of length==1 are broadcast as their single value           */
    /* Arrays of length >1 are iterated over (for the first nmin values) */
    /* The slightly convolved incremental indexing is used due           */
    /* to the stride, which may be any size supported by the platform    */
    for (i = 0; i < nmin; i++) {
        p.X = *xVals;
        p.Y = *yVals;
        p.Z = *zVals;
        p.T = *tVals;

		p = DoTransform(forward, p);

        /* in all full length cases, we overwrite the input with the output,  */
        /* and step on to the next element.                                   */
        /* The casts are somewhat funky, but they compile down to no-ops and  */
        /* they tell compilers and static analyzers that we know what we do   */
        if (xCount > 1) {
            *xVals = p.X;
			xVals += xStep;
        }
        if (yCount > 1) {
            *yVals = p.Y;
            yVals += yStep;
        }
        if (zCount > 1) {
            *zVals = p.Z;
            zVals += zStep;
        }
        if (tCount > 1) {
			*tVals = p.T;
			tVals += tStep;
        }
    }

    /* Last time around, we update the length 1 cases with their transformed alter egos */
    if (xCount == 1)
        *xVals = p.X;
    if (yCount == 1)
        *yVals = p.Y;
    if (zCount == 1)
        *zVals = p.Z;
    if (tCount == 1)
        *tVals = p.T;
}
