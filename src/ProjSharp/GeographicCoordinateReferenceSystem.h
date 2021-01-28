#pragma once
#include "CoordinateReferenceSystem.h"
namespace ProjSharp {
    ref class Ellipsoid;

    public ref class GeographicCoordinateReferenceSystem :
        public CoordinateReferenceSystem
    {
    internal:
        GeographicCoordinateReferenceSystem(ProjContext^ ctx, PJ* pj)
            : CoordinateReferenceSystem(ctx, pj)
        {


        }

    public:
    };

}