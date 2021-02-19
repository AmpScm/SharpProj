#pragma once
namespace SharpProj {
	public ref class CoordinateArea
	{
	public:
		CoordinateArea()
		{}

		CoordinateArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude)
		{
			WestLongitude = westLongitude;
			SouthLatitude = southLatitude;
			EastLongitude = eastLongitude;
			NorthLatitude = northLatitude;
		}

	public:
		property double WestLongitude;
		property double SouthLatitude;
		property double EastLongitude;
		property double NorthLatitude;
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