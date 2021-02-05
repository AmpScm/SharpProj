#include "pch.h"
#include "CoordinateSystem.h"

using namespace SharpProj;

void ProjAxis::Ensure()
{
    const char* name, * abbrev, * direction, * unit_name, * unit_auth_name, * unit_code;
    double unit_conv_factor;

    if (!m_name && proj_cs_get_axis_info(m_cs->Context, m_cs, m_idx,
        &name, &abbrev, &direction, &unit_conv_factor, &unit_name, &unit_auth_name, &unit_code))
    {
        m_name = name ? gcnew String(name) : nullptr;
        m_abbrev = abbrev ? gcnew String(abbrev) : nullptr;
        m_direction = direction ? gcnew String(direction) : nullptr;
        m_unit_conv_factor = unit_conv_factor;
        m_unit_name = unit_name ? gcnew String(unit_name) : nullptr;
        m_unit_auth_name = unit_name ? gcnew String(unit_auth_name) : nullptr;
        m_unit_code = unit_code ? gcnew String(unit_code) : nullptr;
    }
}