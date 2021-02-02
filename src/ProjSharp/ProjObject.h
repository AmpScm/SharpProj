#pragma once
#include "ProjContext.h"
#include "ProjArea.h"

namespace ProjSharp {
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
		 OtherCoordinateOperation = PJ_TYPE_OTHER_COORDINATE_OPERATION,

		 TemporalDatum = PJ_TYPE_TEMPORAL_DATUM,
		 EngineeringDatum = PJ_TYPE_ENGINEERING_DATUM,
		 ParametricDatum = PJ_TYPE_PARAMETRIC_DATUM,
	};


	[System::Diagnostics::DebuggerDisplayAttribute("{Description}")]
	public ref class ProjObject
	{
	protected:
		ProjContext^ m_ctx;
		PJ* m_pj;
		String^ m_infoId;
		String^ m_infoDescription;
		String^ m_infoDefinition;
		String^ m_scope;

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

	public:
		virtual String^ ToString() override
		{
			return Description;
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

			return ctx->Create(proj_clone(ctx, this));
		}

		property String^ Id
		{
			String^ get()
			{
				if (!m_infoId)
				{
					PJ_PROJ_INFO info = proj_pj_info(this);
					m_infoId = gcnew System::String(info.id);
				}
				return m_infoId;
			}
		}

		property String^ Description
		{
			String^ get()
			{
				if (!m_infoDescription)
				{
					PJ_PROJ_INFO info = proj_pj_info(this);
					m_infoDescription = gcnew System::String(info.description);
				}
				return m_infoDescription;
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

		property double Accuraracy
		{
			double virtual get()
			{
				PJ_PROJ_INFO info = proj_pj_info(this);
				return info.accuracy;
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

		property ProjType Type
		{
			ProjType get()
			{
				return (ProjType)proj_get_type(this);
			}
		}

		property ProjArea^ UsageArea
		{
			ProjArea^ get()
			{
				double west, south, east, north;
				const char* name;
				if (proj_get_area_of_use(Context, this, &west, &south, &east, &north, &name))
				{
					return gcnew ProjArea(west, south, east, north, name ? gcnew String(name) : nullptr);
				}
				else
					return nullptr;
			}
		}

		String^ AsJson()
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
		static ProjObject^ Create(String^ definition, [Optional]ProjContext^ ctx);
		static ProjObject^ Create(array<String^>^ from, [Optional]ProjContext^ ctx);

	private protected:
		void SetCoordinate(PJ_COORD& coord, array<double>^ coordinate)
		{
			coord.v[0] = coordinate[0];
			coord.v[1] = coordinate[1];
			if (coordinate->Length > 2)
			{
				coord.v[2] = coordinate[2];
				if (coordinate->Length > 3)
					coord.v[3] = coordinate[3];
			}
		}

		array<double>^ FromCoordinate(const PJ_COORD& coord, int len)
		{
			array<double>^ r = gcnew array<double>(len);
			r[0] = coord.v[0];
			r[1] = coord.v[1];
			if (r->Length > 2)
			{
				r[2] = coord.v[2];
				if (r->Length > 3)
					r[3] = coord.v[3];
			}

			return r;
		}
	};
}