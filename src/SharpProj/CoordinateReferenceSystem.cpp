#include "pch.h"
#include "ProjContext.h"
#include "ProjException.h"
#include "CoordinateReferenceSystem.h"
#include "GeographicCRS.h"
#include "DatumList.h"
#include "CoordinateTransform.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "PrimeMeridian.h"
#include "Datum.h"


using namespace SharpProj;
using namespace SharpProj::Details;

SharpProj::CoordinateReferenceSystem::~CoordinateReferenceSystem()
{
	if ((Object^)m_cs)
	{
		delete m_cs;
		m_cs = nullptr;
	}

	if ((Object^)m_geodCRS)
	{
		if (!ReferenceEquals(m_geodCRS, this)) // Or endless loop
			delete m_geodCRS;
		m_geodCRS = nullptr;
	}
	if ((Object^)m_ellipsoid)
	{
		delete m_ellipsoid;
		m_ellipsoid = nullptr;
	}
	if ((Object^)m_primeMeridian)
	{
		delete m_primeMeridian;
		m_primeMeridian = nullptr;
	}
	if ((Object^)m_baseCrs)
	{
		delete m_baseCrs;
		m_baseCrs = nullptr;
	}
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::Create(String^ from, ProjContext^ ctx)
{
	if (String::IsNullOrWhiteSpace(from))
		throw gcnew ArgumentNullException("from");

	if (!ctx)
		ctx = gcnew ProjContext();

	std::string fromStr = utf8_string(from);
	PJ* pj = proj_create(ctx, fromStr.c_str());

	if (!pj)
		throw ctx->ConstructException();

	if (!proj_is_crs(pj))
	{
		proj_destroy(pj);
		throw gcnew ProjException(String::Format("'{0}' doesn't describe a coordinate system", from));
	}

	return ctx->Create<CoordinateReferenceSystem^>(pj);
}


CoordinateReferenceSystem^ CoordinateReferenceSystem::Create(array<String^>^ from, ProjContext^ ctx)
{
	if (!from)
		throw gcnew ArgumentNullException("from");

	if (!ctx)
		ctx = gcnew ProjContext();

	char** lst = new char* [from->Length + 1];
	for (int i = 0; i < from->Length; i++)
	{
		std::string fromStr = utf8_string(from[i]);
		lst[i] = _strdup(fromStr.c_str());
	}
	lst[from->Length] = 0;

	try
	{
		PJ* pj = proj_create_argv(ctx, from->Length, lst);

		if (!pj)
			throw ctx->ConstructException();

		if (!proj_is_crs(pj))
		{
			proj_destroy(pj);
			throw gcnew ProjException(String::Format("'{0}' doesn't describe a coordinate system", from));
		}

		return ctx->Create<CoordinateReferenceSystem^>(pj);
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

Details::GeodeticCRS^ CoordinateReferenceSystem::GeodeticCRS::get()
{
	if (!m_geodCRS && this)
	{
		Details::GeodeticCRS^ geoCrs = dynamic_cast<Details::GeodeticCRS^>(this);

		if (geoCrs)
			m_geodCRS = geoCrs;
		else
		{

			Context->ClearError(this);
			PJ* pj = proj_crs_get_geodetic_crs(Context, this);

			if (!pj)
				throw Context->ConstructException();

			m_geodCRS = Context->Create<Details::GeodeticCRS^>(pj);
		}
	}
	return m_geodCRS;
}

Details::Datum^ CoordinateReferenceSystem::Datum::get()
{
	if (!m_datum && this)
	{
		Context->ClearError(this);
		PJ* pj = proj_crs_get_datum(Context, this);

		if (!pj)
			pj = proj_crs_get_datum_ensemble(Context, this);

		if (!pj)
		{
			Context->ClearError(this);
			return nullptr;
		}

		m_datum = Context->Create<Details::Datum^>(pj);
	}

	return m_datum;
}


Details::CoordinateSystem^ CoordinateReferenceSystem::CoordinateSystem::get()
{
	if (!m_cs)
	{
		if (Type != ProjType::CompoundCrs)
		{
			Context->ClearError(this);
			PJ* pj = proj_crs_get_coordinate_system(Context, this);

			if (!pj)
			{
				throw Context->ConstructException();
			}

			m_cs = Context->Create<Details::CoordinateSystem^>(pj);
		}
	}
	return m_cs;
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::WithAxisNormalized(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_normalize_for_visualization(context, this);

	if (!pj)
		throw context->ConstructException();

	return context->Create<CoordinateReferenceSystem^>(pj);
}

Details::Ellipsoid^ CoordinateReferenceSystem::Ellipsoid::get()
{
	if (!m_ellipsoid && this)
	{
		Context->ClearError(this);
		PJ* pj = proj_get_ellipsoid(Context, this);

		if (!pj)
			throw Context->ConstructException();

		m_ellipsoid = Context->Create<Details::Ellipsoid^>(pj);
	}
	return m_ellipsoid;
}

Details::PrimeMeridian^ CoordinateReferenceSystem::PrimeMeridian::get()
{
	if (!m_primeMeridian && this)
	{
		Context->ClearError(this);

		PJ* pj = proj_get_prime_meridian(Context, this);

		if (!pj)
			throw Context->ConstructException();

		m_primeMeridian = Context->Create<Details::PrimeMeridian^>(pj);
	}
	return m_primeMeridian;
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::BaseCRS::get()
{
	if (!m_baseCrs && this)
	{
		Context->ClearError(this);
		PJ* pj = proj_get_source_crs(Context, this);

		if (!pj)
		{
			Context->ClearError(this);
			return nullptr;
		}
		m_baseCrs = Context->Create<CoordinateReferenceSystem^>(pj);
	}
	return m_baseCrs;
}

CoordinateTransform^ CoordinateReferenceSystem::DistanceTransform::get()
{
	if (!m_distanceTransform && this && this->GeodeticCRS)
	{
		m_distanceTransform = CoordinateTransform::Create(this, this->GeodeticCRS->WithAxisNormalized(Context), Context);
	}
	return m_distanceTransform;
}

int CoordinateReferenceSystem::AxisCount::get()
{
	if (!m_axis && this && Type != ProjType::CompoundCrs)
	{
		auto cs = CoordinateSystem;

		if (cs)
			m_axis = proj_cs_get_axis_count(cs->Context, cs);

		if (!m_axis)
			m_axis = -1;
	}

	return m_axis;
}

void CoordinateReferenceSystem::AxisCount::set(int value)
{
	m_axis = value;
}

Details::AxisCollection^ CoordinateReferenceSystem::Axis::get()
{
	auto cs = CoordinateSystem;

	if (cs)
		return cs->Axis;
	else
		return nullptr;
}
