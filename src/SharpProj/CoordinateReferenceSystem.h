#pragma once
#include "ProjObject.h"

namespace SharpProj {
	using System::Collections::ObjectModel::ReadOnlyCollection;
	using System::Collections::Generic::List;
	ref class CoordinateTransform;

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

		public ref class CoordinateReferenceSystemInfo
		{
			initonly ProjContext^ _ctx;
			initonly String^ _authName;
			initonly String^ _code;
			initonly ProjType _type;
			initonly bool _deprecated;
			initonly String^ _areaName;
			initonly String^ _projectionName;

		internal:
			CoordinateReferenceSystemInfo(const PROJ_CRS_INFO* info, ProjContext^ ctx)
			{
				_ctx = ctx;
				_authName = Utf8_PtrToString(info->auth_name);
				_code = Utf8_PtrToString(info->code);
				_type = (ProjType)info->type;
				_deprecated = (0 != info->deprecated);
				_areaName = Utf8_PtrToString(info->area_name);
				_projectionName = Utf8_PtrToString(info->projection_method_name);
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

			property bool AllowDeprecated;

		};
	}

	/// <summary>
	/// A coordinate reference system (CRS) refers to the way in which spatial data that represent the earth’s surface (which is round / 3 dimensional) 
	/// is represented in a mathematical 2 or 3 dimensional way.
	/// </summary>
	public ref class CoordinateReferenceSystem : ProjObject
	{
	private:
		ProjContext^ m_ctx;
		CoordinateSystem^ m_cs;
		GeodeticCRS^ m_geodCRS;
		Proj::Ellipsoid^ m_ellipsoid;
		Proj::Datum^ m_datum;
		Proj::PrimeMeridian^ m_primeMeridian;
		CoordinateReferenceSystem^ m_baseCrs;
		CoordinateTransform^ m_distanceTransform;
		CoordinateReferenceSystem^ m_promotedTo3D;
		CoordinateReferenceSystem^ m_demotedTo2D;
		WeakReference<Proj::UsageArea^>^ m_usageArea;
		int m_axis;

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
				Proj::UsageArea^ t = nullptr;

				if (m_usageArea && m_usageArea->TryGetTarget(t))
					return t;
				else
				{
					t = __super::UsageArea;
					if (t)
						m_usageArea = gcnew WeakReference<Proj::UsageArea^>(t);
					else
						m_usageArea = nullptr;

					return t;
				}
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
			return CoordinateReferenceSystem::CreateFromDatabase(authority, code.ToString(), ctx);
		}
	};
}
