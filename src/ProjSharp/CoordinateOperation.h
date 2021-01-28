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
		array<double>^ Transform(array<double>^ coords);
		array<double>^ InverseTransform(array<double>^ coords);

	public:
		property bool HasInverse
		{
			bool get()
			{
				PJ_PROJ_INFO info = proj_pj_info(this);
				return info.has_inverse;
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
		static CoordinateOperation^ Create(ProjContext ^ctx, CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs, ProjArea ^area);
		static CoordinateOperation^ Create(CoordinateReferenceSystem^ sourceCrs, CoordinateReferenceSystem^ targetCrs);
	};

}