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

    auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);

    if (format == "G")
    {
        String^ v;

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
    else if (format == "DMS" || format->StartsWith("DMS:"))
    {
        char c1[32];

        auto sfx = (format->Length > 4) ? format->Substring(4) : "NSEW";

        proj_rtodms(c1, X, (sfx->Length > 0) ? sfx[0] : 'N', (sfx->Length > 1) ? sfx[1] : 'S');

        String^ s = Utf8_PtrToString(c1);

        proj_rtodms(c1, Y, (sfx->Length > 2) ? sfx[2] : 'E', (sfx->Length > 3) ? sfx[3] : 'W');

        String^ gs = (ci && ci->NumberFormat->NumberDecimalSeparator == ",") ? ";" : ",";

        auto v = String::Concat(s, gs, " ", Utf8_PtrToString(c1));

        if (Axis > 2)
        {
            v = String::Concat(v, gs, " ", Z.ToString(formatProvider));
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

bool PPoint::TryParse(String^ stringValue, String^ format, IFormatProvider^ formatProvider, [Out] PPoint% value)
{
    if (String::IsNullOrEmpty(stringValue))
        throw gcnew ArgumentNullException("stringValue");
    else if (!formatProvider)
        throw gcnew ArgumentNullException("formatProvider");

    auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);
    String^ gs = (ci && ci->NumberFormat->NumberDecimalSeparator == ",") ? ";" : ",";

    array<String^>^ parts = stringValue->Split(gcnew array<wchar_t>{ gs[0]}, 4);
    array<double>^ vals = gcnew array<double>(parts->Length);

    for (int i = 0; i < parts->Length; i++)
    {
        if (i < 2 && (format && (format == "DMS" || format->StartsWith("DMS:"))))
        {
            auto sfx = (format->Length > 4) ? format->Substring(4) : "NSEW";

            std::string vv = utf8_string(parts[i]);
            char* pKey = nullptr;

            vals[i] = proj_dmstor(vv.c_str(), &pKey);

            /* How does this determine +/- ???? */
            /*if (pKey > vv.c_str() && sfx->Length > (i * 2) && pKey[-1] != sfx[i * 2])
            {
                //vals[i] = -vals[i];
            }*/
        }
        else
        {
            auto vv = parts[i];

            if (vv->Length > 2 && wchar_t::IsLetter(vv, 0) && vv[1] == '=')
                vv = vv->Substring(2);
            else if (vv->Length > 3 && wchar_t::IsWhiteSpace(vv, 0) && wchar_t::IsLetter(vv, 1) && vv[2] == '=')
                vv = vv->Substring(3);

            if (!double::TryParse(vv, System::Globalization::NumberStyles::Any, formatProvider, vals[i]))
            {
                value = PPoint();
                return false;
            }
        }
    }

    value = PPoint(vals);
    return true;
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
