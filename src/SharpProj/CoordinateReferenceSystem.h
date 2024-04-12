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

        ref class CoordinateMetadata;
        ref class CoordinateSystem;
        ref class UsageArea;
        interface class IHasCelestialBody;
        enum class CoordinateSystemType;
    }

    ref class CoordinateReferenceIdList;

    /// <summary>
    /// A coordinate reference system (CRS) refers to the way in which spatial data that represent the earth's surface (which is round / 3 dimensional) 
    /// is represented in a mathematical 2 or 3 dimensional way.
    /// </summary>
    public ref class CoordinateReferenceSystem : ProjObject, IHasCelestialBody
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
        IDisposable^ m_alsoDispose; // Used for ancestor 'variant' or self created-ctx
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        int m_axis;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_from;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        String^ m_celestialBodyName;

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
        CoordinateReferenceSystem^ Clone([Optional]ProjContext^ ctx) new
        {
            return static_cast<CoordinateReferenceSystem^>(__super::Clone(ctx));
        }

    public:
        /// <summary>
        /// Body on which this applies. Usually 'Earth'
        /// </summary>
        property String^ CelestialBodyName
        {
            virtual String^ get() sealed;
        }

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

        /// <summary>Transform from CRS to backing geodetic CRS with normalized axis</summary>
        [EditorBrowsable(EditorBrowsableState::Never)]
        property CoordinateTransform^ DistanceTransform
        {
            CoordinateTransform^ get();
        }

        CoordinateReferenceSystem^ PromotedTo3D();
        CoordinateReferenceSystem^ DemotedTo2D();

        property Proj::UsageArea^ UsageArea
        {
            virtual Proj::UsageArea^ get() override
            {
                Proj::UsageArea^ t = __super::UsageArea;

                if (!t && m_from)
                {
                    // Work around the issue that WithNormalizedAxis() currently loses this information
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

        property CoordinateSystemType CoordinateSystemType
        {
            Proj::CoordinateSystemType get();
        }

        property bool HasPointMotionOperation
        {
            bool get();
        }

    public:
        /// <summary>
        /// When called on an instance obtained from CoordinateRefenceSystem.DistanceTransform calculates the distance in meters
        /// Between p1 and p2 in meters calculating via the GeodeticCRS below the CoordinateReferenceSystem
        /// disregarding the height.
        /// </summary>
        /// <param name="p1"></param>
        /// <param name="p2"></param>
        /// <returns>Distance in meters or Double.NaN if unable to calculate</returns>
        double GeoDistance(PPoint p1, PPoint p2);

        double GeoDistance(System::Collections::Generic::IEnumerable<PPoint>^ points);

        /// <summary>
        /// When called on an instance obtained from CoordinateRefenceSystem.DistanceTransform calculates the distance in meters
        /// Between p1 and p2 in meters calculating via the GeodeticCRS below the CoordinateReferenceSystem
        /// disregarding the height.
        /// </summary>
        /// <param name="ordinates1"></param>
        /// <param name="ordinates2"></param>
        /// <returns>Distance in meters or Double.NaN if unable to calculate</returns>
        double GeoDistance(array<double>^ ordinates1, array<double>^ ordinates2);

        /// <summary>
        /// When called on an instance obtained from CoordinateRefenceSystem.DistanceTransform calculates the distance in meters
        /// Between p1 and p2 in meters calculating via the GeodeticCRS below the CoordinateReferenceSystem.
        /// After applying the distance over the ellipsoid the Z coordinate is applied in meters, as if the route was a simple straight line (using Pythagoras).
        /// </summary>
        /// <param name="p1"></param>
        /// <param name="p2"></param>
        /// <returns>Distance in meters or Double.NaN if unable to calculate</returns>
        double GeoDistanceZ(PPoint p1, PPoint p2);
        double GeoDistanceZ(System::Collections::Generic::IEnumerable<PPoint>^ points);

        /// <summary>
        /// When called on an instance obtained from CoordinateRefenceSystem.DistanceTransform calculates the distance in meters
        /// Between p1 and p2 in meters calculating via the GeodeticCRS below the CoordinateReferenceSystem.
        /// After applying the distance over the ellipsoid the Z coordinate is applied in meters, as if the route was a simple straight line (using Pythagoras).
        /// </summary>
        /// <param name="ordinates1"></param>
        /// <param name="ordinates2"></param>
        /// <returns>Distance in meters or Double.NaN if unable to calculate</returns>
        double GeoDistanceZ(array<double>^ ordinates1, array<double>^ ordinates2);
        PPoint Geod(PPoint p1, PPoint p2);
        array<double>^ Geod(array<double>^ ordinates1, array<double>^ ordinates2);

        /// <summary>
        /// When called on an instance obtained from CoordinateRefenceSystem.DistanceTransform calculates the area of the polygon defined by points in square meters
        /// </summary>
        double GeoArea(System::Collections::Generic::IEnumerable<PPoint>^ points);

    public:
        /// <summary>
        /// Gets this coordinate system, but now with axis normalized. If <paramref name="context" /> is not NULL, a new
        /// coordinate system within the specified context is returned. Otherwise the coordinate system itself or a coupled
        /// variant is returned. If a normalized variant is created disposing either the origin or the normalized version
        /// will also dispose the other.
        /// </summary>
        /// <param name="context"></param>
        /// <returns></returns>
        CoordinateReferenceSystem^ WithNormalizedAxis([Optional] ProjContext^ context);

        /// <summary>
        /// Gets this coordinate system, but now with axis normalized. If <paramref name="context" /> is not NULL, a new
        /// coordinate system within the specified context is returned. Otherwise the coordinate system itself or a coupled
        /// variant is returned. If a normalized variant is created disposing either the origin or the normalized version
        /// will also dispose the other.
        /// </summary>
        /// <param name="context"></param>
        /// <returns></returns>
        [EditorBrowsableAttribute(EditorBrowsableState::Never)]
        // Deprecated variant
        //[Obsolete("Please use .WithNormalizedAxis()")]
        CoordinateReferenceSystem^ WithAxisNormalized([Optional] ProjContext^ context)
        {
            return WithNormalizedAxis(context);
        }

    public:
        CoordinateMetadata^ CreateMetadata(double epoch);

    public:
        static CoordinateReferenceSystem^ Create(String^ from, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ Create(array<String^>^ from, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromWellKnownText(String^ from, [Optional] ProjContext^ ctx)
        {
            return CreateFromWellKnownText(from, nullptr, ctx);
        }
        static CoordinateReferenceSystem^ CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx)
        {
            return CreateFromWellKnownText(from, nullptr, warnings, ctx);
        }
        static CoordinateReferenceSystem^ CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx);
        static CoordinateReferenceSystem^ CreateFromDatabase(String^ authority, int code, [Optional] ProjContext^ ctx)
        {
            return CreateFromDatabase(authority, code.ToString(), ctx);
        }
        static CoordinateReferenceSystem^ CreateFromEpsg(int epsgCode, [Optional] ProjContext^ ctx)
        {
            return CreateFromDatabase("EPSG", epsgCode, ctx);
        }
        static CoordinateReferenceSystem^ CreateFromDatabase(Proj::Identifier^ identifier, [Optional] ProjContext^ ctx)
        {
            if ((Object^)identifier == nullptr)
                throw gcnew ArgumentNullException("identifier");

            return CreateFromDatabase(identifier->Authority, identifier->Code, ctx);
        }
    };
}
