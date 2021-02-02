#pragma once
namespace ProjSharp {
	public ref class ProjArea
	{
		initonly String^ m_name;
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

	internal:
		ProjArea(double westLongitude, double southLatitude, double eastLongitude, double northLatitude, String^ name)
			: ProjArea(westLongitude, southLatitude, eastLongitude, northLatitude)
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

}