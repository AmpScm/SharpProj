#pragma once
namespace SharpProj {
	public ref class CoordinateArea
	{
		initonly String^ m_name;
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

	internal:
		CoordinateArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude, String^ name)
			: CoordinateArea(westLongitude, southLatitude, eastLongitude, northLatitude)
		{
			m_name = name;
		}

	public:
		property double WestLongitude;
		property double SouthLatitude;
		property double EastLongitude;
		property double NorthLatitude;

		property String^ Name
		{
			String^ get()
			{
				return m_name;
			}
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
		property IntermediateCrsUsage IntermediateCrsUsage;		
	};
}