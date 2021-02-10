#pragma once
#include "ProjObject.h"
#include "ProjCoordinate.h"

namespace SharpProj {
	ref class CoordinateTransform;
	ref class CoordinateReferenceSystem;
	ref class CoordinateArea;
	ref class CoordinateTransformOptions;

	using System::Collections::ObjectModel::ReadOnlyCollection;
	using System::Collections::Generic::List;

	namespace Details {
		public ref class CoordinateTransformFactors
		{
		private:
			initonly double m_meridional_scale;               /* h */
			initonly double m_parallel_scale;                 /* k */
			initonly double m_areal_scale;                    /* s */
			initonly double m_angular_distortion;             /* omega */
			initonly double m_meridian_parallel_angle;        /* theta-prime */
			initonly double m_meridian_convergence;           /* alpha */
			initonly double m_tissot_semimajor;               /* a */
			initonly double m_tissot_semiminor;               /* b */
			initonly double m_dx_dlam, m_dx_dphi;
			initonly double m_dy_dlam, m_dy_dphi;
		internal:
			CoordinateTransformFactors(CoordinateTransform^ op, PJ_FACTORS* factors)
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

		[System::Diagnostics::DebuggerDisplayAttribute("{Name,nq}={ValueString}")]
		public ref class CoordinateTransformParameter
		{
		private:
			initonly CoordinateTransform^ m_op;
			initonly int m_index;

			String^ m_name;
			String^ m_auth_name;
			String^ m_code;
			double m_value;
			String^ m_value_string;
			double m_unit_conv_factor;
			String^ m_unit_name;
			String^ m_unit_auth_name;
			String^ m_unit_code;
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

	using CoordinateTransformParameter = Details::CoordinateTransformParameter;

	public ref class CoordinateTransform : ProjObject
	{
	private:
		String^ m_methodName;
		ReadOnlyCollection<CoordinateTransformParameter^>^ m_params;
		CoordinateReferenceSystem^ m_source;
		CoordinateReferenceSystem^ m_target;
	internal:
		CoordinateTransform(ProjContext^ ctx, PJ* pj)
			: ProjObject(ctx, pj)
		{


		}

	private:
		~CoordinateTransform();

	public:
		ProjCoordinate Apply(ProjCoordinate coord) { return DoTransform(true, coord); }
		array<double>^ Apply(...array<double>^ ordinates) { return DoTransform(true, ProjCoordinate(ordinates)).ToArray(); }
		ProjCoordinate ApplyReversed(ProjCoordinate coord) { return DoTransform(false, coord); }
		array<double>^ ApplyReversed(...array<double>^ ordinates) { return DoTransform(false, ProjCoordinate(ordinates)).ToArray(); }

	protected:
		virtual ProjCoordinate DoTransform(bool forward, ProjCoordinate% coords);

	internal:
		ProjCoordinate FromCoordinate(const PJ_COORD& coord, bool forward);
		

	public:
		property bool HasInverse
		{
			virtual bool get()
			{
				PJ_PROJ_INFO info = proj_pj_info(this);
				return info.has_inverse;
			}
		}

		property int GridUsageCount
		{
			int get()
			{
				return proj_coordoperation_get_grid_used_count(Context, this);
			}
		}

		property Nullable<double> Accuraracy
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

		property bool AngularInput
		{
			bool get()
			{
				return proj_angular_input(this, PJ_FWD);
			}
		}

		property bool AngularOutput
		{
			bool get()
			{
				return proj_angular_output(this, PJ_FWD);
			}
		}

		property bool DegreeInput
		{
			bool get()
			{
				return proj_degree_input(this, PJ_FWD);
			}
		}

		property bool DegreeOutput
		{
			bool get()
			{
				return proj_degree_output(this, PJ_FWD);
			}
		}

		property bool IsInstantiable
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
					const char* auth_name;
					const char* auth_code;

					if (proj_coordoperation_get_method_info(Context, this, &method_name, &auth_name, &auth_code))
					{
						if (method_name)
							m_methodName = gcnew String(method_name);
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

					if (cnt >= 0)
					{
						List<CoordinateTransformParameter^>^ lst = gcnew List<CoordinateTransformParameter^>(cnt);

						for (int i = 0; i < cnt; i++)
							lst->Add(gcnew CoordinateTransformParameter(this, i));

						m_params = lst->AsReadOnly();
					}
				}
				return m_params;
			}
		}

	public:
		CoordinateTransform^ CreateInverse(ProjContext^ ctx)
		{
			if (!ctx)
				ctx = Context;

			if (!HasInverse)
				throw gcnew InvalidOperationException();

			return gcnew CoordinateTransform(ctx, proj_coordoperation_create_inverse(ctx, this));
		}

		CoordinateTransform^ CreateInverse()
		{
			return CreateInverse(Context);
		}

		CoordinateTransform^ CreateNormalized(ProjContext^ ctx)
		{
			if (!ctx)
				ctx = Context;
			return gcnew CoordinateTransform(ctx, proj_normalize_for_visualization(ctx, this));
		}

		CoordinateTransform^ CreateNormalized()
		{
			return CreateNormalized(Context);
		}

	public:
		property CoordinateReferenceSystem^ SourceCRS
		{
			CoordinateReferenceSystem ^ get();
		}

		property CoordinateReferenceSystem^ TargetCRS
		{
			CoordinateReferenceSystem^ get();
		}

	public:
		double EllipsoidDistance(ProjCoordinate coordinate1, ProjCoordinate coordinate2);
		double EllipsoidDistance(array<double>^ ordinates1, array<double>^ ordinates2) { return EllipsoidDistance(ProjCoordinate(ordinates1), ProjCoordinate(ordinates2)); }
		double EllipsoidDistanceZ(ProjCoordinate coordinate1, ProjCoordinate coordinate2);
		double EllipsoidDistanceZ(array<double>^ ordinates1, array<double>^ ordinates2) { return EllipsoidDistanceZ(ProjCoordinate(ordinates1), ProjCoordinate(ordinates2)); }
		ProjCoordinate EllipsoidGeod(ProjCoordinate coordinate1, ProjCoordinate coordinate2);
		array<double>^ EllipsoidGeod(array<double>^ ordinates1, array<double>^ ordinates2) { return EllipsoidGeod(ProjCoordinate(ordinates1), ProjCoordinate(ordinates2)).ToArray(); }

	public:
		static CoordinateTransform^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateArea^ area, [Optional] ProjContext^ ctx);
		static CoordinateTransform^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateTransformOptions^ options, [Optional] ProjContext^ ctx);
		static CoordinateTransform^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, [Optional] ProjContext^ ctx)
		{
			return CoordinateTransform::Create(sourceCrs, targetCrs, (CoordinateTransformOptions^)nullptr, nullptr);
		}

	public:
		double RoundTrip(bool forward, int transforms, ProjCoordinate coordinate);
		double RoundTrip(bool forward, int transforms, array<double>^ ordinates) { return RoundTrip(forward, transforms, ProjCoordinate(ordinates)); }
		Details::CoordinateTransformFactors^ Factors(ProjCoordinate coordinate);
		Details::CoordinateTransformFactors^ Factors(array<double>^ ordinates) { return Factors(ProjCoordinate(ordinates)); }


	public:
		// Some helpers that do not really belong here, but are easy to access in a sensible way
		// if they are here anyway
		static double ToRad(double deg)
		{
			return proj_torad(deg);
		}

		static double ToDeg(double deg)
		{
			return proj_todeg(deg);
		}
	};

}