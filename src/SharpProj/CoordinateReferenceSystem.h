#pragma once
#include "ProjObject.h"
#include "CoordinateReferenceSystemInfo.h"
namespace SharpProj {
    using System::Collections::ObjectModel::ReadOnlyCollection;
    using System::Collections::Generic::List;
    ref class CoordinateTransform;
    ref class CoordinateArea;

    namespace Proj
    {
        ref class Axis;
        ref class AxisCollection;
        ref class Datum;
        ref class Ellipsoid;
        ref class PrimeMeridian;

        ref class GeodeticCRS;
        ref class GeometricCRS;

        ref class CoordinateSystem;
        ref class UsageArea;
    }

    ref class CoordinateReferenceIdList;

    /// <summary>
    /// A coordinate reference system (CRS) refers to the way in which spatial data that represent the earth's surface (which is round / 3 dimensional) 
    /// is represented in a mathematical 2 or 3 dimensional way.
    /// </summary>
    public ref class CoordinateReferenceSystem : ProjObject
    {
    private:
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateSystem^ m_cs;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        GeodeticCRS^ m_geodCRS;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        Proj::Ellipsoid^ m_ellipsoid;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        Proj::Datum^ m_datum;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        Proj::PrimeMeridian^ m_primeMeridian;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_baseCrs;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateTransform^ m_distanceTransform;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_promotedTo3D;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_demotedTo2D;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_axisNormalizedMe;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_nonNormalized;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        int m_axis;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_from;

        ~CoordinateReferenceSystem();

    internal:
        CoordinateReferenceSystem(ProjContext^ ctx, PJ* pj)
            : ProjObject(ctx, pj)
        {
        }


    public:
        property bool IsDeprecated
        {
            bool get()
            {
                return proj_is_deprecated(this) != 0;
            }
        }

        property bool IsDerived
        {
            bool get()
            {
                return proj_crs_is_derived(Context, this) != 0;
            }
        }

        property CoordinateSystem^ CoordinateSystem
        {
            Proj::CoordinateSystem^ get();
        }

    public:
        CoordinateReferenceSystem^ Clone([Optional]ProjContext^ ctx)
        {
            return static_cast<CoordinateReferenceSystem^>(__super::Clone(ctx));
        }

    public:
        //bool TryIdentify([Out] CoordinateReferenceIdList^% identified);

    public:
        property int AxisCount
        {
            virtual int get();
        internal:
            void set(int value);
        }

        property Proj::AxisCollection^ Axis
        {
            virtual Proj::AxisCollection^ get();
        }

        property Proj::Ellipsoid^ Ellipsoid
        {
            Proj::Ellipsoid^ get();
        }

        property Proj::GeodeticCRS^ GeodeticCRS
        {
            Proj::GeodeticCRS^ get();
        }

        property Proj::Datum^ Datum
        {
            Proj::Datum^ get();
        }

        property Proj::PrimeMeridian^ PrimeMeridian
        {
            Proj::PrimeMeridian^ get();
        }

        property CoordinateReferenceSystem^ BaseCRS
        {
            CoordinateReferenceSystem^ get();
        }

        property CoordinateTransform^ DistanceTransform
        {
            CoordinateTransform^ get();
        }

        CoordinateReferenceSystem^ PromotedTo3D();
        CoordinateReferenceSystem^ DemoteTo2D();

        property Proj::UsageArea^ UsageArea
        {
            virtual Proj::UsageArea^ get() override
            {
                Proj::UsageArea^ t = __super::UsageArea;

                if (!t && m_from)
                {
                    // Work around the issue that WithAxisNormalized() currently loses this information
                    double west, south, east, north;
                    const char* name;
                    if (proj_get_area_of_use(Context, m_from, &west, &south, &east, &north, &name))
                    {
                        __super::UsageArea = t = gcnew Proj::UsageArea(this, west, south, east, north, Utf8_PtrToString(name));
                    }
                }

                return t;
            }
        }

    public:
        CoordinateReferenceSystem^ WithAxisNormalized([Optional] ProjContext^ context);

    public:
        static CoordinateReferenceSystem^ Create(String^ from, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ Create(array<String^>^ from, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromWellKnownText(String^ from, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromDatabase(String^ authority, int code, [Optional] ProjContext^ ctx)
        {
            return CreateFromDatabase(authority, code.ToString(), ctx);
        }
        static CoordinateReferenceSystem^ CreateFromEpsg(int epsgCode, [Optional] ProjContext^ ctx)
        {
            return CreateFromDatabase("EPSG", epsgCode, ctx);
        }
    };
}
