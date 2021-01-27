#pragma once
#include "CoordinateBase.h"

namespace ProjSharp {
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

	ref class GeometricCoordinateSystem;

	public ref class CoordinateSystem : CoordinateBase
	{
	private:
		ProjContext^ m_ctx;

		~CoordinateSystem()
		{

		}

	private protected:
		CoordinateSystem(ProjContext^ ctx, PJ* pj)
			: CoordinateBase(ctx, pj)
		{

		}

	public:
		double Distance2D(array<double>^ coordinate1, array<double>^ coordinate2);
		double Distance3D(array<double>^ coordinate1, array<double>^ coordinate2);

	public:
		property CoordinateSystemType Type
		{
			CoordinateSystemType get()
			{
				return (CoordinateSystemType)proj_cs_get_type(Context, this);
			}
		}
		property int Axis
		{
			int get()
			{
				return proj_cs_get_axis_count(Context, this);
			}
		}
		GeometricCoordinateSystem^ GetGeometricCoordinateSystem();

	public:
		static CoordinateSystem^ Create(ProjContext^ ctx, String^ from);
		static CoordinateSystem^ Create(ProjContext^ ctx, ...array<String^>^ from);

	};

	public ref class GeometricCoordinateSystem : CoordinateSystem
	{
	internal:
		GeometricCoordinateSystem(ProjContext^ ctx, PJ* pj)
			: CoordinateSystem(ctx, pj)
		{

		}
	};
}