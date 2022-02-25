#pragma once
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
        ref class GeoidModelInfo;

        /// <summary>
        /// Container for CoordinateReference system information obtained from proj.db via <see cref="ProjContext"/>.GetCoordinateReferenceSystems
        /// </summary>
        [DebuggerDisplay("[{Identifier}] {Name,nq}")]
        public ref class CoordinateReferenceSystemInfo
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly ProjContext^ _ctx;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _authName;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _code;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly ProjType _type;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly bool _deprecated;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _areaName;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _projectionName;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _celestialBodyName;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly ProjArea^ _bbox;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            ReadOnlyCollection<GeoidModelInfo^>^ _geoidModels;

        internal:
            CoordinateReferenceSystemInfo(const PROJ_CRS_INFO* info, ProjContext^ ctx)
            {
                _ctx = ctx;
                _authName = Utf8_PtrToString(info->auth_name);
                _code = Utf8_PtrToString(info->code);
                _name = Utf8_PtrToString(info->name);
                _type = (ProjType)info->type;
                _deprecated = (0 != info->deprecated);
                _areaName = Utf8_PtrToString(info->area_name);
                _projectionName = Utf8_PtrToString(info->projection_method_name);
                _celestialBodyName = Utf8_PtrToString(info->celestial_body_name);
                if (info->bbox_valid)
                    _bbox = gcnew ProjArea(info->west_lon_degree, info->south_lat_degree, info->east_lon_degree, info->north_lat_degree);
            }

        public:
            property String^ Authority
            {
                String^ get()
                {
                    return _authName;
                }
            }

            property String^ Code
            {
                String^ get()
                {
                    return _code;
                }
            }

            property Identifier^ Identifier
            {
                Proj::Identifier^ get()
                {
                    return gcnew Proj::Identifier(Authority, Code);
                }
            }

            property String^ Name
            {
                String^ get()
                {
                    return _name;
                }
            }

            property ProjType Type
            {
                ProjType get()
                {
                    return _type;
                }
            }

            property bool IsDeprecated
            {
                bool get()
                {
                    return _deprecated;
                }
            }

            property String^ AreaName
            {
                String^ get()
                {
                    return _areaName;
                }
            }

            property String^ ProjectionName
            {
                String^ get()
                {
                    return _projectionName;
                }
            }

            /// <summary>
            /// Body on which this crs applies. Usually 'Earth'
            /// </summary>
            property String^ CelestialBodyName
            {
                String^ get()
                {
                    return _celestialBodyName;
                }
            }

            property ProjArea^ BoundingBox
            {
                virtual ProjArea^ get() sealed
                {
                    return _bbox;
                }
            }

            virtual String^ ToString() override
            {
                return Name;
            }

            CoordinateReferenceSystem^ Create([Optional] ProjContext^ ctx);

            // not a property, as this is prepared for a future args object
            ReadOnlyCollection<GeoidModelInfo^>^ GetGeoidModels();
        };

        public ref class CoordinateReferenceSystemFilter
        {
            initonly List<ProjType>^ _types;

        public:
            CoordinateReferenceSystemFilter()
            {
                _types = gcnew List<ProjType>();
            }

            property String^ Authority;

            property List<ProjType>^ Types
            {
                List<ProjType>^ get()
                {
                    return _types;
                }
            }

            property CoordinateArea^ CoordinateArea;

            /// <summary>
            /// Gets or sets a boolean indicating whether <see cref="CoordinateArea"/> must be completely inside the usage area of the <see cref="CoordinateReferenceSystem" />.
            /// False implies that the area must (just) intersect the area.
            /// </summary>
            property bool CompletelyContainsArea;


            property bool AllowDeprecated;

            /// <summary>
            /// Body on which this crs applies. Usually 'Earth'
            /// </summary>
            property String^ CelestialBodyName;
        };

        [DebuggerDisplay("{Name,nq} ({Authority,nq})")]
        public ref class CelestialBodyInfo
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _authName;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _name;

        internal:
            CelestialBodyInfo(const PROJ_CELESTIAL_BODY_INFO* body)
            {
                _authName = Utf8_PtrToString(body->auth_name);
                _name = Utf8_PtrToString(body->name);
            }

        public:
            property String^ Authority
            {
                String^ get()
                {
                    return _authName;
                }
            }

            property String^ Name
            {
                String^ get()
                {
                    return _name;
                }
            }

            property bool IsEarth
            {
                bool get()
                {
                    return Name == "Earth" && Authority == "PROJ";
                }
            }

            virtual String^ ToString() override
            {
                return Name;
            }
        };

        [DebuggerDisplay("{Authority,nq}: {Name,nq}")]
        public ref class GeoidModelInfo
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _authName;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ _name;

        internal:
            GeoidModelInfo(String^ authName, String^ name)
            {
                _authName = authName;
                _name = name;
            }

        public:
            property String^ Authority
            {
                String^ get()
                {
                    return _authName;
                }
            }

            property String^ Name
            {
                String^ get()
                {
                    return _name;
                }
            }

            virtual String^ ToString() override
            {
                return Name;
            }
        };

        [DebuggerDisplay("[{Identifier}] {Name,nq}")]
        public ref class UnitOfMeasurement
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly Identifier^ m_identifier;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ m_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ m_shortName;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ m_category;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly double m_conversionFactor;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly bool m_deprecated;

        internal:
            UnitOfMeasurement(const PROJ_UNIT_INFO* unit_info)
            {
                m_identifier = gcnew Proj::Identifier(Utf8_PtrToString(unit_info->auth_name), Utf8_PtrToString(unit_info->code));
                m_name = Utf8_PtrToString(unit_info->name);
                m_shortName = Utf8_PtrToString(unit_info->proj_short_name);
                m_category = Utf8_PtrToString(unit_info->category);
                m_conversionFactor = unit_info->conv_factor;
                m_deprecated = (unit_info->deprecated != 0);
            }

        public:
            property Identifier^ Identifier
            {
                Proj::Identifier^ get()
                {
                    return m_identifier;
                }
            }

            property String^ Name
            {
                String^ get()
                {
                    return m_name;
                }
            }

            property String^ ShortName
            {
                String^ get()
                {
                    return m_shortName;
                }
            }

            property String^ Category
            {
                String^ get()
                {
                    return m_category;
                }
            }

            property double ConversionFactor
            {
                double get()
                {
                    return m_conversionFactor;
                }
            }

            property bool IsDeprecated
            {
                bool get()
                {
                    return m_deprecated;
                }
            }
        };

        public ref class UnitOfMeasurementFilter
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_authority;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_category;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            bool m_noDeprecated;

        public:
            property String^ Authority
            {
                String^ get()
                {
                    return m_authority;
                }
                void set(String^ value)
                {
                    m_authority = value;
                }
            }

            property String^ Category
            {
                String^ get()
                {
                    return m_category;
                }
                void set(String^ value)
                {
                    m_category = value;
                }
            }

            property bool NoDeprecated
            {
                bool get()
                {
                    return m_noDeprecated;
                }
                void set(bool value)
                {
                    m_noDeprecated = value;
                }
            }
        };
    }
}