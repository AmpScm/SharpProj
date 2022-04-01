#pragma once
#include "ProjObject.h"
namespace SharpProj {
    namespace Proj {
        public ref class Ellipsoid : ProjObject, IHasCelestialBody
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_semi_major_metre;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_semi_minor_metre;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly bool m_is_semi_minor_computed;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_inv_flattening;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_celestialBodyName;

        internal:
            Ellipsoid(ProjContext^ ctx, PJ* pj)
                : ProjObject(ctx, pj)
            {
                double semi_major_metre, semi_minor_metre;
                int is_semi_minor_computed;
                double inv_flattening;

                if (proj_ellipsoid_get_parameters(Context, this,
                    &semi_major_metre, &semi_minor_metre, &is_semi_minor_computed, &inv_flattening))
                {
                    m_semi_major_metre = semi_major_metre;
                    m_semi_minor_metre = semi_minor_metre;
                    m_is_semi_minor_computed = (is_semi_minor_computed != 0);
                    m_inv_flattening = inv_flattening;
                }

            }

        public:
            property double SemiMajorMetre
            {
                double get()
                {
                    return m_semi_major_metre;
                }
            }
            property double SemiMinorMetre
            {
                double get()
                {
                    return m_semi_minor_metre;
                }
            }

            property bool IsSemiMinorComputed
            {
                bool get()
                {
                    return m_is_semi_minor_computed;
                }
            }
            property double InverseFlattening
            {
                double get()
                {
                    return m_inv_flattening;
                }
            }

            /// <summary>
            /// Body on which this applies. Usually 'Earth'
            /// </summary>
            property String^ CelestialBodyName
            {
                virtual String^ get() sealed;
            }

        public:
            static Ellipsoid^ CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx);

            static Ellipsoid^ CreateFromDatabase(String^ authority, int code, [Optional] ProjContext^ ctx)
            {
                return CreateFromDatabase(authority, code.ToString(), ctx);
            }
            static Ellipsoid^ CreateFromDatabase(Proj::Identifier^ identifier, [Optional] ProjContext^ ctx)
            {
                if (!(Object^)identifier)
                    throw gcnew ArgumentNullException("identifier");

                return CreateFromDatabase(identifier->Authority, identifier->Code, ctx);
            }
            static Ellipsoid^ CreateFromEpsg(int epsgCode, [Optional] ProjContext^ ctx)
            {
                return CreateFromDatabase("EPSG", epsgCode, ctx);
            }
        };
    }

    using Proj::Ellipsoid;
}