#include "pch.h"
#include "PPoint.h"
#include "CoordinateTransform.h"
#include "CoordinateSystem.h"

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

		String^ gs = (ci && ci->NumberFormat->NumberDecimalSeparator == ",") ? ";" : ",";
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

String^ PPoint::ToString(CoordinateReferenceSystem^ crs, IFormatProvider^ formatProvider)
{
    if (!crs || !crs->Axis)
        return ToString();

    return ToString(crs->Axis, formatProvider);
}

String^ PPoint::ToString(CoordinateSystem^ cs, IFormatProvider^ formatProvider)
{
    if (!cs || !cs->Axis)
        return ToString();

    return ToString(cs->Axis, formatProvider);
}

String^ PPoint::ToString(AxisCollection^ axis, IFormatProvider^ formatProvider)
{
    if (axis == nullptr)
        throw gcnew ArgumentNullException("axis");

    auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);

    String^ gs = (ci && ci->NumberFormat->NumberDecimalSeparator == ",") ? ";" : ",";

    auto sb = gcnew System::Text::StringBuilder();
    for (int i = 0; i < axis->Count; i++)
    {
        if (i > 0)
        {
            sb->Append(gs);
            sb->Append(L' ');
        }

        sb->Append(axis[i]->Abbreviation);
        sb->Append("=");
        sb->AppendFormat(formatProvider, "{0}", default[i]);
    }
    return sb->ToString();
}
