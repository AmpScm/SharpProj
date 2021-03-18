#include "pch.h"
#include "PrimeMeridian.h"


PrimeMeridian^ PrimeMeridian::CreateFromDatabase(String^ authority, String^ code, ProjContext^ ctx)
{
	if (String::IsNullOrWhiteSpace(authority))
		throw gcnew ArgumentNullException("authority");
	else if (String::IsNullOrWhiteSpace(code))
		throw gcnew ArgumentNullException("code");

	if (!ctx)
		ctx = gcnew ProjContext();

	std::string authStr = utf8_string(authority);
	std::string codeStr = utf8_string(code);
	PJ* pj = proj_create_from_database(ctx, authStr.c_str(), codeStr.c_str(), PJ_CATEGORY_PRIME_MERIDIAN, false, nullptr);

	if (pj)
		return ctx->Create<PrimeMeridian^>(pj);
	else
		throw ctx->ConstructException();
}
