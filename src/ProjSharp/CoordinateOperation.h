#pragma once
#include "ProjObject.h"

namespace ProjSharp {
	ref class CoordinateReferenceSystem;
	ref class ProjArea;

	public ref class CoordinateOperation : ProjObject
	{
	internal:
		CoordinateOperation(ProjContext^ ctx, PJ* pj)
			: ProjObject(ctx, pj)
		{


		}

	public:
		array<double>^ Transform(...array<double>^ coords);
		array<double>^ InverseTransform(...array<double>^ coords);

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

		property double Accuraracy
		{
			double virtual get() override
			{
				return proj_coordoperation_get_accuracy(Context, this);
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
		double EllipsoidDistance(array<double>^ coordinate1, array<double>^ coordinate2);
		double EllipsoidDistanceZ(array<double>^ coordinate1, array<double>^ coordinate2);
		array<double>^ EllipsoidGeod(array<double>^ coordinate1, array<double>^ coordinate2);

	public:
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, ProjArea ^area, [Optional] ProjContext^ ctx);
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, [Optional] ProjContext^ ctx)
		{
			return CoordinateOperation::Create(sourceCrs, targetCrs, nullptr, nullptr);
		}
	};

}