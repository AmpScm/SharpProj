#include "pch.h"
#include "ProjArea.h"

String^ ProjArea::ToString(String^ format, IFormatProvider^ formatProvider)
{
    String^ f = format;
    IFormatProvider^ fp = formatProvider;

    return String::Format(formatProvider, "W-E: {0} - {1}, S-N: {2} - {3}", gcnew array<Object^>{Fmt(f, fp, WestLongitude), Fmt(f, fp, EastLongitude), Fmt(f, fp, SouthLatitude), Fmt(f, fp, NorthLatitude)});
}

String^ ProjRange::ToString(System::String^ format, System::IFormatProvider^ formatProvider)
{
    if (format == "G")
    {
        auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);
        String^ gs = ci ? ci->NumberFormat->NumberGroupSeparator : ",";

        return String::Format(formatProvider, "X: {0} - {1}{2} Y: {3} - {4}", gcnew array<Object^> { MinX, MaxX, gs, MinY, MaxY });
    }
    else
        throw gcnew FormatException();
}
