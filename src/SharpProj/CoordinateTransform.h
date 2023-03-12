#pragma once
#include "ProjObject.h"
#include "CoordinateReferenceSystem.h"

extern "C" {
    struct geod_geodesic;
};

namespace SharpProj {
    ref class ChooseCoordinateTransform;
    ref class CoordinateTransform;
    ref class CoordinateReferenceSystem;
    ref class CoordinateArea;
    ref class CoordinateTransformOptions;
    ref class CoordinateOperation;
    ref class CoordinateTransformList;


    using System::Collections::ObjectModel::ReadOnlyCollection;
    using System::Collections::Generic::List;
    using System::ComponentModel::EditorBrowsableAttribute;
    using System::ComponentModel::EditorBrowsableState;

    namespace Proj {
        ref class ProjOperation;
        ref class ProjOperationList;
        ref class GridUsage;

        public ref class CoordinateTransformFactors
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_meridional_scale;               /* h */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_parallel_scale;                 /* k */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_areal_scale;                    /* s */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_angular_distortion;             /* omega */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_meridian_parallel_angle;        /* theta-prime */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_meridian_convergence;           /* alpha */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_tissot_semimajor;               /* a */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_tissot_semiminor;               /* b */
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_dx_dlam, m_dx_dphi;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_dy_dlam, m_dy_dphi;
        internal:
            CoordinateTransformFactors(const PJ_FACTORS* factors)
            {
                m_meridional_scale = factors->meridional_scale;
                m_parallel_scale = factors->parallel_scale;
                m_areal_scale = factors->areal_scale;
                m_angular_distortion = factors->angular_distortion;
                m_meridian_parallel_angle = factors->meridian_parallel_angle;
                m_meridian_convergence = factors->meridian_convergence;
                m_tissot_semimajor = factors->tissot_semimajor;
                m_tissot_semiminor = factors->tissot_semiminor;
                m_dx_dlam = factors->dx_dlam;
                m_dx_dphi = factors->dx_dphi;
                m_dy_dlam = factors->dy_dlam;
                m_dy_dphi = factors->dy_dphi;
            }

        public:
            property double MeridionalScale
            {
                double get() { return m_meridional_scale; }
            }
            property double ParallelScale
            {
                double get() { return m_parallel_scale; }
            }
            property double ArealScale
            {
                double get() { return m_areal_scale; }
            }
            property double AngularDistortion
            {
                double get() { return m_angular_distortion; }
            }
            property double MeridianParallelAngle
            {
                double get() { return m_meridian_parallel_angle; }
            }
            property double MeridianConvergence
            {
                double get() { return m_meridian_convergence; }
            }
            property double TissotSemimajor
            {
                double get() { return m_tissot_semimajor; }
            }
            property double TissotSemiminor
            {
                double get() { return m_tissot_semiminor; }
            }
            property double DxDlam
            {
                double get() { return m_dx_dlam; }
            }
            property double DxDphi
            {
                double get() { return m_dx_dphi; }
            }
            property double DyDlam
            {
                double get() { return m_dy_dlam; }
            }
            property double DyDphi
            {
                double get() { return m_dy_dphi; }
            }
        };

        [DebuggerDisplay("{Name,nq}={ValueString}")]
        public ref class CoordinateTransformParameter
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly CoordinateTransform^ m_op;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly int m_index;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_auth_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_code;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            double m_value;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_value_string;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            double m_unit_conv_factor;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_unit_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_unit_auth_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_unit_code;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_unit_category;

        internal:
            CoordinateTransformParameter(CoordinateTransform^ op, int index)
            {
                m_op = op;
                m_index = index;
            }

            void Ensure();

        public:
            property String^ Name
            {
                String^ get()
                {
                    Ensure();
                    return m_name;
                }
            }

            property String^ AuthName
            {
                String^ get()
                {
                    Ensure();
                    return m_auth_name;
                }
            }

            property String^ Code
            {
                String^ get()
                {
                    Ensure();
                    return m_code;
                }
            }

            property double Value
            {
                double get()
                {
                    Ensure();
                    return m_value;
                }
            }

            property String^ ValueString
            {
                String^ get()
                {
                    Ensure();
                    if (String::IsNullOrEmpty(m_value_string) && Value != 0.0)
                        m_value_string = Convert::ToString(Value);

                    return m_value_string;
                }
            }

            property double UnitConversionFactor
            {
                double get()
                {
                    Ensure();
                    return m_unit_conv_factor;
                }
            }

            property String^ UnitName
            {
                String^ get()
                {
                    Ensure();
                    return m_unit_name;
                }
            }

            property String^ UnitAuthName
            {
                String^ get()
                {
                    Ensure();
                    return m_unit_auth_name;
                }
            }

            property String^ UnitCode
            {
                String^ get()
                {
                    Ensure();
                    return m_unit_code;
                }
            }

            property String^ UnitCategory
            {
                String^ get()
                {
                    Ensure();
                    return m_unit_category;
                }
            }
        };
    }

    using CoordinateTransformParameter = Proj::CoordinateTransformParameter;

    /// <summary>
    /// The base of <see cref="ChooseCoordinateTransform" />, CoordinateOperation (private),
    /// <see cref="CoordinateTransformList" /> and <see cref="CoordinateTransformList" />
    /// </summary>
    public ref class CoordinateTransform abstract : ProjObject
    {
    private:
        enum class DistanceFlags
        {
            None = 0,
            Setup = 1,
            ApplyRad = 2
        };

        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        String^ m_methodName;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        ReadOnlyCollection<CoordinateTransformParameter^>^ m_params;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_source;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateReferenceSystem^ m_target;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        DistanceFlags m_distanceFlags;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        struct geod_geodesic* m_pgeod;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        ReadOnlyCollection<GridUsage^>^ m_gridUsages;


    protected:
        CoordinateTransform(ProjContext^ ctx, PJ* pj);

    private:
        ~CoordinateTransform();

    public:
        /// <summary>
        /// Transform a single coordinate
        /// </summary>
        /// <param name="coordinate"></param>
        /// <returns></returns>
        PPoint Apply(PPoint coordinate) { return DoTransform(true, coordinate); }
        /// <summary>
        /// Transform a single coordinate
        /// </summary>
        /// <param name="ordinates"></param>
        /// <returns></returns>
        array<double>^ Apply(...array<double>^ ordinates) { return DoTransform(true, PPoint(ordinates)).ToArray(); }
        /// <summary>
        /// Transform a single coordinate backwards
        /// </summary>
        /// <param name="coord"></param>
        /// <returns></returns>
        PPoint ApplyReversed(PPoint coord) { return DoTransform(false, coord); }
        /// <summary>
        /// Transform a single coordinate backwards
        /// </summary>
        /// <param name="ordinates"></param>
        /// <returns></returns>
        array<double>^ ApplyReversed(...array<double>^ ordinates) { return DoTransform(false, PPoint(ordinates)).ToArray(); }

        /// <summary>
        /// Transform a series of coordinates, where the individual coordinate dimension may be represented by an array that
        /// is either fully populated a null pointer and /or a length of zero, which will be treated as a fully populated array
        /// of zeroes of length one, i.e.a constant, which will be treated as a fully populated array of that constant value
        /// </summary>
        /// <remarks>Note that xStep, yStep, ... are in sizeof(double), not byte</remarks>
        [EditorBrowsableAttribute(EditorBrowsableState::Never)]
        void Apply(
            double* xVals, int xStep, int xCount,
            double* yVals, int yStep, int yCount,
            double* zVals, int zStep, int zCount,
            double* tVals, int tStep, int tCount);

        /// <summary>
        /// Transform a series of coordinates, where the individual coordinate dimension may be represented by an array that
        /// is either fully populated a null pointer and /or a length of zero, which will be treated as a fully populated array
        /// of zeroes of length one, i.e.a constant, which will be treated as a fully populated array of that constant value
        /// </summary>
        /// <remarks>Note that xStep, yStep, ... are in sizeof(double), not byte</remarks>
        [EditorBrowsableAttribute(EditorBrowsableState::Never)]
        void ApplyReversed(
            double* xVals, int xStep, int xCount,
            double* yVals, int yStep, int yCount,
            double* zVals, int zStep, int zCount,
            double* tVals, int tStep, int tCount);

        /// <summary>
        /// Transforms a series of coordinates specified as multiple lists of coordinates in-place, first for x, second for y, etc.
        /// </summary>
        /// <param name="ordinateArrays"></param>
        void Apply(...array<array<double>^>^ ordinateArrays);

        /// <summary>
        /// Transforms a series of coordinates specified as multiple lists of coordinates in-place
        /// </summary>
        /// <param name="ordinateArray"></param>
        void Apply(array<double, 2>^ ordinateArray);
 

        /// <summary>
        /// Transforms a series of coordinates specified as multiple lists of coordinates in-place, first for x, second for y, etc.
        /// </summary>
        /// <param name="ordinateArrays"></param>
        void ApplyReversed(...array<array<double>^>^ ordinateArrays);

        /// <summary>
        /// Transforms a series of coordinates specified as multiple lists of coordinates in-place
        /// </summary>
        /// <param name="ordinateArray"></param>
        void ApplyReversed(array<double, 2>^ ordinateArray);

    protected:
        /// <summary>
        /// Implements <see cref="Apply(PPoint)" /> and <see cref="ApplyReversed(PPoint)" />
        /// </summary>
        /// <param name="forward"></param>
        /// <param name="coordinate"></param>
        /// <returns></returns>
        virtual PPoint DoTransform(bool forward, PPoint% coordinate);
        /// <summary>
        /// Implements range Apply and ApplyReversed.
        /// </summary>
        virtual void DoTransform(bool forward,
            double* xVals, int xStep, int xCount,
            double* yVals, int yStep, int yCount,
            double* zVals, int zStep, int zCount,
            double* tVals, int tStep, int tCount);

    internal:
        PPoint FromCoordinate(const PJ_COORD& coord, bool forward);

    public:
        CoordinateTransform^ Clone([Optional]ProjContext^ ctx) new
        {
            return static_cast<CoordinateTransform^>(__super::Clone(ctx));
        }

    public:
        property bool HasInverse
        {
            virtual bool get()
            {
                PJ_PROJ_INFO info = proj_pj_info(this);
                return info.has_inverse;
            }
        }

        property ReadOnlyCollection<GridUsage^>^ GridUsages
        {
            virtual ReadOnlyCollection<GridUsage^>^ get();
        }

        property Nullable<double> Accuracy
        {
            Nullable<double> virtual get()
            {
                double d = proj_coordoperation_get_accuracy(Context, this);

                if (d == -1.0)
                    return Nullable<double>();
                else
                    return d;
            }
        }

        property virtual bool IsAvailable
        {
            bool get()
            {
                return 0 != proj_coordoperation_is_instantiable(Context, this);
            }
        }

        property bool HasBallParkTransformation
        {
            bool get()
            {
                return 0 != proj_coordoperation_has_ballpark_transformation(Context, this);
            }
        }

        property String^ MethodName
        {
            String^ get()
            {
                if (!m_methodName)
                {
                    const char* method_name;

                    if (proj_coordoperation_get_method_info(Context, this, &method_name, nullptr, nullptr))
                    {
                        m_methodName = Utf8_PtrToString(method_name);
                    }
                }

                return m_methodName;
            }
        }

        property ReadOnlyCollection<CoordinateTransformParameter^>^ Parameters
        {
            virtual ReadOnlyCollection<CoordinateTransformParameter^>^ get()
            {
                if (!m_params)
                {
                    int cnt = proj_coordoperation_get_param_count(Context, this);

                    array<CoordinateTransformParameter^>^ lst;
                    if (cnt > 0)
                    {
                        lst = gcnew array<CoordinateTransformParameter^>(cnt);

                        for (int i = 0; i < cnt; i++)
                            lst[i] = gcnew CoordinateTransformParameter(this, i);
                    }
                    else
                        lst = EMPTY_ARRAY(CoordinateTransformParameter^);

                    m_params = Array::AsReadOnly(lst);
                }
                return m_params;
            }
        }

    public:
        virtual CoordinateTransform^ CreateInverse([Optional]ProjContext^ ctx)
        {
            UNUSED_ALWAYS(ctx);
            throw gcnew InvalidOperationException();
        }

    public:
        property CoordinateReferenceSystem^ SourceCRS
        {
            CoordinateReferenceSystem^ get();
        }

        property CoordinateReferenceSystem^ TargetCRS
        {
            CoordinateReferenceSystem^ get();
        }

    internal:
        void EnsureDistance()
        {
            if (m_distanceFlags != DistanceFlags::None)
                return;

            SetupDistance();
        }
    public:
        void SetupDistance();

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
        double GeoDistance(array<double>^ ordinates1, array<double>^ ordinates2) { return GeoDistance(PPoint(ordinates1), PPoint(ordinates2)); }
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
        double GeoDistanceZ(array<double>^ ordinates1, array<double>^ ordinates2) { return GeoDistanceZ(PPoint(ordinates1), PPoint(ordinates2)); }
        PPoint Geod(PPoint p1, PPoint p2);
        array<double>^ Geod(array<double>^ ordinates1, array<double>^ ordinates2) { return Geod(PPoint(ordinates1), PPoint(ordinates2)).ToArray(); }

        /// <summary>
        /// When called on an instance obtained from CoordinateRefenceSystem.DistanceTransform calculates the area of the polygon defined by points in square meters
        /// </summary>
        double GeoArea(System::Collections::Generic::IEnumerable<PPoint>^ points);

    private protected:
        virtual ProjObject^ DoClone(ProjContext^ ctx) override;

    public:
        static CoordinateTransform^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateTransformOptions^ options, [Optional] ProjContext^ ctx);
        static CoordinateTransform^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateArea^ area, [Optional] ProjContext^ ctx)
        {
            auto opts = gcnew CoordinateTransformOptions();
            opts->Area = area;

            return CoordinateTransform::Create(sourceCrs, targetCrs, opts, ctx);
        }
        static CoordinateTransform^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, [Optional] ProjContext^ ctx)
        {
            return CoordinateTransform::Create(sourceCrs, targetCrs, gcnew CoordinateTransformOptions(), ctx);
        }

        static CoordinateTransform^ Create(String^ from, [Optional] ProjContext^ ctx);
        static CoordinateTransform^ Create(array<String^>^ definition, [Optional] ProjContext^ ctx);
        static CoordinateTransform^ CreateFromDatabase(String^ authority, String^ code, [Optional] ProjContext^ ctx);
        static CoordinateTransform^ CreateFromDatabase(String^ authority, int code, [Optional] ProjContext^ ctx)
        {
            return CreateFromDatabase(authority, code.ToString(), ctx);
        }
        static CoordinateTransform^ CreateFromEpsg(int epsgCode, [Optional] ProjContext^ ctx)
        {
            return CreateFromDatabase("EPSG", epsgCode, ctx);
        }
        static CoordinateTransform^ CreateFromDatabase(Proj::Identifier^ identifier, [Optional] ProjContext^ ctx)
        {
            if (!(Object^)identifier)
                throw gcnew ArgumentNullException("identifier");

            return CreateFromDatabase(identifier->Authority, identifier->Code, ctx);
        }

        static CoordinateTransform^ CreateFromWellKnownText(String^ from, [Optional] ProjContext^ ctx)
        {
            return CreateFromWellKnownText(from, nullptr, ctx);
        }
        static CoordinateTransform^ CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Optional] ProjContext^ ctx);
        static CoordinateTransform^ CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx)
        {
            return CreateFromWellKnownText(from, nullptr, warnings, ctx);
        }
        static CoordinateTransform^ CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx);

        static CoordinateTransform^ CreateSingle(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateTransformOptions^ options, [Optional] ProjContext^ ctx);

    public:
        /// <summary>
        /// Measure internal consistency of a given transformation. The function performs n round trip transformations starting in either the forward or reverse direction. Returns the euclidean distance of the starting point coo and the resulting coordinate after n iterations back and forth.
        /// </summary>
        /// <param name="forward"></param>
        /// <param name="transforms"></param>
        /// <param name="coordinate"></param>
        /// <returns></returns>
        double RoundTrip(bool forward, int transforms, PPoint coordinate);
        /// <summary>
        /// Measure internal consistency of a given transformation. The function performs n round trip transformations starting in either the forward or reverse direction. Returns the euclidean distance of the starting point coo and the resulting coordinate after n iterations back and forth.
        /// </summary>
        /// <param name="forward"></param>
        /// <param name="transforms"></param>
        /// <param name="ordinates"></param>
        /// <returns></returns>
        double RoundTrip(bool forward, int transforms, array<double>^ ordinates) { return RoundTrip(forward, transforms, PPoint(ordinates)); }
        /// <summary>
        /// Calculate various cartographic properties, such as scale factors, angular distortion and meridian convergence. Depending on the underlying projection values will be calculated either numerically (default) or analytically.
        /// </summary>
        /// <param name="coordinate"></param>
        /// <returns></returns>
        Proj::CoordinateTransformFactors^ Factors(PPoint coordinate);
        /// <summary>
        /// Calculate various cartographic properties, such as scale factors, angular distortion and meridian convergence. Depending on the underlying projection values will be calculated either numerically (default) or analytically.
        /// </summary>
        /// <param name="ordinates"></param>
        /// <returns></returns>
        Proj::CoordinateTransformFactors^ Factors(array<double>^ ordinates) { return Factors(PPoint(ordinates)); }


    public:
        literal double DegreesToRadians = (2.0 * System::Math::PI / 360.0);
        literal double RadiansToDegrees = (360.0 / System::Math::PI / 2.0);
        // Some helpers that do not really belong here, but are easy to access in a sensible way
        // if they are here anyway
        static double ToRad(double deg)
        {
            return deg * DegreesToRadians; // proj_torad(deg); // Inlineable by .Net CLR
        }

        static double ToDeg(double rad)
        {
            return rad * RadiansToDegrees; //return proj_todeg(rad); // Inlineable by .Net CLR
        }

        static double ApplyAccuracy(double value, double accuracy)
        {
            if (accuracy <= 0)
                return value;

            double acc = Math::Log10(accuracy);

            if (acc <= 0)
                return Math::Round(value, (int)-acc);
            else
            {
                double f = Math::Pow(10, (int)acc);

                return f * Math::Round(value / f);
            }
        }

    public:
        virtual IReadOnlyList<CoordinateTransform^>^ Options()
        {
            return Array::AsReadOnly(gcnew array<CoordinateTransform^> { this });
        }

        virtual IReadOnlyList<CoordinateTransform^>^ Steps()
        {
            return Array::AsReadOnly(gcnew array<CoordinateTransform^> { this });
        }

        virtual IReadOnlyList<ProjOperation^>^ ProjOperations();
    };

    // For now internal. Could be made public
    ref class CoordinateOperation : CoordinateTransform
    {
    internal:
        CoordinateOperation(ProjContext^ ctx, PJ* pj)
            : CoordinateTransform(ctx, pj)
        {

        }

    public:
        virtual CoordinateTransform^ CreateInverse([Optional]ProjContext^ ctx) override
        {
            if (!ctx)
                ctx = Context;

            if (!HasInverse)
                throw gcnew InvalidOperationException();

            return gcnew CoordinateOperation(ctx, proj_coordoperation_create_inverse(ctx, this));
        }
    };

}