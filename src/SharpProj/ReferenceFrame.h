#pragma once
#include "ProjDatum.h"
namespace SharpProj {
    public ref class ReferenceFrame :
        public ProjDatum
    {
    internal:
        ReferenceFrame(ProjContext^ ctx, PJ* pj)
            : ProjDatum(ctx, pj)
        {


        }

    public:
        property Nullable<double> ReferenceEpoch
        {
            Nullable<double> get()
            {
                if (Type == ProjType::DynamicGeodeticReferenceFrame || Type == ProjType::DynamicVerticalReferenceFrame)
                    return proj_dynamic_datum_get_frame_reference_epoch(Context, this);
                else
                    return Nullable<double>();
            }
        }

    public:
    };

}