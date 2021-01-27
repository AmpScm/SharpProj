#pragma once
namespace ProjSharp {
	public ref class ProjArea
	{
	public:
		ProjArea()
		{}

		ProjArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude)
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

}