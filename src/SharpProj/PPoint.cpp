#include "pch.h"
#include "PPoint.h"
#include "CoordinateTransform.h"
#include "CoordinateSystem.h"

using namespace SharpProj;
using System::Text::StringBuilder;
using System::Globalization::NumberStyles;

static void AppendDMS(StringBuilder^ sb, System::IFormatProvider^ formatProvider, double value, bool addSpaces)
{
    double dd = Math::Truncate(value);
    int sign = Math::Sign(value);

    int d = (int)dd * sign;
    

    double r = (value - dd) * sign;
    r *= 60.0;

    int min = (int)Math::Truncate(r);    
    r -= min;
    r *= 60.0;

    double sec = Math::Round(r, 6);

    if (sec > 59.999999)
    {
        sec = 0;
        min++;
        if (min >= 60)
        {
            min -= 60;
            d += sign;
        }
    }
    sb->AppendFormat(d.ToString(formatProvider));
    sb->Append(L"\xB0");
    if (addSpaces)
        sb->Append(" ");
    sb->Append(min.ToString(formatProvider));
    sb->Append("\'");
    if (addSpaces)
        sb->Append(" ");
    sb->Append(sec.ToString(formatProvider));
    sb->Append("\"");
}

static bool TryParseDMS(String^ text, System::IFormatProvider^ formatProvider, wchar_t pos_symbol, wchar_t neg_symbol, [Out] double% value)
{
    if (String::IsNullOrWhiteSpace(text))
        return false;

    value = 0.0;

    int nLast = text->Length;

    while (nLast > 0 && wchar_t::IsWhiteSpace(text, nLast - 1))
        nLast--;

    int sign = 1;
    if (nLast > 0)
    {
        wchar_t l = wchar_t::ToUpperInvariant(text[nLast - 1]);

        if (l == neg_symbol)
        {
            sign = -1;
            nLast--;
        }
        else if (l == pos_symbol)
        {
            sign = 1;
            nLast--;
        }
        else if (l == 'S' || l == 'W')
        {
            sign = -1;
            nLast--;
        }
        else
            sign = 1;
    }
    else
        sign = 1;

    int nStart = 0;
    while (nStart < text->Length && wchar_t::IsWhiteSpace(text, nStart))
        nStart++;
    int nEnd = nStart;
    while (nEnd < text->Length && wchar_t::IsDigit(text, nEnd))
        nEnd++;

    if (nEnd >= nLast)
        return false;

    if (text[nEnd] != L'\xB0' && text[nEnd] != 'd' && text[nEnd] != 'h')
        return false;

    if (!double::TryParse(text->Substring(nStart, nEnd-nStart), NumberStyles::Any, formatProvider, value))
        return false;

    nStart = nEnd + 1;
    while (nStart < text->Length && wchar_t::IsWhiteSpace(text, nStart))
        nStart++;

    nEnd = nStart;
    while (nEnd < text->Length && wchar_t::IsDigit(text, nEnd))
        nEnd++;

    double vv;
    if (!double::TryParse(text->Substring(nStart, nEnd - nStart), NumberStyles::Any, formatProvider, vv))
        return false;

    value += vv / 60.0;

    if (nEnd >= text->Length || (text[nEnd] != L'\'' && text[nEnd] != L'm'))
        return false;

    nStart = nEnd + 1;
    while (nStart < text->Length && wchar_t::IsWhiteSpace(text, nStart))
        nStart++;

    auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);
    wchar_t gs = (ci && ci->NumberFormat->NumberDecimalSeparator) ? ci->NumberFormat->NumberDecimalSeparator[0] : L'.';

    nEnd = nStart;
    while (nEnd < text->Length && (wchar_t::IsDigit(text, nEnd) || text[nEnd] == '.' || text[nEnd] == gs))
        nEnd++;

    if (!double::TryParse(text->Substring(nStart, nEnd - nStart), NumberStyles::Any, formatProvider, vv))
        return false;

    value += vv / 3600.0;

    if (nEnd >= text->Length || (text[nEnd] != L'\"' && text[nEnd] != L's'))
        return false;
    nEnd++;

    if (nEnd != nLast)
        return false;

    value *= sign;
    return true;
}

String^ PPoint::ToString(System::String^ format, System::IFormatProvider^ formatProvider)
{
    if (String::IsNullOrEmpty(format))
        format = "G";
    else if (!formatProvider)
        throw gcnew ArgumentNullException("formatProvider");

    auto ci = dynamic_cast<System::Globalization::CultureInfo^>(formatProvider);

    if (format == "G" || format== "g")
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
            v = String::Format(formatProvider, "X={0}{3} Y={1}{3} Z={2}", gcnew array<Object^> { X, Y, Z, gs });
        }

        if (HasT)
            return String::Format(formatProvider, "{0}{2} T={1}", v, T, gs);
        else
            return v;
    }
    else if (format->Length == 1 && ((String^)L"eEfFnNrR")->IndexOf(format[0]) >= 0)
    {
        bool k = wchar_t::IsUpper(format, 0);
        String^ v;

        String^ gs = (ci && ci->NumberFormat->NumberDecimalSeparator == ",") ? ";" : ",";
        switch (Axis)
        {
        case 1:
            v = String::Format(formatProvider, "{1}{0}", X.ToString(format, formatProvider), k ? "X=" : "");
            break;
        case 2:
            v = String::Format(formatProvider, "{3}{0}{2} {4}{1}", X.ToString(format, formatProvider), Y.ToString(format, formatProvider), gs, k ? "X=" : "", k ? "Y=" : "");
            break;
        case 3:
        default:
            v = String::Format(formatProvider, "{4}{0}{3} {5}{1}{3} {6}{2}", X.ToString(format, formatProvider), Y.ToString(format, formatProvider), Z.ToString(format, formatProvider), gs,
                               k ? "X=" : "", k ? "Y=" : "", k ? "Z=" : "");
        }

        if (HasT)
            return String::Format(formatProvider, "{0}{2} T={1}", v, T.ToString(format, formatProvider), gs);
        else
            return v;
    }
    else if (format == "D" || format == "d" || format->StartsWith("D:", StringComparison::OrdinalIgnoreCase))
    {
        auto sb = gcnew StringBuilder();
        auto gs = (ci && ci->NumberFormat->NumberDecimalSeparator == ",") ? L';' : L',';
        auto sfx = (format->Length > 2) ? format->Substring(2) : "NSEW";

        AppendDMS(sb, formatProvider, X, format[0] == 'D');

        if (X > 0)
            sb->Append(sfx->Length > 0 ? sfx[0] : L'N');
        else
            sb->Append(sfx->Length > 1 ? sfx[1] : L'S');

        sb->Append(gs);
        sb->Append(" ");

        AppendDMS(sb, formatProvider, Y, format[0] == 'D');

        if (Y > 0)
            sb->Append(sfx->Length > 2 ? sfx[2] : L'E');
        else
            sb->Append(sfx->Length > 3 ? sfx[3] : L'W');

        if (Axis > 2)
        {
            sb->Append(gs);
            sb->Append(" ");
            sb->Append(Z.ToString(formatProvider));
        }

        if (HasT)
        {
            sb->Append(gs);
            sb->Append(" T=");
            sb->Append(T.ToString(formatProvider));
        }

        return sb->ToString();
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
    double TVal = double::NaN;

    for (int i = 0; i < parts->Length; i++)
    {
        if (parts[i]->IndexOfAny(gcnew array<wchar_t>{L'\xB0', 'd'}) > 0
            || (i < 2 && format && (format == "D" || format == "d" || format->StartsWith("D:", StringComparison::OrdinalIgnoreCase))))
        {
            auto sfx = (format && format->StartsWith("D:", StringComparison::OrdinalIgnoreCase)) ? format->Substring(2) : "NSEW";

            double val;
            if (!TryParseDMS(parts[i], formatProvider, sfx->Length > (2 * i) ? sfx[2 * i] : L'\0', sfx->Length > (2 * i + 1) ? sfx[2 * i + 1] : L'\0', val))
            {
                value = PPoint();
                return false;
            }

            vals[i] = val;
        }
        else
        {
            auto vv = parts[i];
            bool is_T = false;

            if (vv->Length > 2 && wchar_t::IsLetter(vv, 0) && vv[1] == '=')
            {
                if (i == parts->Length - 1 && vv[0] == 'T')
                    is_T = true;
                vv = vv->Substring(2);
            }
            else if (vv->Length > 3 && wchar_t::IsWhiteSpace(vv, 0) && wchar_t::IsLetter(vv, 1) && vv[2] == '=')
            {
                if (i == parts->Length - 1 && vv[1] == 'T')
                    is_T = true;
                vv = vv->Substring(3);
            }

            double val;
            if (!double::TryParse(vv, System::Globalization::NumberStyles::Any, formatProvider, val))
            {
                value = PPoint();
                return false;
            }

            if (!is_T)
                vals[i] = val;
            else
            {
                vals[i] = 0.0;

                TVal = val;
            }
        }
    }

    value = PPoint(vals);

    if (!double::IsNaN(TVal))
    {
        value = value.WithT(TVal);
        if (parts->Length == 3)
            value.Axis = 2;
    }

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
