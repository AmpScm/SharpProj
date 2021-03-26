#pragma once
#include <proj.h>

#include "ProjContext.h"
#include "CoordinateArea.h"
#include "PPoint.h"
#include "UsageArea.h"

namespace SharpProj {
	using System::Collections::Generic::IReadOnlyList;
	using System::Diagnostics::DebuggerBrowsableAttribute;
	using System::Diagnostics::DebuggerBrowsableState;
	value class PPoint;

	namespace Proj {
		ref class ProjObject;

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

		[DebuggerDisplay("{String(),nq}")]
		public ref class Identifier sealed
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

			Identifier(String^ authority, String^ code)
			{
				if (!authority)
					throw gcnew ArgumentNullException("authority");
				if (!code)
					throw gcnew ArgumentNullException("code");

				m_authority = authority;
				m_code = code;
			}

		public:
			property String^ Authority
			{
				String^ get();
			}

			property String^ Code
			{
				String^ get();
			}

			[Obsolete("Use .Code"), DebuggerBrowsable(DebuggerBrowsableState::Never)]
			property String^ Name
			{
				String^ get() { return Code; }
			}

			virtual String^ ToString() override
			{
				return Authority + ":" + Code;
			}

			virtual bool Equals(Object^ other) override
			{
				auto idOther = dynamic_cast<Identifier^>(other);

				if (!idOther)
					return false;
				else
					return idOther->Authority == Authority && idOther->Name == Name;
			}

			virtual int GetHashCode() override
			{
				return Code ? Code->GetHashCode() : 0;
			}
		};

		public enum class WktType
		{
			WKT2_2015 = PJ_WKT2_2015,
			WKT2_2015_Simplified = PJ_WKT2_2015_SIMPLIFIED,
			WKT2_2019 = PJ_WKT2_2019,
			WKT2_2018 = PJ_WKT2_2018, // Alias for 2019 (via PROJ)
			WKT2_2019_Simplified = PJ_WKT2_2019_SIMPLIFIED,
			WKT2_2018_Simplified = PJ_WKT2_2018_SIMPLIFIED, // Alias for 2019 (via PROJ)
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

		[DebuggerDisplay("Count = {Count}")]
		public ref class IdentifierList : IReadOnlyList<Identifier^>
		{
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			initonly ProjObject^ m_object;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
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

		

		[DebuggerDisplay("[{Type}] {ToString(),nq}")]
		public ref class ProjObject
		{
		private:
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			ProjContext^ m_ctx;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			PJ* m_pj;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			String^ m_infoId;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			String^ m_name;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			String^ m_infoDefinition;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			String^ m_scope;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			Proj::IdentifierList^ m_idList;
			[DebuggerBrowsable(DebuggerBrowsableState::Never)]
			bool m_noProj;

		private:
			~ProjObject();
			!ProjObject();

		private protected:
			void ForceUnknownInfo()
			{
				m_infoId = "?";
				m_name = "?";
				m_infoDefinition = "?";
				m_scope = "?";
				m_noProj = true;
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

		internal:
			static array<String^>^ FromStringList(PROJ_STRING_LIST lst)
			{
				if (!lst || !*lst)
					return Array::Empty<String^>();

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
					return m_ctx;
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
						const char* name = proj_get_name(this);
						m_name = name ? gcnew System::String(name) : nullptr;
					}
					return m_name;
				}
			protected private:
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
			protected private:
				void set(String^ value)
				{
					m_infoDefinition = value;
				}
			}

			property String^ Scope
			{
				String^ get()
				{
					if (!m_scope)
					{
						const char* scope = proj_get_scope(this);

						m_scope = Utf8_PtrToString(scope);
					}
					return m_scope;
				}
			protected private:
				void set(String^ value)
				{
					m_scope = value;
				}
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
					double west, south, east, north;
					const char* name;
					if (proj_get_area_of_use(Context, this, &west, &south, &east, &north, &name))
					{
						return gcnew Proj::UsageArea(this, west, south, east, north, Utf8_PtrToString(name));
					}
					else
						return nullptr;
				}
			}

			String^ AsProjJson()
			{
				if (m_noProj)
					return nullptr;

				const char* v = proj_as_projjson(Context, this, nullptr);

				return Utf8_PtrToString(v);
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

				const char* v = proj_as_wkt(Context, this, PJ_WKT2_2019, opts);

				return Utf8_PtrToString(v);
			}

			String^ AsWellKnownText()
			{
				return AsWellKnownText(nullptr);
			}

			String^ AsProjString(ProjStringOptions ^options)
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
			property Proj::IdentifierList^ Identifiers
			{
				Proj::IdentifierList^ get();
			}

			bool IsEquivalentTo(ProjObject^ other, [Optional] ProjContext^ ctx)
			{
				if (!other || m_noProj || other->m_noProj)
					return false;

				if (!ctx)
					ctx = Context;

				return 0 != proj_is_equivalent_to_with_ctx(ctx, this, other, PJ_COMP_EQUIVALENT);
			}

			bool IsEquivalentToRelaxed(ProjObject^ other, [Optional] ProjContext^ ctx)
			{
				if (!other || other->m_noProj)
					return false;

				if (!ctx)
					ctx = Context;

				return 0 != proj_is_equivalent_to_with_ctx(ctx, this, other, PJ_COMP_EQUIVALENT_EXCEPT_AXIS_ORDER_GEOGCRS);
			}

		public:
			static ProjObject^ Create(String^ definition, [Optional]ProjContext^ ctx);
			static ProjObject^ Create(array<String^>^ from, [Optional]ProjContext^ ctx);

			static ProjObject^ CreateFromWellKnownText(String^ from, [Optional] ProjContext^ ctx);
			static ProjObject^ CreateFromWellKnownText(String^ from, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx);

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