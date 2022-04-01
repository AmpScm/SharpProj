#pragma once
#include "ProjObject.h"
namespace SharpProj {
    namespace Proj {
        public ref class PrimeMeridian :
            public ProjObject
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_longitude;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_unit_conv_factor;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ m_unit_name;

        internal:
            PrimeMeridian(ProjContext^ ctx, PJ* pj)
                : ProjObject(ctx, pj)
            {
                double longitude;
                double unit_conv_factor;
                const char* unit_name;

                if (proj_prime_meridian_get_parameters(Context, this, &longitude, &unit_conv_factor, &unit_name))
                {
                    m_longitude = longitude;
                    m_unit_conv_factor = unit_conv_factor;
                    m_unit_name = Utf8_PtrToString(unit_name);
                }
            }

        public:
            property double Longitude
            {
                double get()
                {
                    return m_longitude;
                }
            }

            property double UnitConversionFactor
            {
                double get()
                {
                    return m_unit_conv_factor;
                }
            }

            property String^ UnitName
            {
                String^ get()
                {
                    return m_unit_name;
                }
            }

        public:
            static PrimeMeridian^ CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx);

            static PrimeMeridian^ CreateFromDatabase(String^ authority, int code, [Optional] ProjContext^ ctx)
            {
                return CreateFromDatabase(authority, code.ToString(), ctx);
            }

            static PrimeMeridian^ CreateFromDatabase(Proj::Identifier^ identifier, [Optional] ProjContext^ ctx)
            {
                if (!(Object^)identifier)
                    throw gcnew ArgumentNullException("identifier");

                return CreateFromDatabase(identifier->Authority, identifier->Code, ctx);
            }

            static PrimeMeridian^ CreateFromEpsg(int epsgCode, [Optional] ProjContext^ ctx)
            {
                return CreateFromDatabase("Epsg", epsgCode, ctx);
            }

        };
    }
}