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
		CoordinateReferenceSystem^ GetNormalized(ProjContext^ context);
		CoordinateReferenceSystem^ GetGeodeticCoordinateReferenceSystem(ProjContext^ context);
		ProjDatum^ GetHorizontalDatum(ProjContext^ context);
		ProjDatum^ GetDatum(ProjContext^ context);
		DatumEnsamble^ GetDatumEnsamble(ProjContext^ context);
		ProjDatum^ GetDatumForced(ProjContext^ context);
		CoordinateSystem^ GetCoordinateSystem(ProjContext^ context);
		Ellipsoid^ GetEllipsoid(ProjContext^ context);
		PrimeMeridian^ GetPrimeMeridian(ProjContext^ context);
		CoordinateOperation^ GetCoordinateOperation(ProjContext^ context);

	public:
		static CoordinateReferenceSystem^ Create(ProjContext^ ctx, String^ from);
		static CoordinateReferenceSystem^ Create(ProjContext^ ctx, ...array<String^>^ from);

	};
}
