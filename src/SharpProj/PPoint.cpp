#include "pch.h"
#include "PPoint.h"
#include "CoordinateTransform.h"

using namespace SharpProj;

String^ PPoint::ToString(System::String^ format, System::IFormatProvider^ formatProvider)
{
	if (String::IsNullOrEmpty(format))
		format = "G";
	else if (!formatProvider)
		throw gcnew ArgumentNullException("formatProvider");

	if (format == "G")
	{
		String^ v;

		auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);

		String^ gs = (ci && ci->NumberFormat->CurrencyDecimalSeparator == ",") ? ";" : ",";
		switch (Axis)
		{
		case 1:
			v = String::Format(formatProvider, "X={0}", X);
			break;
		case 2:
			v = String::Format(formatProvider, "X={0}{2} Y={1}", X, Y, gs);
			break;
		case 3:
		default:
			v = String::Format(formatProvider, "X={0}{3} Y={1}{3} Z={2}", X, Y, Z, gs);
		}

		if (HasT)
			return String::Format(formatProvider, "{0}{2} T={1}", v, T, gs);
		else
			return v;
	}
	else
	{
		throw gcnew FormatException(String::Format("Format '{0}' not supported yet", format));
	}
}

PPoint SharpProj::PPoint::DegToRad()
{
	PPoint pc = PPoint(
		CoordinateTransform::ToRad(X),
		CoordinateTransform::ToRad(Y),
		Z,
		T);
	pc.Axis = Axis;
	return pc;
}

PPoint SharpProj::PPoint::RadToDeg()
{
	PPoint pc = PPoint(
		CoordinateTransform::ToDeg(X),
		CoordinateTransform::ToDeg(Y),
		Z,
		T);
	pc.Axis = Axis;
	return pc;
}

