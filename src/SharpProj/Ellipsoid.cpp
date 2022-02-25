#include "pch.h"
#include "Ellipsoid.h"

using namespace SharpProj;


Ellipsoid^ Ellipsoid::CreateFromDatabase(String^ authority, String^ code, ProjContext^ ctx)
{
	if (String::IsNullOrWhiteSpace(authority))
		throw gcnew ArgumentNullException("authority");
	else if (String::IsNullOrWhiteSpace(code))
		throw gcnew ArgumentNullException("code");

	if (!ctx)
		ctx = gcnew ProjContext();

	std::string authStr = utf8_string(authority);
	std::string codeStr = utf8_string(code);
	PJ* pj = proj_create_from_database(ctx, authStr.c_str(), codeStr.c_str(), PJ_CATEGORY_ELLIPSOID, false, nullptr);

	if (pj)
		return ctx->Create<Ellipsoid^>(pj);
	else
		throw ctx->ConstructException();
}

String^ Ellipsoid::CelestialBodyName::get()
{
    if (!m_celestialBodyName)
    {
        const char* bodyName = proj_get_celestial_body_name(Context, this);

        m_celestialBodyName = Utf8_PtrToString(bodyName);
    }
    return m_celestialBodyName;
}
