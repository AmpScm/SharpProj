#pragma once
#include "ProjDatum.h"
namespace ProjSharp {
    public ref class ReferenceFrame :
        public ProjDatum
    {
    internal:
        ReferenceFrame(ProjContext^ ctx, PJ* pj)
            : ProjDatum(ctx, pj)
        {


        }

    public:
    };

}