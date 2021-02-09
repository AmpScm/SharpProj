#pragma once
#include "ProjObject.h"
#include "ProjCoordinate.h"

namespace SharpProj {
	ref class CoordinateOperation;
	ref class CoordinateReferenceSystem;
	ref class CoordinateArea;
	ref class CoordinateTransformOptions;

	using System::Collections::ObjectModel::ReadOnlyCollection;
	using System::Collections::Generic::List;

	namespace Details {
		public ref class CoordinateOperationFactors
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
			CoordinateOperationFactors(CoordinateOperation^ op, PJ_FACTORS* factors)
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
		public ref class CoordinateOperationParameter
		{
		private:
			initonly CoordinateOperation^ m_op;
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
			CoordinateOperationParameter(CoordinateOperation^ op, int index)
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

	using CoordinateOperationParameter = Details::CoordinateOperationParameter;

	public ref class CoordinateOperation : ProjObject
	{
	private:
		String^ m_methodName;
		ReadOnlyCollection<CoordinateOperationParameter^>^ m_params;
	internal:
		CoordinateOperation(ProjContext^ ctx, PJ* pj)
			: ProjObject(ctx, pj)
		{


		}

	public:
		ProjCoordinate Transform(ProjCoordinate coord) { return DoTransform(true, coord); }
		array<double>^ Transform(...array<double>^ coord) { return DoTransform(true, ProjCoordinate::FromArray(coord)).ToArray(); }
		ProjCoordinate InverseTransform(ProjCoordinate coord) { return DoTransform(false, coord); }
		array<double>^ InverseTransform(...array<double>^ coord) { return DoTransform(false, ProjCoordinate::FromArray(coord)).ToArray(); }

	protected:
		virtual ProjCoordinate DoTransform(bool forward, ProjCoordinate% coords);

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

		property IReadOnlyList<CoordinateOperationParameter^>^ Parameters
		{
			virtual IReadOnlyList<CoordinateOperationParameter^>^ get()
			{
				if (!m_params)
				{
					int cnt = proj_coordoperation_get_param_count(Context, this);

					if (cnt >= 0)
					{
						List<CoordinateOperationParameter^>^ lst = gcnew List<CoordinateOperationParameter^>(cnt);

						for (int i = 0; i < cnt; i++)
							lst->Add(gcnew CoordinateOperationParameter(this, i));

						m_params = lst->AsReadOnly();
					}
				}
				return m_params;
			}
		}

	public:
		CoordinateOperation^ CreateInverse(ProjContext^ ctx)
		{
			if (!ctx)
				ctx = Context;

			if (!HasInverse)
				throw gcnew InvalidOperationException();

			return gcnew CoordinateOperation(ctx, proj_coordoperation_create_inverse(ctx, this));
		}

		CoordinateOperation^ CreateInverse()
		{
			return CreateInverse(Context);
		}

		CoordinateOperation^ CreateNormalized(ProjContext^ ctx)
		{
			if (!ctx)
				ctx = Context;
			return gcnew CoordinateOperation(ctx, proj_normalize_for_visualization(ctx, this));
		}

		CoordinateOperation^ CreateNormalized()
		{
			return CreateNormalized(Context);
		}

	public:
		CoordinateReferenceSystem^ GetSourceCoordinateReferenceSystem([Optional] ProjContext^ context);
		CoordinateReferenceSystem^ GetTargetCoordinateReferenceSystem([Optional] ProjContext^ context);

	public:
		double EllipsoidDistance(ProjCoordinate coordinate1, ProjCoordinate coordinate2);
		double EllipsoidDistance(array<double>^ coordinate1, array<double>^ coordinate2) { return EllipsoidDistance(ProjCoordinate::FromArray(coordinate1), ProjCoordinate::FromArray(coordinate2)); }
		double EllipsoidDistanceZ(ProjCoordinate coordinate1, ProjCoordinate coordinate2);
		double EllipsoidDistanceZ(array<double>^ coordinate1, array<double>^ coordinate2) { return EllipsoidDistanceZ(ProjCoordinate::FromArray(coordinate1), ProjCoordinate::FromArray(coordinate2)); }
		ProjCoordinate EllipsoidGeod(ProjCoordinate coordinate1, ProjCoordinate coordinate2);
		array<double>^ EllipsoidGeod(array<double>^ coordinate1, array<double>^ coordinate2) { return EllipsoidGeod(ProjCoordinate::FromArray(coordinate1), ProjCoordinate::FromArray(coordinate2)).ToArray(); }

	public:
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateArea^ area, [Optional] ProjContext^ ctx);
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, CoordinateTransformOptions^ options, [Optional] ProjContext^ ctx);
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, [Optional] ProjContext^ ctx)
		{
			return CoordinateOperation::Create(sourceCrs, targetCrs, (CoordinateTransformOptions^)nullptr, nullptr);
		}

	public:
		double RoundTrip(bool forward, int transforms, ProjCoordinate coordinate);
		double RoundTrip(bool forward, int transforms, array<double>^ coordinate) { return RoundTrip(forward, transforms, ProjCoordinate::FromArray(coordinate)); }
		Details::CoordinateOperationFactors^ Factors(ProjCoordinate coordinate);
		Details::CoordinateOperationFactors^ Factors(array<double>^ coordinate) { return Factors(ProjCoordinate::FromArray(coordinate)); }


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