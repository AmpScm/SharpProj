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

			if (!m_latLonTransform || !m_latLonTransform->HasInverse)
			{
				// Ok, then we fall back to the WGS84 definition for the coordinate conversion

				CoordinateReferenceSystem^ wgs84 = CoordinateReferenceSystem::CreateFromEpsg(4326, crs->Context)->WithAxisNormalized(nullptr);
				m_latLonTransform = CoordinateTransform::Create(crs, wgs84, crs->Context);

				if (!m_latLonTransform->HasInverse)
					m_latLonTransform = nullptr;
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

	if (llc)
	{
		const int steps = 21;
		auto x = gcnew array<double>(steps * 4);
		auto y = gcnew array<double>(steps * 4);

		double east_step, north_step;

		if (EastLongitude >= WestLongitude)
			east_step = (EastLongitude - WestLongitude) / (steps - 1);
		else
			east_step = (EastLongitude + 360.0 - WestLongitude) / (steps - 1);

		north_step = (NorthLatitude - SouthLatitude) / (steps - 1);

		for (int j = 0; j < steps; j++)
		{
			double test_lon = WestLongitude + j * east_step;

			if (test_lon > 180.0)
				test_lon -= 360.0;

			x[steps * 0 + j] = test_lon;
			y[steps * 0 + j] = SouthLatitude;
			x[steps * 1 + j] = test_lon;
			y[steps * 1 + j] = NorthLatitude;
			x[steps * 2 + j] = WestLongitude;
			y[steps * 2 + j] = SouthLatitude + j * north_step;
			x[steps * 3 + j] = EastLongitude;
			y[steps * 3 + j] = SouthLatitude + j * north_step;
		}

		if (WestLongitude == -180 && EastLongitude == 180)
		{
			// We project the entire world west->east
			// And we have some duplicated corner points. Let's use these to avoid infinite results in a few cases
			const int dup0 = steps * 2 + 0;
			const int dupA = steps * 0 + 0;
			const int dup1 = steps * 2 + steps - 1;
			const int dupB = steps * 1 + 0;
			const int dup2 = steps * 3 + 0;
			const int dupC = steps * 0 + steps - 1;
			const int dup3 = steps * 3 + steps - 1;
			const int dupD = steps * 1 + steps - 1;


#ifdef _DEBUG
			if (x[dup0] != x[dupA] || y[dup0] != y[dupA] || dup0 == dupA)
				throw gcnew InvalidOperationException("P1");
			if (x[dup1] != x[dupB] || y[dup1] != y[dupB] || dup1 == dupB)
				throw gcnew InvalidOperationException("P2");
			if (x[dup2] != x[dupC] || y[dup2] != y[dupC] || dup2 == dupC)
				throw gcnew InvalidOperationException("P3");
			if (x[dup3] != x[dupD] || y[dup3] != y[dupD] || dup3 == dupD)
				throw gcnew InvalidOperationException("P4");
#endif

			double smallStep = Math::Min(east_step, north_step);

			// Note: We don't correct for overflow at 180/-180 degrees as the
			// bounds are constant in this block.

			x[dup0] = WestLongitude + smallStep;
			y[dup0] = NorthLatitude - smallStep;
			x[dup1] = EastLongitude - smallStep;
			y[dup1] = NorthLatitude - smallStep;
			x[dup2] = WestLongitude + smallStep;
			y[dup2] = SouthLatitude + smallStep;
			x[dup3] = EastLongitude - smallStep;
			y[dup3] = SouthLatitude + smallStep;

			// And replace the one after south and north center points with a point slightly
			// off the center point to also avoid asymptots here
			x[steps * 0 + (steps / 2) + 1] = x[steps * 0 + (steps / 2)];
			y[steps * 0 + (steps / 2) + 1] = y[steps * 0 + (steps / 2)] + smallStep;

			x[steps * 1 + (steps / 2) + 1] = x[steps * 1 + (steps / 2)];
			y[steps * 1 + (steps / 2) + 1] = y[steps * 1 + (steps / 2)] - smallStep;
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

		for (int j = 0; j < x->Length; j++)
		{
			if (!double::IsInfinity(x[j]) && !double::IsInfinity(y[j]))
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
