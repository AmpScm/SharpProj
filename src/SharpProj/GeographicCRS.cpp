#include "pch.h"
#include "GeographicCRS.h"

using namespace SharpProj;

CoordinateReferenceSystem^ BoundCRS::HubCRS::get()
{
	if (!m_hubCrs && this)
	{
		Context->ClearError(this);
		PJ* pj = proj_get_source_crs(Context, this);

		if (!pj)
			throw Context->ConstructException();

		m_hubCrs = Context->Create<CoordinateReferenceSystem^>(pj);
	}
	return m_hubCrs;
}
