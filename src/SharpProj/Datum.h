#pragma once
#include "ProjObject.h"
namespace SharpProj {
    namespace Proj {
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
            static Datum^ CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx);
        };
    }
}