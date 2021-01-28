#pragma once
#include "ProjObject.h"
namespace ProjSharp {
    public ref class DatumEnsamble :
        public ProjObject
    {
    internal:
        DatumEnsamble(ProjContext^ ctx, PJ* pj)
            : ProjObject(ctx, pj)
        {


        }

    public:
    };
}
