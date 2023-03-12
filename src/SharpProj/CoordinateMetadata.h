#pragma once
#include "ProjObject.h"
namespace SharpProj {
    ref class CoordinateReferenceSystem;

    namespace Proj {        

        ref class CoordinateMetadata :
            public ProjObject
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            CoordinateReferenceSystem^ m_crs;
        internal:
            CoordinateMetadata(ProjContext^ ctx, PJ* pj)
                : ProjObject(ctx, pj)
            {
                //proj_get_codes_from_database

            }
        private:
            ~CoordinateMetadata();

                
        public:
            property Nullable<double> Epoch
            {
                Nullable<double> get()
                {
                    auto v = proj_coordinate_metadata_get_epoch(Context, this);

                    if (isnan(v))
                        return Nullable<double>();
                    else
                        return v;
                }
            }   

            property CoordinateReferenceSystem^ CRS
            {
                CoordinateReferenceSystem^ get();
            }
        };

    }
}