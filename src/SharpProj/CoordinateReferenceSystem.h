#pragma once
#include "ProjObject.h"

namespace SharpProj {
	using System::Collections::ObjectModel::ReadOnlyCollection;
	ref class CoordinateTransform;

	namespace ProjDetaile
	{
		ref class Axis;
		ref class AxisCollection;
		ref class Datum;
		ref class Ellipsoid;
		ref class PrimeMeridian;

		ref class GeodeticCRS;
		ref class GeometricCRS;

		ref class CoordinateSystem;
	}

	public ref class CoordinateReferenceSystem : ProjObject
	{
	private:
		ProjContext^ m_ctx;
		CoordinateSystem^ m_cs;
		GeodeticCRS^ m_geodCRS;
		ProjDetaile::Ellipsoid^ m_ellipsoid;
		ProjDetaile::Datum^ m_datum;
		ProjDetaile::PrimeMeridian^ m_primeMeridian;
		CoordinateReferenceSystem^ m_baseCrs;
		CoordinateTransform^ m_distanceTransform;
		int m_axis;

		~CoordinateReferenceSystem();

	internal:
		CoordinateReferenceSystem(ProjContext^ ctx, PJ* pj)
			: ProjObject(ctx, pj)
		{
		}


	public:
		property bool IsDeprecated
		{
			bool get()
			{
				return proj_is_deprecated(this);
			}
		}

		property CoordinateSystem^ CoordinateSystem
		{
			ProjDetaile::CoordinateSystem^ get();
		}

	public:
		CoordinateReferenceSystem^ Clone([Optional]ProjContext^ ctx)
		{
			return static_cast<CoordinateReferenceSystem^>(__super::Clone(ctx));
		}

	public:
		property int AxisCount
		{
			virtual int get();
		internal:
			void set(int value);
		}

		property ProjDetaile::AxisCollection^ Axis
		{
			virtual ProjDetaile::AxisCollection^ get();
		}

		property ProjDetaile::Ellipsoid^ Ellipsoid
		{
			ProjDetaile::Ellipsoid^ get();
		}

		property ProjDetaile::GeodeticCRS^ GeodeticCRS
		{
			ProjDetaile::GeodeticCRS^ get();
		}

		property ProjDetaile::Datum^ Datum
		{
			ProjDetaile::Datum^ get();
		}

		property ProjDetaile::PrimeMeridian^ PrimeMeridian
		{
			ProjDetaile::PrimeMeridian^ get();
		}

		property CoordinateReferenceSystem^ BaseCRS
		{
			CoordinateReferenceSystem^ get();
		}

		property CoordinateTransform^ DistanceTransform
		{
			CoordinateTransform^ get();
		}

	public:
		CoordinateReferenceSystem^ WithAxisNormalized([Optional] ProjContext^ context);

	public:
		static CoordinateReferenceSystem^ Create(String^ from, [Optional] ProjContext^ ctx);
		static CoordinateReferenceSystem^ Create(array<String^>^ from, [Optional] ProjContext^ ctx);
	};
}
