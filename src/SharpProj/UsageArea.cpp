#include "pch.h"
#include "ProjObject.h"
#include "CoordinateTransform.h"
#include "GeographicCRS.h"
#include "UsageArea.h"

CoordinateTransform^ UsageArea::GetLatLonConvert()
{
	if (!m_latLonTransform)
	{
		CoordinateReferenceSystem^ crs = dynamic_cast<CoordinateReferenceSystem^>(m_obj);

		if (crs && crs->Type != ProjType::VerticalCrs)
		{
			// We need a CRS to convert the Lat/Lon coordinates from. Theoretically we should probably use WGS84, but the extent
			// is usually only set in just a few decimal places, so we take the more efficient Geodetic CRS of the system itself for
			// the conversion as that doesn't go through all kinds of loops to optimize precision (e.g. gridshifts)
			//
			// (See WKT specification on OpenGeoSpatial.org)

			// Let's just use the already cached distance transform if possible
			m_latLonTransform = crs->DistanceTransform;

			if (!m_latLonTransform)
			{
				// Ok, then we fall back to the WGS84 definition for the coordinate conversion

				CoordinateReferenceSystem^ wgs84 = CoordinateReferenceSystem::CreateFromEpsg(4326, crs->Context)->WithAxisNormalized(nullptr);
				m_latLonTransform = CoordinateTransform::Create(crs, wgs84, crs->Context);
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

void UsageArea::EnsureMinMax()
{
	if (m_hasMinMax)
		return;

	m_hasMinMax = true;

	m_minX = Double::NaN;
	m_minY = Double::NaN;
	m_maxX = Double::NaN;
	m_maxY = Double::NaN;

	auto llc = UsageArea::GetLatLonConvert();

	if (llc)
	{
		const int steps = 21;
		auto x = gcnew array<double>(steps * 4);
		auto y = gcnew array<double>(steps * 4);

		double east_step;

		if (m_eastLongitude >= m_westLongitude)
			east_step = (m_eastLongitude - m_westLongitude) / (steps - 1);
		else
			east_step = (m_eastLongitude + 360.0 - m_westLongitude) / (steps - 1);

		for (int j = 0; j < steps; j++)
		{
			double test_lon = m_westLongitude + j * east_step;

			if (test_lon > 180.0)
				test_lon -= 360.0;

			x[j] = test_lon;
			y[j] = m_southLatitude;
			x[steps + j] = test_lon;
			y[steps + j] = m_northLatitude;
			x[steps * 2 + j] = m_westLongitude;
			y[steps * 2 + j] = m_southLatitude + j * (m_northLatitude - m_southLatitude) / 20;
			x[steps * 3 + j] = m_eastLongitude;
			y[steps * 3 + j] = m_southLatitude + j * (m_northLatitude - m_southLatitude) / 20;
		}
		{
			pin_ptr<double> px = &x[0];
			pin_ptr<double> py = &y[0];
			llc->ApplyReversed(
				px, 1, x->Length,
				py, 1, y->Length,
				nullptr, 0, 0,
				nullptr, 0, 0);
		}
		m_minX = Double::PositiveInfinity;
		m_minY = Double::PositiveInfinity;
		m_maxX = Double::NegativeInfinity;
		m_maxY = Double::NegativeInfinity;
		for (int j = 0; j < 21 * 4; j++)
		{
			if (x[j] != HUGE_VAL && y[j] != HUGE_VAL)
			{
				m_minX = Math::Min(m_minX, x[j]);
				m_minY = Math::Min(m_minY, y[j]);
				m_maxX = Math::Max(m_maxX, x[j]);
				m_maxY = Math::Max(m_maxY, y[j]);
			}
		}
	}
}

double UsageArea::MinY::get()
{
	EnsureMinMax();

	return m_minY;
}

double UsageArea::MaxY::get()
{
	EnsureMinMax();

	return m_maxY;
}

double UsageArea::MinX::get()
{
	EnsureMinMax();

	return m_minX;
}

double UsageArea::MaxX::get()
{
	EnsureMinMax();

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
