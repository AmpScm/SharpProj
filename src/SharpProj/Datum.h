#pragma once
#include "ProjObject.h"
namespace SharpProj {
    namespace Proj {
        ref class Ellipsoid;

        public ref class Datum : ProjObject, IHasCelestialBody
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            Ellipsoid^ m_ellipsoid;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_celestialBodyName;

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

            /// <summary>
            /// Body on which this applies. Usually 'Earth'
            /// </summary>
            property String^ CelestialBodyName
            {
                virtual String^ get() sealed;
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