#include "pch.h"
#include "ProjObject.h"
#include "ProjException.h"
#include "PPoint.h"
#include "CoordinateTransform.h"
#include "CoordinateTransformList.h"
#include "CoordinateReferenceSystem.h"
#include "CoordinateReferenceSystemList.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "GeographicCRS.h"
#include "PrimeMeridian.h"
#include "ReferenceFrame.h"
#include "DatumList.h"
#include "Datum.h"

ProjObject^ ProjContext::Create(String^ definition)
{
	if (String::IsNullOrWhiteSpace(definition))
		throw gcnew ArgumentNullException("definition");

	std::string fromStr = utf8_string(definition);
	PJ* pj = proj_create(this, fromStr.c_str());

	if (!pj)
		throw ConstructException();

	return Create(pj);
}

ProjObject^ ProjContext::Create(...array<String^>^ from)
{
	char** lst = new char* [from->Length + 1];
	for (int i = 0; i < from->Length; i++)
	{
		std::string fromStr = utf8_string(from[i]);
		lst[i] = _strdup(fromStr.c_str());
	}
	lst[from->Length] = 0;

	try
	{
		PJ* pj = proj_create_argv(this, from->Length, lst);

		if (!pj)
			throw ConstructException();

		return Create(pj);
	}
	finally
	{
		for (int i = 0; i < from->Length; i++)
		{
			free(lst[i]);
		}
		delete[] lst;
	}
}

ProjObject^ ProjContext::CreateFromWellKnownText(String^ from)
{
	array<String^>^ wars = nullptr;

	return CreateFromWellKnownText(from, wars);
}

ProjObject^ ProjContext::CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings)
{
	if (String::IsNullOrWhiteSpace(from))
		throw gcnew ArgumentNullException("from");

	PROJ_STRING_LIST wrs = nullptr;
	PROJ_STRING_LIST errs = nullptr;
	const char* options[32] = {};

	std::string fromStr = utf8_string(from);
	PJ* pj = proj_create_from_wkt(this, fromStr.c_str(), options, &wrs, &errs);

	warnings = ProjObject::FromStringList(wrs);
	array<String^>^ errors = ProjObject::FromStringList(errs);

	if (wrs)
		proj_string_list_destroy(wrs);
	if (errs)
		proj_string_list_destroy(errs);

	if (!pj)
	{
		Exception^ ex = ConstructException();
		if (errors && errors->Length)
		{
			Exception^ ex2 = (ex && ex->Message->Length) ? ex : nullptr;

			for each (String ^ msg in errors)
			{
				ex2 = gcnew ProjException(msg, ex2);
			}

			if (ex2)
				throw ex2;
		}
		throw ex;
	}

	if (!proj_is_crs(pj))
	{
		proj_destroy(pj);
		throw gcnew ProjException(String::Format("'{0}' doesn't describe a coordinate system", from));
	}

	return Create<CoordinateReferenceSystem^>(pj);
}

ProjObject^ ProjContext::Create(PJ* pj)
{
	if (!pj)
		throw gcnew ArgumentNullException("pj");

	switch ((ProjType)proj_get_type(pj))
	{
	case ProjType::Ellipsoid:
		return gcnew Ellipsoid(this, pj);

	case ProjType::PrimeMeridian:
		return gcnew Proj::PrimeMeridian(this, pj);

	case ProjType::GeodeticReferenceFrame:
	case ProjType::DynamicGeodeticReferenceFrame:
	case ProjType::VerticalReferenceFrame:
	case ProjType::DynamicVerticalReferenceFrame:
		return gcnew Proj::ReferenceFrame(this, pj);

	case ProjType::DatumEnsamble:
		return gcnew Proj::DatumList(this, pj);

	case ProjType::GeographicCrs: // Never used. Only inherited
	case ProjType::Geographic2DCrs:
	case ProjType::Geographic3DCrs:
		return gcnew GeographicCRS(this, pj);

	case ProjType::GeodeticCrs:
	case ProjType::GeocentricCrs:
		return gcnew GeodeticCRS(this, pj);
	
	case ProjType::CompoundCrs:
		return gcnew CoordinateReferenceSystemList(this, pj);

	case ProjType::CRS: // abstract
	case ProjType::VerticalCrs:
	case ProjType::ProjectedCrs:
	case ProjType::TemporalCrs:
	case ProjType::EngineeringCrs:	
	case ProjType::OtherCrs:
		return gcnew CoordinateReferenceSystem(this, pj);

	case ProjType::BoundCrs:
		return gcnew BoundCRS(this, pj);

	case ProjType::Conversion:
	case ProjType::Transformation:
	case ProjType::OtherCoordinateTransform:
		return gcnew CoordinateTransform(this, pj);

	case ProjType::ConcatenatedOperation:
		return gcnew CoordinateTransformList(this, pj);

	case ProjType::TemporalDatum:
	case ProjType::EngineeringDatum:
	case ProjType::ParametricDatum:
		return gcnew Proj::Datum(this, pj);

	case ProjType::ChooseTransform:
	case ProjType::CoordinateSystem:
		throw gcnew InvalidOperationException(); // Never returned by proj, but needed for sensible API

	case ProjType::Unknown:		
	default:
		CoordinateSystemType cst = (CoordinateSystemType)proj_cs_get_type(this, pj);

		if (cst != CoordinateSystemType::Unknown)
			return gcnew CoordinateSystem(this, pj);

		ClearError(pj);


		if (!strcmp(proj_get_name(pj), "Transformation pipeline manager"))
		{
			return gcnew CoordinateTransform(this, pj);
		}

		return gcnew ProjObject(this, pj);
	}
}

generic<typename T> where T : ProjObject
T ProjContext::Create(PJ* pj)
{
	ProjObject^ o = Create(pj);

	return safe_cast<T>(o);
}

ProjObject^ ProjObject::Create(String^ definition, [Optional]ProjContext^ ctx)
{
	if (!ctx)
		ctx = gcnew ProjContext();

	return ctx->Create(definition);
}

ProjObject^ ProjObject::Create(array<String^>^ from, [Optional]ProjContext^ ctx)
{
	if (!ctx)
		ctx = gcnew ProjContext();

	return ctx->Create(from);
}

ProjObject^ ProjObject::CreateFromWellKnownText(String^ from, [Optional] ProjContext^ ctx)
{
	if (!ctx)
		ctx = gcnew ProjContext();

	return ctx->CreateFromWellKnownText(from);
}

ProjObject^ ProjObject::CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx)
{
	if (!ctx)
		ctx = gcnew ProjContext();

	return ctx->CreateFromWellKnownText(from, warnings);
}

IdentifierList^ ProjObject::Identifiers::get()
{
	if (!m_idList && proj_get_id_auth_name(this, 0))
		m_idList = gcnew IdentifierList(this);

	return m_idList;
}

Identifier^ IdentifierList::default::get(int index)
{
	if (index < 0 || m_Items ? (index >= m_Items->Length) : !proj_get_id_auth_name(m_object, index))
		throw gcnew IndexOutOfRangeException();

	if (m_Items)
	{
		if (!m_Items[index])
			m_Items[index] = gcnew Identifier(m_object, index);

		return m_Items[index];
	}
	else
	{
		// Count still unknown
		return gcnew Identifier(m_object, index);
	}
}

int IdentifierList::Count::get()
{
	if (m_Items)
		return m_Items->Length;

	for (int i = 0; i < 256 /* some MAX */; i++)
	{
		if (!proj_get_id_auth_name(m_object, i))
		{
			m_Items = gcnew array<Identifier^>(i);
			return i;
		}
	}
	m_Items = Array::Empty<Identifier^>();
	return 0;
}

System::Collections::Generic::IEnumerator<Identifier^>^ IdentifierList::GetEnumerator()
{
	for(int i = 0; i < Count /* initializes m_Items if necessary */; i++)
	{
		if (!m_Items[i])
			m_Items[i] = gcnew Identifier(m_object, i);
	}

	return static_cast<System::Collections::Generic::IEnumerable<Identifier^>^>(m_Items)->GetEnumerator();
}

String^ Identifier::Authority::get()
{
	if (!m_authority)
	{
		const char* auth = proj_get_id_auth_name(m_object, m_index);

		m_authority = auth ? Utf8_PtrToString(auth) : nullptr;
	}
	return m_authority;
}

String^ Identifier::Name::get()
{
	if (!m_code)
	{
		const char* code = proj_get_id_code(m_object, m_index);

		m_code = Utf8_PtrToString(code);
	}
	return m_code;
}


CoordinateTransform^ UsageArea::GetLatLonConvert()
{
	if (!m_latLonTransform)
	{
		CoordinateReferenceSystem^ crs = dynamic_cast<CoordinateReferenceSystem^>(m_obj);

		if (crs)
		{			
			try
			{
				CoordinateReferenceSystem^ wgs84 = CoordinateReferenceSystem::Create("EPSG:4326", crs->Context)->WithAxisNormalized(crs->Context);
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

double UsageArea::MinY::get()
{
	if (!m_minY.HasValue)
	{
		m_minY = Math::Min(Math::Min(NorthEastCorner.Y, NorthWestCorner.Y), Math::Min(SouthEastCorner.Y, SouthWestCorner.Y));

		if (m_minY.HasValue)
		{
			if (double::IsNaN(m_minY.Value))
				m_minY = Nullable<double>();
			else
			{
				auto ll = GetLatLonConvert();

				if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
					m_minY = CoordinateTransform::ApplyAccuracy(m_minY.Value, ll->Accuraracy.Value);
			}
		}
	}

	return m_minY.Value;
}

double UsageArea::MaxY::get()
{
	if (!m_maxY.HasValue)
	{
		m_maxY = Math::Max(Math::Max(NorthEastCorner.Y, NorthWestCorner.Y), Math::Max(SouthEastCorner.Y, SouthWestCorner.Y));

		if (m_maxY.HasValue)
		{
			if (double::IsNaN(m_maxY.Value))
				m_maxY = Nullable<double>();
			else
			{
				auto ll = GetLatLonConvert();

				if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
					m_maxY = CoordinateTransform::ApplyAccuracy(m_maxY.Value, ll->Accuraracy.Value);
			}
		}
	}

	return m_maxY.Value;
}

double UsageArea::MinX::get()
{
	if (!m_minX.HasValue)
	{
		m_minX = Math::Min(Math::Min(NorthEastCorner.X, NorthWestCorner.X), Math::Min(SouthEastCorner.X, SouthWestCorner.X));

		if (m_minX.HasValue)
		{
			if (double::IsNaN(m_minX.Value))
				m_minX = Nullable<double>();
			else
			{
				auto ll = GetLatLonConvert();

				if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
					m_minX = CoordinateTransform::ApplyAccuracy(m_minX.Value, ll->Accuraracy.Value);
			}
		}
	}

	return m_minX.Value;
}

double UsageArea::MaxX::get()
{
	if (!m_maxX.HasValue)
	{
		m_maxX = Math::Max(Math::Max(NorthEastCorner.X, NorthWestCorner.X), Math::Max(SouthEastCorner.X, SouthWestCorner.X));

		if (m_maxX.HasValue)
		{
			if (double::IsNaN(m_maxX.Value))
				m_maxX = Nullable<double>();
			else
			{
				auto ll = GetLatLonConvert();

				if (ll->Accuraracy.HasValue && ll->Accuraracy.Value > 0)
					m_maxX = CoordinateTransform::ApplyAccuracy(m_maxX.Value, ll->Accuraracy.Value);
			}
		}
	}

	return m_maxX.Value;
}
