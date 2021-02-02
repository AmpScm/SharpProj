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
	};

}