#pragma once
#include "ProjContext.h"
#include "CoordinateArea.h"
#include "PPoint.h"

namespace SharpProj {
	using System::Collections::Generic::IReadOnlyList;
	ref class ProjObject;

	namespace Details {
		public enum class ProjType
		{
			Unknown = PJ_TYPE_UNKNOWN,

			Ellipsoid = PJ_TYPE_ELLIPSOID,

			PrimeMeridian = PJ_TYPE_PRIME_MERIDIAN,

			GeodeticReferenceFrame = PJ_TYPE_GEODETIC_REFERENCE_FRAME,
			DynamicGeodeticReferenceFrame = PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME,
			VerticalReferenceFrame = PJ_TYPE_VERTICAL_REFERENCE_FRAME,
			DynamicVerticalReferenceFrame = PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME,
			DatumEnsamble = PJ_TYPE_DATUM_ENSEMBLE,

			/** Abstract type, not returned by proj_get_type() */
			CRS = PJ_TYPE_CRS,

			GeodeticCrs = PJ_TYPE_GEODETIC_CRS,
			GeocentricCrs = PJ_TYPE_GEOCENTRIC_CRS,

			/** proj_get_type() will never return that type, but
			 * PJ_TYPE_GEOGRAPHIC_2D_CRS or PJ_TYPE_GEOGRAPHIC_3D_CRS. */
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

		public ref class Identifier
		{
		internal:
			initonly ProjObject^ m_object;
			initonly int m_index;
			String^ m_authority;
			String^ m_code;

			Identifier(ProjObject^ object, int index)
			{
				m_object = object;
				m_index = index;
			}
		public:
			property String^ Authority
			{
				String^ get();
			}

			property String^ Name
			{
				String^ get();
			}
		};

		[System::Diagnostics::DebuggerDisplayAttribute("Count = {Count}")]
		public ref class IdentifierList : IReadOnlyList<Identifier^>
		{
			initonly ProjObject^ m_object;
			array<Identifier^>^ m_Items;

		internal:
			IdentifierList(ProjObject^ obj)
			{
				if (!obj)
					throw gcnew ArgumentNullException("obj");

				m_object = obj;
			}

		private:
			virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
			{
				return GetEnumerator();
			}

		public:
			// Inherited via IReadOnlyCollection
			virtual System::Collections::Generic::IEnumerator<Identifier^>^ GetEnumerator();

			// Inherited via IReadOnlyList
			virtual property int Count
			{
				int get();
			}
			virtual property Identifier^ default[int]
			{
				Identifier ^ get(int index);
			}
		};
	}

	[System::Diagnostics::DebuggerDisplayAttribute("[{Type}] {ToString(),nq}")]
	public ref class ProjObject
	{
	protected:
		ProjContext^ m_ctx;
		PJ* m_pj;
		String^ m_infoId;
		String^ m_name;
		String^ m_infoDefinition;
		String^ m_scope;
		Details::IdentifierList^ m_idList;

	private:
		~ProjObject()
		{
			if (m_pj)
			{
				try
				{
					proj_destroy(m_pj);
				}
				finally
				{
					m_pj = nullptr;
				}
			}
		}

	private protected:
		void ForceUnknownInfo()
		{
			m_infoId = "?";
			m_name = "?";
			m_infoDefinition = "?";
			m_scope = "?";
		}

	internal:
		ProjObject(ProjContext^ ctx, PJ* pj)
		{
			if (!ctx)
				throw gcnew ArgumentNullException("ctx");
			else if (!pj)
				throw gcnew ArgumentNullException("pj");

			m_ctx = ctx;
			m_pj = pj;
		}

		static operator PJ* (ProjObject^ pj)
		{
			if ((Object^)pj == nullptr)
				return nullptr;
			else if (pj->m_pj == nullptr)
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

	public:
		property ProjContext^ Context
		{
			ProjContext^ get()
			{
				return m_ctx;
			}
		}

		ProjObject^ Clone(ProjContext^ ctx)
		{
			if (!ctx)
				throw gcnew ArgumentNullException("ctx");

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
					const char *name = proj_get_name(this);
					m_name = name ? gcnew System::String(name) : nullptr;
				}
				return m_name;
			}
		internal:
			void set(String^ value)
			{
				m_name = value;
			}
		}

		property String^ Definition
		{
			String^ get()
			{
				if (!m_infoDefinition)
				{
					PJ_PROJ_INFO info = proj_pj_info(this);
					m_infoDefinition = gcnew System::String(info.definition);
				}
				return m_infoDefinition;
			}
		}

		property String^ Scope
		{
			String^ get()
			{
				if (!m_scope)
				{
					const char* scope = proj_get_scope(this);

					if (scope)
						m_scope = gcnew String(scope);
				}
				return m_scope;
			}
		}

		property Details::ProjType Type
		{
			virtual Details::ProjType get()
			{
				return (Details::ProjType)proj_get_type(this);
			}
		}

		property CoordinateArea^ UsageArea
		{
			CoordinateArea^ get()
			{
				double west, south, east, north;
				const char* name;
				if (proj_get_area_of_use(Context, this, &west, &south, &east, &north, &name))
				{
					return gcnew CoordinateArea(west, south, east, north, name ? gcnew String(name) : nullptr);
				}
				else
					return nullptr;
			}
		}

		String^ AsProjJson()
		{
			const char* v = proj_as_projjson(Context, this, nullptr);

			return v ? gcnew String(v) : nullptr;
		}

		String^ AsWellKnownText()
		{
			const char* v = proj_as_wkt(Context, this, PJ_WKT2_2019, nullptr);

			return v ? gcnew String(v) : nullptr;
		}

		String^ AsProjString()
		{
			const char* v = proj_as_proj_string(Context, this, PJ_PROJ_5/* Last as of 2021-01 */, nullptr);

			return v ? gcnew String(v) : nullptr;
		}

	public:
		property Details::IdentifierList^ Identifiers
		{
			Details::IdentifierList^ get();
		}

		bool IsEquivalentTo(ProjObject^ other, [Optional] ProjContext^ ctx)
		{
			if (!other)
				return false;

			if (!ctx)
				ctx = Context;

			return 0 != proj_is_equivalent_to_with_ctx(ctx, this, other, PJ_COMP_EQUIVALENT);
		}

		bool IsEquivalentToRelaxed(ProjObject^ other, [Optional] ProjContext^ ctx)
		{
			if (!other)
				return false;

			if (!ctx)
				ctx = Context;

			return 0 != proj_is_equivalent_to_with_ctx(ctx, this, other, PJ_COMP_EQUIVALENT_EXCEPT_AXIS_ORDER_GEOGCRS);
		}

	public:
		static ProjObject^ Create(String^ definition, [Optional]ProjContext^ ctx);
		static ProjObject^ Create(array<String^>^ from, [Optional]ProjContext^ ctx);

	private protected:
		void SetCoordinate(PJ_COORD& coord, PPoint %coordinate)
		{
			coord.v[0] = coordinate.X;
			coord.v[1] = coordinate.Y;
			coord.v[2] = coordinate.Z;
			coord.v[3] = coordinate.T;
		}		
	};
}