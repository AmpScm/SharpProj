#include "pch.h"
#include "ProjContext.h"
#include "ProjCrs.h"

using namespace ProjSharp;


ProjCrs^ ProjCrs::Create(ProjContext^ ctx, String^ from)
{
	if (!ctx)
		throw gcnew ArgumentNullException("ctx");
	else if (String::IsNullOrWhiteSpace(from))
		throw gcnew ArgumentNullException("from");

	std::string fromStr = utf8_string(from);
	PJ* pj = proj_create(ctx, fromStr.c_str());


	if (!pj)
		throw ctx->ConstructException();

	return gcnew ProjCrs(ctx, pj);
}


ProjCrs^ ProjCrs::Create(ProjContext^ ctx, ...array<String^>^ from)
{
	if (!ctx)
		throw gcnew ArgumentNullException("ctx");
	else if (!from)
		throw gcnew ArgumentNullException("from");

	char** lst = new char*[from->Length+1];
	for(int i = 0; i < from->Length; i++)
	{
		std::string fromStr = utf8_string(from[i]);
		lst[i] = strdup(fromStr.c_str());
	}
	lst[from->Length] = 0;
	
	try
	{
		PJ* pj = proj_create_argv(ctx, from->Length, lst);


		if (!pj)
			throw ctx->ConstructException();

		return gcnew ProjCrs(ctx, pj);
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
