#pragma once
#include "ProjObject.h"
namespace SharpProj {
	namespace Details {

		using System::Collections::ObjectModel::ReadOnlyCollection;
		using System::Collections::Generic::List;


		ref class CoordinateSystem;

		public enum class CoordinateSystemType
		{
			Unknown = PJ_CS_TYPE_UNKNOWN,

			Cartesian = PJ_CS_TYPE_CARTESIAN,
			Ellipsoidal = PJ_CS_TYPE_ELLIPSOIDAL,
			Vertical = PJ_CS_TYPE_VERTICAL,
			Spherical = PJ_CS_TYPE_SPHERICAL,
			Ordinal = PJ_CS_TYPE_ORDINAL,
			Parametric = PJ_CS_TYPE_PARAMETRIC,
			DateTimeTemporal = PJ_CS_TYPE_DATETIMETEMPORAL,
			TemporalCount = PJ_CS_TYPE_TEMPORALCOUNT,
			TemporalMeasure = PJ_CS_TYPE_TEMPORALMEASURE
		};

		[System::Diagnostics::DebuggerDisplayAttribute("{Name}")]
		public ref class Axis
		{
			initonly CoordinateSystem^ m_cs;
			initonly int m_idx;
			String^ m_name;
			String^ m_abbrev;
			String^ m_direction;
			double m_unit_conv_factor;
			String^ m_unit_name;
			String^ m_unit_auth_name;
			String^ m_unit_code;
		internal:
			Axis(CoordinateSystem^ cs, int idx)
			{
				m_cs = cs;
				m_idx = idx;
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

			property String^ Abbreviation
			{
				String^ get()
				{
					Ensure();
					return m_abbrev;
				}
			}

			property String^ Direction
			{
				String^ get()
				{
					Ensure();
					return m_direction;
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
		};

		[System::Diagnostics::DebuggerDisplayAttribute("{DebuggerDisplay(),nq}")]
		public ref class AxisCollection : public System::Collections::ObjectModel::ReadOnlyCollection<Axis^>
		{
		public:
			AxisCollection(System::Collections::Generic::IList<Axis^>^ list)
				: System::Collections::ObjectModel::ReadOnlyCollection<Axis^>(list)
			{

			}


		private:
			String^ DebuggerDisplay()
			{
				auto sb = gcnew System::Text::StringBuilder();

				for each (auto v in this)
				{
					if (sb->Length)
						sb->Append(", ");

					sb->AppendFormat("{0}[{1}]", v->Name, v->UnitName);
				}

				return sb->ToString();
			}
		};

		typedef Axis ProjAxis;
		public ref class CoordinateSystem :
			public ProjObject
		{
			AxisCollection^ m_axis;
		internal:
			CoordinateSystem(ProjContext^ ctx, PJ* pj)
				: ProjObject(ctx, pj)
			{


			}

		public:
			property CoordinateSystemType CoordinateSystemType
			{
				Details::CoordinateSystemType get()
				{
					return (Details::CoordinateSystemType)proj_cs_get_type(Context, this);
				}
			}

			property ProjType Type
			{
				virtual ProjType get() override
				{
					return ProjType::CoordinateSystem;
				}
			}

		public:
			property Details::AxisCollection^ Axis
			{
				virtual Details::AxisCollection^ get()
				{
					if (!m_axis)
					{
						int cnt = proj_cs_get_axis_count(Context, this);

						if (cnt > 0)
						{
							List<ProjAxis^>^ lst = gcnew List<ProjAxis^>(cnt);

							for (int i = 0; i < cnt; i++)
								lst->Add(gcnew ProjAxis(this, i));

							m_axis = gcnew Details::AxisCollection(lst);
						}
					}
					return m_axis;
				}
			}
		};
	}
}