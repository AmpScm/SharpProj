#include "pch.h"
#include "ProjObject.h"
#include "CoordinateTransform.h"
#include "UsageArea.h"

CoordinateTransform^ UsageArea::GetLatLonConvert()
{
	if (!m_latLonTransform)
	{
		CoordinateReferenceSystem^ crs = dynamic_cast<CoordinateReferenceSystem^>(m_obj);

		if (crs)
		{
			try
			{
				CoordinateReferenceSystem^ wgs84 = CoordinateReferenceSystem::CreateFromEpsg(4326, crs->Context)->WithAxisNormalized(crs->Context);
				m_latLonTransform = CoordinateTransform::Create(crs, wgs84, crs->Context);
			}
			catch (ProjException^)
			{
				m_latLonTransform = crs->DistanceTransform; // Not the expected ellipsoid
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

double UsageArea::MinY::get()
{
	if (!m_minY.HasValue)
	{
		double minY = Math::Min(Math::Min(NorthEastCorner.Y, NorthWestCorner.Y), Math::Min(SouthEastCorner.Y, SouthWestCorner.Y));

		if (double::IsNaN(minY) || double::IsInfinity(minY))
			m_minY = double::NaN;
		else
		{
			auto ll = GetLatLonConvert();

			if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
				m_minY = CoordinateTransform::ApplyAccuracy(minY, ll->Accuraracy.Value);
			else
				m_minY = double::NaN;
		}
	}

	return m_minY.Value;
}

double UsageArea::MaxY::get()
{
	if (!m_maxY.HasValue)
	{
		double maxY = Math::Max(Math::Max(NorthEastCorner.Y, NorthWestCorner.Y), Math::Max(SouthEastCorner.Y, SouthWestCorner.Y));

		if (double::IsNaN(maxY) || double::IsInfinity(maxY))
			m_maxY = double::NaN;
		else
		{
			auto ll = GetLatLonConvert();

			if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
				m_maxY = CoordinateTransform::ApplyAccuracy(maxY, ll->Accuraracy.Value);
			else
				m_maxY = double::NaN;
		}
	}

	return m_maxY.Value;
}

double UsageArea::MinX::get()
{
	if (!m_minX.HasValue)
	{
		double minX = Math::Min(Math::Min(NorthEastCorner.X, NorthWestCorner.X), Math::Min(SouthEastCorner.X, SouthWestCorner.X));

		if (double::IsNaN(minX) || double::IsInfinity(minX))
			m_minX = double::NaN;
		else
		{
			auto ll = GetLatLonConvert();

			if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
				m_minX = CoordinateTransform::ApplyAccuracy(minX, ll->Accuraracy.Value);
			else
				m_minX = double::NaN;
		}
	}

	return m_minX.Value;
}

double UsageArea::MaxX::get()
{
	if (!m_maxX.HasValue)
	{
		double maxX = Math::Max(Math::Max(NorthEastCorner.X, NorthWestCorner.X), Math::Max(SouthEastCorner.X, SouthWestCorner.X));

		if (double::IsNaN(maxX) || double::IsInfinity(maxX))
			m_maxX = double::NaN;
		else
		{
			auto ll = GetLatLonConvert();

			if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
				m_maxX = CoordinateTransform::ApplyAccuracy(maxX, ll->Accuraracy.Value);
			else
				m_maxX = double::NaN;
		}
	}

	return m_maxX.Value;
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
