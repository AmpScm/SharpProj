#include "pch.h"
#include "CoordinateSystem.h"
#include "CoordinateReferenceSystem.h"

using namespace SharpProj;

void SharpProj::Proj::Axis::Ensure()
{
    const char* name, * abbrev, * direction, * unit_name, * unit_auth_name, * unit_code;
    double unit_conv_factor;

    if (!m_name && proj_cs_get_axis_info(m_cs->Context, m_cs, m_idx,
        &name, &abbrev, &direction, &unit_conv_factor, &unit_name, &unit_auth_name, &unit_code))
    {
        m_name = Utf8_PtrToString(name);
        m_abbrev = Utf8_PtrToString(abbrev);
        m_direction = Utf8_PtrToString(direction);
        m_unit_conv_factor = unit_conv_factor;
        m_unit_name = Utf8_PtrToString(unit_name);
        m_unit_auth_name = Utf8_PtrToString(unit_auth_name);
        m_unit_code = Utf8_PtrToString(unit_code);
    }
}

CoordinateSystemType CoordinateReferenceSystem::CoordinateSystemType::get()
{
    return CoordinateSystem->CoordinateSystemType;
}
