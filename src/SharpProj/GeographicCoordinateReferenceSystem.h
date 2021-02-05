#pragma once
#include "CoordinateReferenceSystem.h"
namespace SharpProj {
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