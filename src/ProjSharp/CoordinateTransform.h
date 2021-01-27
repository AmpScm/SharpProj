#pragma once
#include "CoordinateBase.h"

namespace ProjSharp {
	ref class CoordinateSystem;
	ref class ProjArea;

	public ref class CoordinateTransform : CoordinateBase
	{
	private:
		CoordinateTransform(ProjContext^ ctx, PJ* pj)
			: CoordinateBase(ctx, pj)
		{


		}

	public:
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
		array<double>^ Transform(array<double>^ coords);
		array<double>^ InverseTransform(array<double>^ coords);


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
		static CoordinateTransform^ Create(ProjContext ^ctx, CoordinateSystem^ sourceCrs, CoordinateSystem^ targetCrs, ProjArea ^area);
		static CoordinateTransform^ Create(CoordinateSystem^ sourceCrs, CoordinateSystem^ targetCrs);
	};

}