#pragma once
#include "ProjObject.h"
namespace SharpProj {
    namespace ProjDetaile {
        public ref class Datum :
            public ProjObject
        {
        internal:
            Datum(ProjContext^ ctx, PJ* pj)
                : ProjObject(ctx, pj)
            {
                //proj_get_codes_from_database

            }

        public:
        };
    }
}