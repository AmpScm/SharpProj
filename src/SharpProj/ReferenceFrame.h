#pragma once
#include "Datum.h"

namespace SharpProj {
    namespace ProjDetaile {
        ref class Ellipsoid;

        public ref class ReferenceFrame :
            public Datum
        {
        private:
            Ellipsoid^ m_ellipsoid;
        internal:
            ReferenceFrame(ProjContext^ ctx, PJ* pj)
                : Datum(ctx, pj)
            {


            }

        public:
            property Nullable<double> EpochYear
            {
                Nullable<double> get()
                {
                    if (Type == ProjType::DynamicGeodeticReferenceFrame || Type == ProjType::DynamicVerticalReferenceFrame)
                    {
                        double d = proj_dynamic_datum_get_frame_reference_epoch(Context, this);
                        if (d != -1.0)
                            return d;
                        else
                            return Nullable<double>();
                    }
                    else
                        return Nullable<double>();
                }
            }

            property Ellipsoid^ Ellipsoid
            {
                ProjDetaile::Ellipsoid^ get();
            }

        public:
        };

    }
}