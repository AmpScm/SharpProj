#include "pch.h"
#include "ProjContext.h"
#include "ProjException.h"
#include "CoordinateReferenceSystem.h"
#include "ProjDatumList.h"
#include "CoordinateTransform.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "PrimeMeridian.h"
#include "ProjDatum.h"


using namespace SharpProj;

SharpProj::CoordinateReferenceSystem::~CoordinateReferenceSystem()
{
	if ((Object^)m_cs)
	{
		delete m_cs;
		m_cs = nullptr;
	}

	if ((Object^)m_geodCRS)
	{
		delete m_geodCRS;
		m_geodCRS = nullptr;
	}
	if ((Object^)m_ellipsoid)
	{
		delete m_ellipsoid;
		m_ellipsoid = nullptr;
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

CoordinateReferenceSystem^ CoordinateReferenceSystem::GeodeticCRS::get()
{
	if (!m_geodCRS && this)
	{
		Context->ClearError(this);
		PJ* pj = proj_crs_get_geodetic_crs(Context, this);

		if (!pj)
			throw Context->ConstructException();

		m_geodCRS = Context->Create<CoordinateReferenceSystem^>(pj);
	}
	return m_geodCRS;
}

ProjDatum^ CoordinateReferenceSystem::GetHorizontalDatum(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError(this);
	PJ* pj = proj_crs_get_horizontal_datum(context, this);

	if (!pj)
		throw context->ConstructException();

	return context->Create<ProjDatum^>(pj);
}

ProjDatum^ CoordinateReferenceSystem::GetDatum(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError(this);
	PJ* pj = proj_crs_get_datum(context, this);

	if (!pj)
		throw context->ConstructException();

	return context->Create<ProjDatum^>(pj);
}

ProjDatumList^ CoordinateReferenceSystem::GetDatumList(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError(this);
	PJ* pj = proj_crs_get_datum_ensemble(context, this);

	if (!pj)
	{
		if (proj_context_errno(context) == 0)
			return nullptr;

		throw context->ConstructException();
	}

	return context->Create< ProjDatumList ^>(pj);
}

ProjDatum^ CoordinateReferenceSystem::GetDatumForced(ProjContext^ context)
{
	if (!context)
		context = Context;

	context->ClearError(this);
	PJ* pj = proj_crs_get_datum_forced(context, this);

	if (!pj)
		throw context->ConstructException();

	return context->Create<ProjDatum^>(pj);
}

SharpProj::CoordinateSystem^ CoordinateReferenceSystem::CoordinateSystem::get()
{
	if (!m_cs)
	{
		if (Type == ProjType::CompoundCrs)
		{
			int ax = proj_cs_get_axis_count(Context, this);

			GC::KeepAlive(ax);

		}
		else
		{
			Context->ClearError(this);
			PJ* pj = proj_crs_get_coordinate_system(Context, this);

			if (!pj)
			{
				throw Context->ConstructException();
			}

			m_cs = Context->Create<SharpProj::CoordinateSystem^>(pj);
		}
	}
	return m_cs;
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::GetNormalized(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_normalize_for_visualization(context, this);

	if (!pj)
		throw context->ConstructException();

	return context->Create<CoordinateReferenceSystem^>(pj);
}

Ellipsoid^ CoordinateReferenceSystem::Ellipsoid::get()
{
	if (!m_ellipsoid && this)
	{
		Context->ClearError(this);
		PJ* pj = proj_get_ellipsoid(Context, this);

		if (!pj)
			throw Context->ConstructException();

		m_ellipsoid = Context->Create<SharpProj::Ellipsoid^>(pj);
	}
	return m_ellipsoid;
}

PrimeMeridian^ CoordinateReferenceSystem::GetPrimeMeridian(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_get_prime_meridian(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<PrimeMeridian^>(context->Create(pj));
}

CoordinateTransform^ CoordinateReferenceSystem::GetTransform(ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_crs_get_coordoperation(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<CoordinateTransform^>(context->Create(pj));
}

CoordinateReferenceSystem^ CoordinateReferenceSystem::GetBaseCRS([Optional] ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_get_source_crs(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<CoordinateReferenceSystem^>(context->Create(pj));
}


CoordinateReferenceSystem^ CoordinateReferenceSystem::GetHubCRS([Optional] ProjContext^ context)
{
	if (!context)
		context = Context;

	PJ* pj = proj_get_target_crs(context, this);

	if (!pj)
		throw context->ConstructException();

	return static_cast<CoordinateReferenceSystem^>(context->Create(pj));
}


int CoordinateReferenceSystem::AxisCount::get()
{
	if (!m_axis && Type != ProjType::CompoundCrs)
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
