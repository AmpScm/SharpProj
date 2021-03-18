#include "pch.h"
#include "Datum.h"
#include "Ellipsoid.h"

Datum::~Datum()
{
	if ((Object^)m_ellipsoid)
	{
		delete m_ellipsoid;
		m_ellipsoid = nullptr;
	}
}

Datum^ Datum::CreateFromDatabase(String^ authority, String^ code, ProjContext^ ctx)
{
	if (String::IsNullOrWhiteSpace(authority))
		throw gcnew ArgumentNullException("authority");
	else if (String::IsNullOrWhiteSpace(code))
		throw gcnew ArgumentNullException("code");

	if (!ctx)
		ctx = gcnew ProjContext();

	std::string authStr = utf8_string(authority);
	std::string codeStr = utf8_string(code);
	PJ* pj = proj_create_from_database(ctx, authStr.c_str(), codeStr.c_str(), PJ_CATEGORY_DATUM_ENSEMBLE, false, nullptr);

	if (!pj)
	{
		ctx->ClearError();
		pj = proj_create_from_database(ctx, authStr.c_str(), codeStr.c_str(), PJ_CATEGORY_DATUM, false, nullptr);
	}

	if (pj)
		return ctx->Create<Datum^>(pj);
	else
		throw ctx->ConstructException();
}

Proj::Ellipsoid^ Datum::Ellipsoid::get()
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
