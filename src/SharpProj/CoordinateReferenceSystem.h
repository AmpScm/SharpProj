#pragma once
#include "ProjObject.h"

namespace SharpProj {
	using System::Collections::ObjectModel::ReadOnlyCollection;
	ref class GeometricCoordinateReferenceSystem;
	ref class ProjDatum;
	ref class ProjDatumList;
	ref class CoordinateSystem;
	ref class Ellipsoid;
	ref class PrimeMeridian;
	ref class CoordinateTransform;

	namespace Details
	{
		ref class Axis;
		ref class AxisCollection;
	}

	public ref class CoordinateReferenceSystem : ProjObject
	{
	private:
		ProjContext^ m_ctx;
		CoordinateSystem^ m_cs;
		CoordinateReferenceSystem^ m_geodCRS;
		Ellipsoid^ m_ellipsoid;
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
			SharpProj::CoordinateSystem^ get();
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

		property Details::AxisCollection^ Axis
		{
			virtual Details::AxisCollection^ get();
		}

		property Ellipsoid^ Ellipsoid
		{
			SharpProj::Ellipsoid^ get();
		}

		property CoordinateReferenceSystem^ GeodeticCRS
		{
			CoordinateReferenceSystem^ get();
		}

	public:
		CoordinateReferenceSystem^ GetNormalized([Optional] ProjContext^ context);		
		ProjDatum^ GetHorizontalDatum([Optional] ProjContext^ context);
		ProjDatum^ GetDatum([Optional] ProjContext^ context);
		ProjDatumList^ GetDatumList([Optional] ProjContext^ context);
		ProjDatum^ GetDatumForced([Optional] ProjContext^ context);		
		PrimeMeridian^ GetPrimeMeridian([Optional] ProjContext^ context);
		CoordinateTransform^ GetTransform([Optional] ProjContext^ context);
		CoordinateReferenceSystem^ GetBaseCRS([Optional] ProjContext^ context);
		CoordinateReferenceSystem^ GetHubCRS([Optional] ProjContext^ context);

	public:
		static CoordinateReferenceSystem^ Create(String^ from, [Optional] ProjContext^ ctx);
		static CoordinateReferenceSystem^ Create(array<String^>^ from, [Optional] ProjContext^ ctx);
	};
}
