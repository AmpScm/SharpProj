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
		Exception^ ex = ctx->ConstructException();
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

	try
	{
		throw ctx->ConstructException();
	}
	finally
	{
		delete ctx;
	}
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

			Context->ClearError(this);
			PJ* pj = proj_crs_get_geodetic_crs(Context, this);

			if (!pj)
				throw Context->ConstructException();

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
	if (!context)
		context = Context;

	PJ* pj = proj_normalize_for_visualization(context, this);

	if (!pj)
		throw context->ConstructException();

	return context->Create<CoordinateReferenceSystem^>(pj);
}

Proj::Ellipsoid^ CoordinateReferenceSystem::Ellipsoid::get()
{
	if (!m_ellipsoid && this)
	{
		Context->ClearError(this);
		PJ* pj = proj_get_ellipsoid(Context, this);

		if (!pj)
			throw Context->ConstructException();

		m_ellipsoid = Context->Create<Proj::Ellipsoid^>(pj);
	}
	return m_ellipsoid;
}

Proj::PrimeMeridian^ CoordinateReferenceSystem::PrimeMeridian::get()
{
	if (!m_primeMeridian && this)
	{
		Context->ClearError(this);

		PJ* pj = proj_get_prime_meridian(Context, this);

		if (!pj)
			throw Context->ConstructException();

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
		{
			Context->ClearError(this);
			return nullptr;
		}
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
		{
			Context->ClearError(this);
			return nullptr;
		}
		m_demotedTo2D = Context->Create<CoordinateReferenceSystem^>(pj);
	}
	return m_demotedTo2D;
}

CoordinateTransform^ CoordinateReferenceSystem::DistanceTransform::get()
{
	if (!m_distanceTransform && this && this->GeodeticCRS)
	{
		m_distanceTransform = CoordinateTransform::Create(this, this->GeodeticCRS->WithAxisNormalized(Context), Context);
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


ReadOnlyCollection<CoordinateReferenceSystemInfo^>^ ProjContext::GetCoordinateReferenceSystems(CoordinateReferenceSystemFilter^ filter)
{
	if (!filter)
		throw gcnew ArgumentNullException("filter");

	std::string auth_name;
	if (filter->Authority)
		auth_name = ::utf8_string(filter->Authority);
	PROJ_CRS_LIST_PARAMETERS* params = proj_get_crs_list_parameters_create();

	try
	{
		auto types = filter->Types->ToArray();
		pin_ptr<ProjType> pTypes;

		if (types->Length)
		{
			pTypes = &types[0];
			params->typesCount = types->Length;
			params->types = reinterpret_cast<PJ_TYPE*>(pTypes);
		}

		params->allow_deprecated = filter->AllowDeprecated;

		int count;
		PROJ_CRS_INFO** infoList = proj_get_crs_info_list_from_database(this, auth_name.length() ? auth_name.c_str() : nullptr, params, &count);

		auto r = gcnew List<CoordinateReferenceSystemInfo^>(count);

		for (int i = 0; i < count; i++)
			r->Add(gcnew CoordinateReferenceSystemInfo(infoList[i], this));

		proj_crs_info_list_destroy(infoList);

		return r->AsReadOnly();
	}
	finally
	{
		proj_get_crs_list_parameters_destroy(params);
	}
}

ReadOnlyCollection<CoordinateReferenceSystemInfo^>^ ProjContext::GetCoordinateReferenceSystems()
{
	return GetCoordinateReferenceSystems(gcnew CoordinateReferenceSystemFilter());
}

CoordinateReferenceSystem^ CoordinateReferenceSystemInfo::Create(ProjContext^ ctx)
{
	if (!ctx && _ctx)
		ctx = _ctx;
	else
		ctx = gcnew ProjContext();

	return CoordinateReferenceSystem::CreateFromDatabase(Authority, Code, ctx);
}