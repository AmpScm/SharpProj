#include "pch.h"
#include "CoordinateMetadata.h"
#include "CoordinateReferenceSystem.h"

using namespace SharpProj;
using namespace SharpProj::Proj;

CoordinateMetadata::~CoordinateMetadata()
{
    DisposeIfNotNull(m_crs);
}

CoordinateReferenceSystem^ CoordinateMetadata::CRS::get()
{
    if (!m_crs && this)
    {
        PJ* pj = proj_get_source_crs(Context, this);

        if (!pj)
        {
            Context->ClearError(this);
            return nullptr;
        }
        else
            m_crs = Context->Create<CoordinateReferenceSystem^>(pj);
    }
    return m_crs;
}