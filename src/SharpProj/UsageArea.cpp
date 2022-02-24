#include "pch.h"
#include "ProjObject.h"
#include "CoordinateTransform.h"
#include "GeographicCRS.h"
#include "UsageArea.h"

void UsageArea::InternalDispose()
{
    DisposeIfNotNull(m_latLonTransform);
}

CoordinateTransform^ UsageArea::GetLatLonConvert()
{
    if (!m_latLonTransform)
    {
        CoordinateReferenceSystem^ crs = dynamic_cast<CoordinateReferenceSystem^>(m_obj);
        CoordinateTransform^ ct;

        if (!crs && (ct = dynamic_cast<CoordinateTransform^>(m_obj)) && ct->SourceCRS)
            crs = ct->SourceCRS;

        if (crs && crs->Type != ProjType::VerticalCrs)
        {
            // We need a CRS to convert the Lat/Lon coordinates from. Theoretically we should probably use WGS84, but the extent
            // is usually only set in just a few decimal places, so we take the more efficient Geodetic CRS of the system itself for
            // the conversion as that doesn't go through all kinds of loops to optimize precision (e.g. gridshifts)
            //
            // (See WKT specification on OpenGeoSpatial.org)

            // Let's just use the already cached distance transform if possible
            m_latLonTransform = crs->DistanceTransform;

            if (!m_latLonTransform || !m_latLonTransform->HasInverse)
            {
                // Ok, then we fall back to the WGS84 definition for the coordinate conversion

                CoordinateReferenceSystem^ wgs84 = CoordinateReferenceSystem::CreateFromEpsg(4326, crs->Context)->WithNormalizedAxis(nullptr);
                m_latLonTransform = CoordinateTransform::Create(crs, wgs84, crs->Context);

                if (!m_latLonTransform->HasInverse)
                    m_latLonTransform = nullptr;

                delete wgs84;
            }
        }
    }

    return m_latLonTransform;
}

SharpProj::PPoint UsageArea::NorthWestCorner::get()
{
    if (!m_NW.HasValue)
    {
        CoordinateTransform^ t = GetLatLonConvert();

        if (t)
            m_NW = t->ApplyReversed(PPoint(WestLongitude, NorthLatitude));
        else
            m_NW = PPoint(double::NaN, double::NaN);
    }
    return m_NW.Value;
}

SharpProj::PPoint UsageArea::SouthEastCorner::get()
{
    if (!m_SE.HasValue)
    {
        CoordinateTransform^ t = GetLatLonConvert();

        if (t)
            m_SE = t->ApplyReversed(PPoint(EastLongitude, SouthLatitude));
        else
            m_SE = PPoint(double::NaN, double::NaN);
    }
    return m_SE.Value;
}

SharpProj::PPoint UsageArea::SouthWestCorner::get()
{
    if (!m_SW.HasValue)
    {
        CoordinateTransform^ t = GetLatLonConvert();

        if (t)
            m_SW = t->ApplyReversed(PPoint(WestLongitude, SouthLatitude));
        else
            m_SW = PPoint(double::NaN, double::NaN);
    }
    return m_SW.Value;
}

SharpProj::PPoint UsageArea::NorthEastCorner::get()
{
    if (!m_NE.HasValue)
    {
        CoordinateTransform^ t = GetLatLonConvert();

        if (t)
            m_NE = t->ApplyReversed(PPoint(EastLongitude, NorthLatitude));
        else
            m_NE = PPoint(double::NaN, double::NaN);
    }
    return m_NE.Value;
}

SharpProj::PPoint UsageArea::Center::get()
{
    return PPoint((MinX + MaxX) / 2.0, (MinY + MaxY) / 2.0);
}

void UsageArea::CalculateBounds()
{
    if (m_hasMinMax)
        return;

    m_hasMinMax = true;

    m_minX = Double::NaN;
    m_minY = Double::NaN;
    m_maxX = Double::NaN;
    m_maxY = Double::NaN;

    auto llc = UsageArea::GetLatLonConvert();

    double xmin, ymin, xmax, ymax;

    if (llc && proj_trans_bounds(llc->Context, llc, PJ_INV, WestLongitude, SouthLatitude, EastLongitude, NorthLatitude,
        &xmin, &ymin, &xmax, &ymax, 21))
    {
        m_minX = xmin;
        m_minY = ymin;
        m_maxX = xmax;
        m_maxY = ymax;
    }
}

double UsageArea::MinY::get()
{
    CalculateBounds();

    return m_minY;
}

double UsageArea::MaxY::get()
{
    CalculateBounds();

    return m_maxY;
}

double UsageArea::MinX::get()
{
    CalculateBounds();

    return m_minX;
}

double UsageArea::MaxX::get()
{
    CalculateBounds();

    return m_maxX;
}

double UsageArea::CenterX::get()
{
    if (!double::IsNaN(MinX) && !double::IsNaN(MaxX))
        return (MinX + MaxX) / 2;
    else
        return double::NaN;
}

double UsageArea::CenterY::get()
{
    if (!double::IsNaN(MinY) && !double::IsNaN(MaxY))
        return (MinY + MaxY) / 2;
    else
        return double::NaN;
}
