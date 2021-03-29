#pragma once
#include "LatititudeLongitudeArea.h"
namespace SharpProj {

	public ref class CoordinateArea : LatitudeLongitudeArea
	{
	public:
		CoordinateArea()
		{}

		CoordinateArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude)
			: LatitudeLongitudeArea(westLongitude, southLatitude, eastLongitude, northLatitude)
		{
		}

	public:
		virtual property double WestLongitude
		{
			virtual double get() override
			{
				return LatitudeLongitudeArea::WestLongitude;
			}
		public:
			void set(double value) new
			{
				LatitudeLongitudeArea::WestLongitude = value;
			}
		}
		virtual property double SouthLatitude
		{
			virtual double get() override
			{
				return LatitudeLongitudeArea::SouthLatitude;
			}
		public:
			void set(double value) new
			{
				LatitudeLongitudeArea::SouthLatitude = value;
			}
		}
		virtual property double EastLongitude
		{
			virtual double get() override
			{
				return LatitudeLongitudeArea::EastLongitude;
			}
		public:
			void set(double value) new
			{
				LatitudeLongitudeArea::EastLongitude = value;
			}
		}
		virtual property double NorthLatitude
		{
			virtual double get() override
			{
				return LatitudeLongitudeArea::NorthLatitude;
			}
		public:
			void set(double value) new
			{
				LatitudeLongitudeArea::NorthLatitude = value;
			}
		}

	public:
		static operator CoordinateArea^ (ILatitudeLongitudeArea^ area)
		{
			if (!area)
				return nullptr;

			return gcnew CoordinateArea(area->WestLongitude, area->SouthLatitude, area->EastLongitude, area->NorthLatitude);
		}
	};

	public enum class IntermediateCrsUsage
	{
		Auto,
		Always,
		Never
	};

	public ref class CoordinateTransformOptions
	{
	public:
		property CoordinateArea^ Area;
		property String^ Authority;
		property Nullable<double> Accuracy;
		property bool NoBallparkConversions;
		property bool NoDiscardIfMissing;
		property bool UsePrimaryGridNames;
		property bool UseSuperseded;
		property bool StrictContains;
		property IntermediateCrsUsage IntermediateCrsUsage;		
	};
}