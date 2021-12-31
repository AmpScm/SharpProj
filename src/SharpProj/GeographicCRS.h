#pragma once
#include "CoordinateReferenceSystem.h"
namespace SharpProj {

    namespace Proj {
        public ref class GeodeticCRS : public CoordinateReferenceSystem
        {
        internal:
            GeodeticCRS(ProjContext^ ctx, PJ* pj)
                : CoordinateReferenceSystem(ctx, pj)
            {
            }

        };

        public ref class GeographicCRS :
            public GeodeticCRS
        {
        internal:
            GeographicCRS(ProjContext^ ctx, PJ* pj)
                : GeodeticCRS(ctx, pj)
            {


            }

        public:
        };

        public ref class BoundCRS : public CoordinateReferenceSystem
        {
        private:
            CoordinateReferenceSystem^ m_hubCrs;

        internal:
            BoundCRS(ProjContext^ ctx, PJ* pj)
                : CoordinateReferenceSystem(ctx, pj)
            {

            }
        private:
            ~BoundCRS()
            {
                DisposeIfNotNull(m_hubCrs);
            }

        public:
            property CoordinateReferenceSystem^ HubCRS
            {
                CoordinateReferenceSystem^ get();
            }
        };
    }
}