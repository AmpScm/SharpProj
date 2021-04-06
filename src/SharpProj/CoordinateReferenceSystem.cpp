#include "pch.h"
#include <proj_experimental.h>

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
using namespace SharpProj::Proj;

CoordinateReferenceSystem::~CoordinateReferenceSystem()
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
	m_from = nullptr;
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

		if (from->Trim()->StartsWith("+") && !from->Contains("+type=crs"))
		{
			try
			{
				return CoordinateReferenceSystem::Create(from + " +type=crs", ctx);
			}
			catch (ProjException^)
			{
			}
		}

		throw gcnew ProjException(String::Format("'{0}' doesn't describe a coordinate system", from));
	}

	return ctx->Create<CoordinateReferenceSystem^>(pj);
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::CreateFromWellKnownText(String^ from, ProjContext^ ctx)
{
	array<String^>^ wars = nullptr;

	return CreateFromWellKnownText(from, wars, ctx);
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings, ProjContext^ ctx)
{
	if (String::IsNullOrWhiteSpace(from))
		throw gcnew ArgumentNullException("from");

	if (!ctx)
		ctx = gcnew ProjContext();

	PROJ_STRING_LIST wrs = nullptr;
	PROJ_STRING_LIST errs = nullptr;
	const char* options[32] = {};

	std::string fromStr = utf8_string(from);
	PJ* pj = proj_create_from_wkt(ctx, fromStr.c_str(), options, &wrs, &errs);

	warnings = FromStringList(wrs);
	array<String^>^ errors = FromStringList(errs);

	if (wrs)
		proj_string_list_destroy(wrs);
	if (errs)
		proj_string_list_destroy(errs);

	if (!pj)
	{
		Exception^ ex = ctx->ConstructException("CRS from WKT failed");
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
	lst[from->Length] = 0; // also used for 'type=crs'

	try
	{
		PJ* pj = proj_create_argv(ctx, from->Length, lst);

		if (!pj)
			throw ctx->ConstructException();

		if (!proj_is_crs(pj))
		{
			lst[from->Length] = "type=crs";
			proj_destroy(pj);

			pj = proj_create_argv(ctx, from->Length + 1, lst);

			if (!pj || !proj_is_crs(pj))
			{
				if (pj)
					proj_destroy(pj);

				throw gcnew ProjException(String::Format("'{0}' doesn't describe a coordinate system", String::Join(" ", from)));
			}
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

CoordinateReferenceSystem^ CoordinateReferenceSystem::CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx)
{
	if (String::IsNullOrWhiteSpace(authority))
		throw gcnew ArgumentNullException("authority");
	else if (String::IsNullOrWhiteSpace(code))
		throw gcnew ArgumentNullException("code");

	if (!ctx)
		ctx = gcnew ProjContext();

	std::string authStr = utf8_string(authority);
	std::string codeStr = utf8_string(code);
	PJ* pj = proj_create_from_database(ctx, authStr.c_str(), codeStr.c_str(), PJ_CATEGORY_CRS, false, nullptr);

	if (pj)
		return ctx->Create<CoordinateReferenceSystem^>(pj);
	else
		throw ctx->ConstructException();
}

Proj::GeodeticCRS^ CoordinateReferenceSystem::GeodeticCRS::get()
{
	if (!m_geodCRS && this)
	{
		Proj::GeodeticCRS^ geoCrs = dynamic_cast<Proj::GeodeticCRS^>(this);

		if (geoCrs)
			m_geodCRS = geoCrs;
		else
		{

			PJ* pj = proj_crs_get_geodetic_crs(Context, this);

			if (!pj)
				Context->ClearError(this);
			else
				m_geodCRS = Context->Create<Proj::GeodeticCRS^>(pj);
		}
	}
	return m_geodCRS;
}

Proj::Datum^ CoordinateReferenceSystem::Datum::get()
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

		m_datum = Context->Create<Proj::Datum^>(pj);
	}

	return m_datum;
}


Proj::CoordinateSystem^ CoordinateReferenceSystem::CoordinateSystem::get()
{
	if (!m_cs)
	{
		if (Type != ProjType::CompoundCrs)
		{
			Context->ClearError(this);
			PJ* pj = proj_crs_get_coordinate_system(Context, this);

			if (!pj)
			{
				Context->ClearError(this);
				return nullptr;
			}

			m_cs = Context->Create<Proj::CoordinateSystem^>(pj);
		}
	}
	return m_cs;
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::WithAxisNormalized(ProjContext^ context)
{
	bool allowReturnSame = false;
	if (!context)
	{
		context = Context;
		allowReturnSame = true;
	}

	PJ* pj = proj_normalize_for_visualization(context, this);

	if (!pj)
		throw context->ConstructException("Axis normalization failed");

	if (allowReturnSame && proj_is_equivalent_to_with_ctx(context, pj, this, PJ_COMP_STRICT))
	{
		proj_destroy(pj);
		return this;
	}

	CoordinateReferenceSystem^ crs = context->Create<CoordinateReferenceSystem^>(pj);

	double lon;
	if (!proj_get_area_of_use(context, pj, &lon, nullptr, nullptr, nullptr, nullptr) && lon > -1000.0) /* lon = -1000 is unavailable */
		crs->m_from = this;

	return crs;
}

Proj::Ellipsoid^ CoordinateReferenceSystem::Ellipsoid::get()
{
	if (!m_ellipsoid && this)
	{
		PJ* pj = proj_get_ellipsoid(Context, this);

		if (!pj)
			Context->ClearError(this);
		else
			m_ellipsoid = Context->Create<Proj::Ellipsoid^>(pj);
	}
	return m_ellipsoid;
}

Proj::PrimeMeridian^ CoordinateReferenceSystem::PrimeMeridian::get()
{
	if (!m_primeMeridian && this)
	{
		PJ* pj = proj_get_prime_meridian(Context, this);

		if (!pj)
			Context->ClearError(this);
		else
			m_primeMeridian = Context->Create<Proj::PrimeMeridian^>(pj);
	}
	return m_primeMeridian;
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::BaseCRS::get()
{
	if (!m_baseCrs && this)
	{
		PJ* pj = proj_get_source_crs(Context, this);

		if (!pj)
		{
			Context->ClearError(this);
			return nullptr;
		}
		else
			m_baseCrs = Context->Create<CoordinateReferenceSystem^>(pj);
	}
	return m_baseCrs;
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::PromotedTo3D()
{
	if (!m_promotedTo3D && this)
	{
		PJ* pj = proj_crs_promote_to_3D(Context, nullptr, this);

		if (!pj)
			Context->ClearError(this);
		else
			m_promotedTo3D = Context->Create<CoordinateReferenceSystem^>(pj);
	}
	return m_promotedTo3D;
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::DemoteTo2D()
{
	if (!m_demotedTo2D && this)
	{
		PJ* pj = proj_crs_demote_to_2D(Context, nullptr, this);

		if (!pj)
			Context->ClearError(this);
		else
			m_demotedTo2D = Context->Create<CoordinateReferenceSystem^>(pj);
	}
	return m_demotedTo2D;
}

CoordinateTransform^ CoordinateReferenceSystem::DistanceTransform::get()
{
	if (!m_distanceTransform && this && this->GeodeticCRS)
	{
		try
		{
			m_distanceTransform = CoordinateTransform::Create(this, this->GeodeticCRS->WithAxisNormalized(nullptr), Context);
		}
		catch (ProjException^)
		{
			return nullptr;
		}
		m_distanceTransform->EnsureDistance();
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

Proj::AxisCollection^ CoordinateReferenceSystem::Axis::get()
{
	auto cs = CoordinateSystem;

	if (cs)
		return cs->Axis;
	else
		return nullptr;
}
