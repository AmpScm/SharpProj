#pragma once
#include <proj.h>

#include "ProjContext.h"
#include "CoordinateArea.h"
#include "PPoint.h"
#include "UsageArea.h"
#include "ProjIdentifier.h"

namespace SharpProj {
    using System::Collections::Generic::IReadOnlyList;
    value class PPoint;

    namespace Proj {
        ref class ProjObject;

        public interface class IHasCelestialBody
        {
            property String^ CelestialBodyName
            {
                String^ get();
            }
        };

        public enum class ProjType
        {
            Unknown = PJ_TYPE_UNKNOWN,

            Ellipsoid = PJ_TYPE_ELLIPSOID,

            PrimeMeridian = PJ_TYPE_PRIME_MERIDIAN,

            GeodeticReferenceFrame = PJ_TYPE_GEODETIC_REFERENCE_FRAME,
            DynamicGeodeticReferenceFrame = PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME,
            VerticalReferenceFrame = PJ_TYPE_VERTICAL_REFERENCE_FRAME,
            DynamicVerticalReferenceFrame = PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME,
            DatumEnsemble = PJ_TYPE_DATUM_ENSEMBLE,

            /// <summary>
            /// Abstract type, not returned by <see cref="ProjObject" />.Type.
            /// </summary>
            CRS = PJ_TYPE_CRS,

            GeodeticCrs = PJ_TYPE_GEODETIC_CRS,
            GeocentricCrs = PJ_TYPE_GEOCENTRIC_CRS,

            /// <summary>
            /// Abstract type, not returned by <see cref="ProjObject" />.Type. Will return Geographic2DCrs or Geographic3DCrs instead.
            /// </summary>
            GeographicCrs = PJ_TYPE_GEOGRAPHIC_CRS,

            Geographic2DCrs = PJ_TYPE_GEOGRAPHIC_2D_CRS,
            Geographic3DCrs = PJ_TYPE_GEOGRAPHIC_3D_CRS,
            VerticalCrs = PJ_TYPE_VERTICAL_CRS,
            ProjectedCrs = PJ_TYPE_PROJECTED_CRS,
            CompoundCrs = PJ_TYPE_COMPOUND_CRS,
            TemporalCrs = PJ_TYPE_TEMPORAL_CRS,
            EngineeringCrs = PJ_TYPE_ENGINEERING_CRS,
            BoundCrs = PJ_TYPE_BOUND_CRS,
            OtherCrs = PJ_TYPE_OTHER_CRS,

            Conversion = PJ_TYPE_CONVERSION,
            Transformation = PJ_TYPE_TRANSFORMATION,
            ConcatenatedOperation = PJ_TYPE_CONCATENATED_OPERATION,
            OtherCoordinateTransform = PJ_TYPE_OTHER_COORDINATE_OPERATION,

            TemporalDatum = PJ_TYPE_TEMPORAL_DATUM,
            EngineeringDatum = PJ_TYPE_ENGINEERING_DATUM,
            ParametricDatum = PJ_TYPE_PARAMETRIC_DATUM,




            // Local types
            ChooseTransform = 1001,
            CoordinateSystem
        };



        public enum class WktType
        {
            WKT2_2015 = PJ_WKT2_2015,
            WKT2_2015_Simplified = PJ_WKT2_2015_SIMPLIFIED,
            WKT2_2019 = PJ_WKT2_2019,
            WKT2_2019_Simplified = PJ_WKT2_2019_SIMPLIFIED,
            WKT1_GDAL = PJ_WKT1_GDAL,
            WKT1_SRI = PJ_WKT1_ESRI
        };

        public ref class WktOptions
        {
        public:
            property WktType WktType;
            property bool SingleLine;
            property bool NoIndentation;
            property Nullable<bool> WriteAxis;
            property bool Strict;
            property bool AllowEllipsoidalHeightAsVerticalCRS;

        public:
            WktOptions()
            {
                WktType = Proj::WktType::WKT2_2019;
            }
        };

        public enum class ProjStringType
        {
            Proj5 = PJ_PROJ_5,
            Proj4 = PJ_PROJ_4
        };

        public ref class ProjStringOptions
        {
        public:
            property ProjStringType ProjStringType;
            property bool MultiLine;
            property bool NoIndentation;
            property bool WriteApproxFlag;

        public:
            ProjStringOptions()
            {

            }
        };

        public enum class ProjJsonType
        {
            None = 0,
            SchemaV02,
            SchemaV04,
            SchemaV05
        };

        public ref class ProjJsonOptions
        {
        public:
            property ProjJsonType ProjJsonType;
            property bool NoMultiLine;
            property bool NoIndentation;

        public:
            ProjJsonOptions()
            {

            }

        private:
#define PROJ_SCHEMA_PREFIX "https://proj.org/schemas/"
#define PROJ_SCHEMA_SUFFIX "/projjson.schema.json"
        public:
            static initonly String^ SchemaV0_2_Url = PROJ_SCHEMA_PREFIX "v0.2" PROJ_SCHEMA_SUFFIX;
            static initonly String^ SchemaV0_4_Url = PROJ_SCHEMA_PREFIX "v0.4" PROJ_SCHEMA_SUFFIX;
            static initonly String^ SchemaV0_5_Url = PROJ_SCHEMA_PREFIX "v0.5" PROJ_SCHEMA_SUFFIX;
            // Don't forget to update the ProjJsonType enum and AsProjJson() function(s).

            static property String^ LastSchemaUrl
            {
                String^ get() { return SchemaV0_5_Url; }
            }
        };

        public ref class CreateFromWKTOptions
        {
        public:
            [field: DebuggerBrowsable(DebuggerBrowsableState::Never)]
            property bool Strict;
        };

        [DebuggerDisplay("[{Type}] {ToString(),nq}")]
        public ref class ProjObject
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            ProjContext^ m_ctx;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            item_wrapper<PJ_CONTEXT, ProjContext, PJ>& m_pj;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_remarks;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_scope;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            Proj::IdentifierList^ m_idList;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            bool m_noProj;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            Proj::UsageArea^ m_usageArea;

            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            bool _disposed;

        private:
            ~ProjObject();
            !ProjObject();

        private protected:
            void ForceUnknownInfo()
            {
                m_noProj = true;
            }

        internal:
            ProjObject(ProjContext^ ctx, PJ* pj);

            static operator PJ* (ProjObject^ pj)
            {
                if ((Object^)pj == nullptr)
                    return nullptr;
                else if (pj->_disposed || pj->m_pj == nullptr)
                    throw gcnew ObjectDisposedException("PJ disposed");

                return pj->m_pj;
            }

            static operator bool(ProjObject^ pj)
            {
                if ((Object^)pj == nullptr)
                    return false;
                else if (pj->m_pj == nullptr)
                    return false;

                return true;
            }

        public:
            virtual String^ ToString() override
            {
                auto name = Name;

                return name ? name : "<no-name>";
            }

        internal:
            static array<String^>^ FromStringList(PROJ_STRING_LIST lst)
            {
                if (!lst || !*lst)
                    return EMPTY_ARRAY(String^);

                auto items = gcnew System::Collections::Generic::List<String^>();

                while (*lst)
                {
                    items->Add(Utf8_PtrToString(*lst));
                    lst++;
                }

                return items->ToArray();
            }

        public:
            property ProjContext^ Context
            {
                ProjContext^ get()
                {
                    return ((Object^)m_ctx != nullptr) ? m_ctx : (ProjContext^)nullptr;
                }
            }

            ProjObject^ Clone([Optional]ProjContext^ ctx)
            {
                if (!ctx)
                    ctx = Context->Clone();

                return DoClone(ctx);
            }

        private protected:
            virtual ProjObject^ DoClone(ProjContext^ ctx)
            {
                return ctx->Create(proj_clone(ctx, this));
            }

        public:
            property String^ Name
            {
                String^ get()
                {
                    if (!m_name)
                    {
                        m_name = Utf8_PtrToString(proj_get_name(this));
                    }
                    return m_name;
                }
            protected private:
                void set(String^ value)
                {
                    m_name = value;
                }
            }

            property String^ Remarks
            {
                String^ get();
            }

            property String^ Scope
            {
                String^ get();
            }

            property Proj::ProjType Type
            {
                virtual Proj::ProjType get()
                {
                    return (Proj::ProjType)proj_get_type(this);
                }
            }

            property UsageArea^ UsageArea
            {
                virtual Proj::UsageArea^ get()
                {
                    if (!m_usageArea)
                    {
                        double west, south, east, north;
                        const char* name;
                        if (proj_get_area_of_use(Context, this, &west, &south, &east, &north, &name))
                        {
                            m_usageArea = gcnew Proj::UsageArea(this, west, south, east, north, Utf8_PtrToString(name));
                        }
                    }
                    return m_usageArea;
                }
            internal:
                void set(Proj::UsageArea^ value)
                {
                    m_usageArea = value;
                }
            }

            String^ AsProjJson(ProjJsonOptions^ options)
            {
                if (m_noProj)
                    return nullptr;

                const char* opts[30] = {};
                int nOpts = 0;

                if (options && options->NoMultiLine)
                    opts[nOpts++] = "MULTILINE=NO";
                if (options && options->NoIndentation)
                    opts[nOpts++] = "INDENTATION_WIDTH=0";
                if (options && options->ProjJsonType != ProjJsonType::None)
                {
                    switch (options->ProjJsonType)
                    {
                    case ProjJsonType::SchemaV02:
                        opts[nOpts++] = "SCHEMA=" PROJ_SCHEMA_PREFIX "v0.2" PROJ_SCHEMA_SUFFIX;
                        break;
                    case ProjJsonType::SchemaV04:
                        opts[nOpts++] = "SCHEMA=" PROJ_SCHEMA_PREFIX "v0.4" PROJ_SCHEMA_SUFFIX;
                        break;
                    case ProjJsonType::SchemaV05:
                        opts[nOpts++] = "SCHEMA=" PROJ_SCHEMA_PREFIX "v0.5" PROJ_SCHEMA_SUFFIX;
                        break;
                    default:
                        throw gcnew ArgumentOutOfRangeException();
                    }
                }

                const char* v = proj_as_projjson(Context, this, opts);

                return Utf8_PtrToString(v);
            }

            String^ AsProjJson()
            {
                return AsProjJson(nullptr);
            }

            String^ AsWellKnownText(WktOptions^ options)
            {
                if (m_noProj)
                    return nullptr;
                PJ_WKT_TYPE tp = options ? (PJ_WKT_TYPE)options->WktType : PJ_WKT2_2019;
                const char* opts[30] = {};
                int nOpts = 0;

                if (options && (tp != PJ_WKT1_ESRI) == options->SingleLine)
                {
                    opts[nOpts++] = (tp != PJ_WKT1_ESRI) ? "MULTILINE=NO" : "MULTILINE=YES";
                }
                if (options && options->NoIndentation)
                {
                    opts[nOpts++] = "INDENTATION_WIDTH=0";
                }
                if (options && options->WriteAxis.HasValue)
                {
                    opts[nOpts++] = options->WriteAxis.Value ? "OUTPUT_AXIS=YES" : "OUTPUT_AXIS=NO";
                }
                if (!options || !options->Strict)
                {
                    opts[nOpts++] = "STRICT=NO";
                }
                if (options && options->AllowEllipsoidalHeightAsVerticalCRS)
                {
                    opts[nOpts++] = "ALLOW_ELLIPSOIDAL_HEIGHT_AS_VERTICAL_CRS=YES";
                }

                const char* v = proj_as_wkt(Context, this, tp, opts);

                return Utf8_PtrToString(v);
            }

            String^ AsWellKnownText()
            {
                return AsWellKnownText(nullptr);
            }

            String^ AsProjString(ProjStringOptions^ options)
            {
                if (m_noProj)
                    return nullptr;

                PJ_PROJ_STRING_TYPE string_type = options ? (PJ_PROJ_STRING_TYPE)options->ProjStringType : PJ_PROJ_5 /* Last as of 2021-01 */;

                const char* opts[30] = {};
                int nOpts = 0;

                if (options && options->MultiLine)
                {
                    opts[nOpts++] = "MULTILINE=YES";
                }
                if (options && options->NoIndentation)
                {
                    opts[nOpts++] = "INDENTATION_WIDTH=0";
                }
                if (options && options->WriteApproxFlag)
                {
                    opts[nOpts++] = "USE_APPROX_TMERC=YES";
                }

                const char* v = proj_as_proj_string(Context, this, string_type, opts);

                return Utf8_PtrToString(v);
            }

            String^ AsProjString()
            {
                return AsProjString(nullptr);
            }

        public:
            /// <summary>
            /// Gets the list of declared identifiers of this proj object. (Most commonly filled from the database or WKT)
            /// </summary>
            property Proj::IdentifierList^ Identifiers
            {
                Proj::IdentifierList^ get();
            }

            /// <summary>
            /// Gets the first (primary) <see cref="Proj::Identifier" /> from the <see cref="Identifiers"/> list.
            /// </summary>
            property Proj::Identifier^ Identifier
            {
                Proj::Identifier^ get()
                {
                    auto a = Identifiers;
                    if (a)
                        return a[0];
                    else
                        return nullptr;
                }
            }

            /// <summary>
            /// Checks if this Object is equivalent to the other proj object.
            /// </summary>
            /// <param name="other"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            bool IsEquivalentTo(ProjObject^ other, [Optional] ProjContext^ ctx)
            {
                if (!other || m_noProj || other->m_noProj)
                    return false;

                if (!ctx)
                    ctx = Context;

                return 0 != proj_is_equivalent_to_with_ctx(ctx, this, other, PJ_COMP_EQUIVALENT);
            }

            /// <summary>
            /// Checks if this Object is equivalent to the other proj object with the axis in any order.
            /// </summary>
            /// <param name="other"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            bool IsEquivalentToRelaxed(ProjObject^ other, [Optional] ProjContext^ ctx)
            {
                if (!other || other->m_noProj)
                    return false;

                if (!ctx)
                    ctx = Context;

                return 0 != proj_is_equivalent_to_with_ctx(ctx, this, other, PJ_COMP_EQUIVALENT_EXCEPT_AXIS_ORDER_GEOGCRS);
            }

        public:
            /// <summary>
            /// Creates a proj object from its definition. Shortcut for <see cref="ProjContext::Create(String^)" />
            /// </summary>
            /// <param name="definition"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            static ProjObject^ Create(String^ definition, [Optional]ProjContext^ ctx);
            /// <summary>
            /// Creates a proj object from its definition. Shortcut for <see cref="ProjContext"/>.Create" />
            /// </summary>
            /// <param name="from"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            static ProjObject^ Create(array<String^>^ from, [Optional]ProjContext^ ctx);

            /// <summary>
            /// Creates a proj object from its definition. Shortcut for <see cref="ProjContext"/>.CreateFromWellKnownText" />
            /// </summary>
            /// <param name="from"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            static ProjObject^ CreateFromWellKnownText(String^ from, [Optional] ProjContext^ ctx)
            {
                return CreateFromWellKnownText(from, nullptr, ctx);
            }
            /// <summary>
            /// Creates a proj object from its definition. Shortcut for <see cref="ProjContext"/>.CreateFromWellKnownText" />
            /// </summary>
            /// <param name="from"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            static ProjObject^ CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Optional] ProjContext^ ctx);
            /// <summary>
            /// Creates a proj object from its definition. Shortcut for <see cref="ProjContext"/>.CreateFromWellKnownText" />
            /// </summary>
            /// <param name="from"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            static ProjObject^ CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx)
            {
                return CreateFromWellKnownText(from, nullptr, warnings, ctx);
            }

            /// <summary>
            /// Creates a proj object from its definition. Shortcut for <see cref="ProjContext"/>.CreateFromWellKnownText" />
            /// </summary>
            /// <param name="from"></param>
            /// <param name="ctx"></param>
            /// <returns></returns>
            static ProjObject^ CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx);

        private protected:
            void SetCoordinate(PJ_COORD& coord, PPoint% coordinate)
            {
                coord.v[0] = coordinate.X;
                coord.v[1] = coordinate.Y;
                coord.v[2] = coordinate.Z;
                coord.v[3] = coordinate.T;
            }
        };
    }
}