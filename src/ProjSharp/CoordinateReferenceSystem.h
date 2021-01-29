#pragma once
#include "ProjObject.h"

namespace ProjSharp {
	ref class GeometricCoordinateReferenceSystem;
	ref class ProjDatum;
	ref class DatumEnsamble;
	ref class CoordinateSystem;
	ref class Ellipsoid;
	ref class PrimeMeridian;
	ref class CoordinateOperation;

	public ref class CoordinateReferenceSystem : ProjObject
	{
	private:
		ProjContext^ m_ctx;

		~CoordinateReferenceSystem()
		{

		}

	internal:
		CoordinateReferenceSystem(ProjContext^ ctx, PJ* pj)
			: ProjObject(ctx, pj)
		{
		}

	public:

	public:
		CoordinateReferenceSystem^ GetNormalized([Optional] ProjContext^ context);
		CoordinateReferenceSystem^ GetGeodeticCoordinateReferenceSystem([Optional] ProjContext^ context);
		ProjDatum^ GetHorizontalDatum([Optional] ProjContext^ context);
		ProjDatum^ GetDatum([Optional] ProjContext^ context);
		DatumEnsamble^ GetDatumEnsamble([Optional] ProjContext^ context);
		ProjDatum^ GetDatumForced([Optional] ProjContext^ context);
		CoordinateSystem^ GetCoordinateSystem([Optional] ProjContext^ context);
		Ellipsoid^ GetEllipsoid([Optional] ProjContext^ context);
		PrimeMeridian^ GetPrimeMeridian([Optional] ProjContext^ context);
		CoordinateOperation^ GetCoordinateOperation([Optional] ProjContext^ context);

	public:
		static CoordinateReferenceSystem^ Create(ProjContext^ ctx, String^ from);
		static CoordinateReferenceSystem^ Create(ProjContext^ ctx, ...array<String^>^ from);

	};
}
