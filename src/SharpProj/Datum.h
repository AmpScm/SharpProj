#pragma once
#include "ProjObject.h"
namespace SharpProj {
    namespace Proj {
        ref class Ellipsoid;

        public ref class Datum :
            public ProjObject
        {
        private:
            [DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
            Ellipsoid^ m_ellipsoid;
        internal:
            Datum(ProjContext^ ctx, PJ* pj)
                : ProjObject(ctx, pj)
            {
                //proj_get_codes_from_database

            }
        private:
            ~Datum();

        public:
            property Proj::Ellipsoid^ Ellipsoid
            {
                Proj::Ellipsoid^ get();
            }

        public:
            static Datum^ CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx);
            static Datum^ CreateFromDatabase(String^ authority, int code, [Optional] ProjContext^ ctx)
            {
                return Datum::CreateFromDatabase(authority, code.ToString(), ctx);
            }
            static Datum^ CreateFromEpsg(int epsgCode, [Optional] ProjContext^ ctx)
            {
                return CreateFromDatabase("EPSG", epsgCode, ctx);
            }
        };
    }
}