#pragma once
#include "ProjObject.h"
namespace SharpProj {
    public ref class ProjDatum :
        public ProjObject
    {
    internal:
        ProjDatum(ProjContext^ ctx, PJ* pj)
            : ProjObject(ctx, pj)
        {
            //proj_get_codes_from_database

        }

    public:
    };
}