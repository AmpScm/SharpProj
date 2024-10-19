#include "pch.h"
#include "ProjFactory.h"

using namespace SharpProj;
using namespace SharpProj::Proj;

CoordinateSystem^ ProjFactory::CreateCoordinateSystem(CoordinateSystemType type, int axisCount, array<AxisDefinition^>^ axis)
{
    proj_create_cs; // TODO: Implement using this function
    throw gcnew NotImplementedException();
}

CoordinateSystem^ ProjFactory::CreateCartesianCoordinateSystem(Cartesian2DType type, String^ unitName, double conversionFactor)
{
    if (type < Cartesian2DType::EastingNorthing || type > Cartesian2DType::WestingSouthing)
        throw gcnew ArgumentOutOfRangeException(nameof(type));
    else if (String::IsNullOrEmpty(unitName))
        throw gcnew ArgumentNullException(nameof(unitName));

    auto unit_name = utf8_string(unitName);

    auto pj = proj_create_cartesian_2D_cs(this, (PJ_CARTESIAN_CS_2D_TYPE)type, unit_name.c_str(), conversionFactor);

    if (pj)
        return gcnew CoordinateSystem(m_context, pj);
    else
        throw m_context->ConstructException("CreateCartesianCoordinateSystem");
}

CoordinateSystem^ ProjFactory::CreateEllipsoidalCoordinateSystem(Ellipsoidal2DType type, String^ unitName, double conversionFactor)
{
    if (type < Ellipsoidal2DType::LongitudeLatitude || type > Ellipsoidal2DType::LatitudeLongitude)
        throw gcnew ArgumentOutOfRangeException(nameof(type));
    else if (String::IsNullOrEmpty(unitName))
        throw gcnew ArgumentNullException(nameof(unitName));

    auto unit_name = utf8_string(unitName);

    auto pj = proj_create_ellipsoidal_2D_cs(this, (PJ_ELLIPSOIDAL_CS_2D_TYPE)type, unit_name.c_str(), conversionFactor);

    if (pj)
        return gcnew CoordinateSystem(m_context, pj);
    else
        throw m_context->ConstructException("CreateEllipsoidalCoordinateSystem");
}

CoordinateSystem^ ProjFactory::CreateEllipsoidalCoordinateSystem(Ellipsoidal3DType type, String^ horizontalUnitName, double horizontalConversionFactor, String^ verticalUnitName, double verticalConversionFactor)
{
    if (type < Ellipsoidal3DType::LongitudeLatitudeHeight || type > Ellipsoidal3DType::LatitudeLongitudeHeight)
        throw gcnew ArgumentOutOfRangeException(nameof(type));
    else if (String::IsNullOrEmpty(horizontalUnitName))
        throw gcnew ArgumentNullException(nameof(horizontalUnitName));
    else if (String::IsNullOrEmpty(verticalUnitName))
        throw gcnew ArgumentNullException(nameof(verticallUnitName));

    auto h_unit_name = utf8_string(horizontalUnitName);
    auto v_unit_name = utf8_string(verticalUnitName);

    auto pj = proj_create_ellipsoidal_3D_cs(this, (PJ_ELLIPSOIDAL_CS_3D_TYPE)type,
        h_unit_name.c_str(), horizontalConversionFactor,
        v_unit_name.c_str(), verticalConversionFactor);

    if (pj)
        return gcnew CoordinateSystem(m_context, pj);
    else
        throw m_context->ConstructException("CreateEllipsoidalCoordinateSystem");
}
