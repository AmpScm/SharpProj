#pragma once
namespace ProjSharp {

	public ref class Proj
	{
	private:
		Proj()
		{}

	public:
		static double ToRad(double deg)
		{
			return proj_torad(deg);
		}

		static double ToDeg(double deg)
		{
			return proj_todeg(deg);
		}


	public:
		static property System::Version^ Version
		{
			System::Version^ get()
			{
				return gcnew System::Version(PROJ_VERSION_MAJOR, PROJ_VERSION_MINOR, PROJ_VERSION_PATCH);
			}
		}
	};

}