#include "pch.h"
#include "ReferenceFrame.h"
#include "Ellipsoid.h"


Ellipsoid^ ReferenceFrame::Ellipsoid::get()
{
	if (!m_ellipsoid && this)
	{
		if (Type == ProjType::GeodeticReferenceFrame || Type == ProjType::GeodeticReferenceFrame)
		{
			PJ* pj = proj_get_ellipsoid(Context, this);

			if (!pj)
			{
				Context->ClearError(this);
				return nullptr;
			}

			m_ellipsoid = Context->Create<ProjDetaile::Ellipsoid^>(pj);
		}
	}

	return m_ellipsoid;
}
