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
			initonly LatitudeLongitudeArea^ _bbox;

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
				if (info->bbox_valid)
					_bbox = gcnew LatitudeLongitudeArea(info->west_lon_degree, info->south_lat_degree, info->east_lon_degree, info->north_lat_degree);
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

			property LatitudeLongitudeArea^ BoundingBox
			{
				virtual LatitudeLongitudeArea^ get() sealed
				{
					return _bbox;
				}
			}

			virtual String^ ToString() override
			{
				return Name;
			}

			CoordinateReferenceSystem^ Create([Optional] ProjContext^ ctx);
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
		};
	}
}