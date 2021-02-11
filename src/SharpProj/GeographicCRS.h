#pragma once
#include "CoordinateReferenceSystem.h"
namespace SharpProj {

    namespace ProjDetaile {
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
                if ((Object^)m_hubCrs)
                {
                    delete m_hubCrs;
                    m_hubCrs = nullptr;
                }
            }

        public:
            property CoordinateReferenceSystem^ HubCRS
            {
                CoordinateReferenceSystem^ get();
            }
        };
    }
}