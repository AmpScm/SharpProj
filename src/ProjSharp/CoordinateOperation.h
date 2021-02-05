#pragma once
#include "ProjObject.h"

namespace ProjSharp {
	ref class CoordinateOperation;
	ref class CoordinateReferenceSystem;
	ref class ProjArea;

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

	public ref class CoordinateOperation : ProjObject
	{
	internal:
		CoordinateOperation(ProjContext^ ctx, PJ* pj)
			: ProjObject(ctx, pj)
		{


		}

	public:
		array<double>^ Transform(...array<double>^ coords) { return DoTransform(true, coords); }
		array<double>^ InverseTransform(...array<double>^ coords) { return DoTransform(false, coords); }

	protected:
		virtual array<double>^ DoTransform(bool forward, array<double>^ coords);

	public:
		property bool HasInverse
		{
			bool get()
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
		double EllipsoidDistance(array<double>^ coordinate1, array<double>^ coordinate2);
		double EllipsoidDistanceZ(array<double>^ coordinate1, array<double>^ coordinate2);
		array<double>^ EllipsoidGeod(array<double>^ coordinate1, array<double>^ coordinate2);

	public:
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, ProjArea ^area, [Optional] ProjContext^ ctx);
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, [Optional] ProjContext^ ctx)
		{
			return CoordinateOperation::Create(sourceCrs, targetCrs, nullptr, nullptr);
		}

	public:
		double RoundTrip(bool forward, int transforms, array<double>^ coordinate);
		CoordinateOperationFactors^ Factors(array<double>^ coordinate);
	};

}