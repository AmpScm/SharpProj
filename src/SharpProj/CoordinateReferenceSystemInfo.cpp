#include "pch.h"
#include "ProjObject.h"
#include "CoordinateReferenceSystem.h"
#include "CoordinateReferenceSystemInfo.h"

private ref class CRSComparer : System::Collections::Generic::IComparer<CoordinateReferenceSystemInfo^>
{
public:
    // Inherited via IComparer
    virtual int Compare(SharpProj::Proj::CoordinateReferenceSystemInfo^ x, SharpProj::Proj::CoordinateReferenceSystemInfo^ y)
    {
        int n = StringComparer::OrdinalIgnoreCase->Compare(x->Authority, y->Authority);
        if (n != 0)
            return n;

        int xi, yi;

        if (int::TryParse(x->Code, xi) && int::TryParse(y->Code, yi))
        {
            n = xi - yi;

            if (n != 0)
                return n;
        }

        return StringComparer::OrdinalIgnoreCase->Compare(x->Name, y->Name);
    }

    static initonly CRSComparer^ Instance = gcnew CRSComparer();
};

ReadOnlyCollection<CoordinateReferenceSystemInfo^>^ ProjContext::GetCoordinateReferenceSystems(CoordinateReferenceSystemFilter^ filter)
{
    if (!filter)
        throw gcnew ArgumentNullException("filter");

    std::string auth_name;
    if (filter->Authority)
        auth_name = ::utf8_string(filter->Authority);
    PROJ_CRS_LIST_PARAMETERS* params = proj_get_crs_list_parameters_create();

    try
    {
        auto types = filter->Types->ToArray();
        pin_ptr<ProjType> pTypes;

        if (types->Length)
        {
            pTypes = &types[0];
            params->typesCount = types->Length;
            params->types = reinterpret_cast<PJ_TYPE*>(pTypes);
        }

        params->allow_deprecated = filter->AllowDeprecated;

        if (filter->CoordinateArea)
        {
            params->bbox_valid = true;

            params->west_lon_degree = filter->CoordinateArea->WestLongitude;
            params->south_lat_degree = filter->CoordinateArea->SouthLatitude;
            params->east_lon_degree = filter->CoordinateArea->EastLongitude;
            params->north_lat_degree = filter->CoordinateArea->NorthLatitude;

            params->crs_area_of_use_contains_bbox = filter->CompletelyContainsArea;
        }

        if (filter->CelestialBodyName)
        {
            params->celestial_body_name = utf8_string(filter->CelestialBodyName);
        }

        int count;
        array<CoordinateReferenceSystemInfo^>^ result;

        PROJ_CRS_INFO** infoList = proj_get_crs_info_list_from_database(this, auth_name.length() ? auth_name.c_str() : nullptr, params, &count);
        try
        {
            result = gcnew array<CoordinateReferenceSystemInfo^>(count);

            for (int i = 0; i < count; i++)
                result[i] = gcnew CoordinateReferenceSystemInfo(infoList[i], this);
        }
        finally
        {
            proj_crs_info_list_destroy(infoList);
        }

        Array::Sort(result, CRSComparer::Instance);
        return Array::AsReadOnly(result);
    }
    finally
    {
        proj_get_crs_list_parameters_destroy(params);
    }
}

ReadOnlyCollection<CoordinateReferenceSystemInfo^>^ ProjContext::GetCoordinateReferenceSystems()
{
    return GetCoordinateReferenceSystems(gcnew CoordinateReferenceSystemFilter());
}

CoordinateReferenceSystem^ CoordinateReferenceSystemInfo::Create(ProjContext^ ctx)
{
    if (!ctx)
        ctx = _ctx;

    return CoordinateReferenceSystem::CreateFromDatabase(Authority, Code, ctx);
}
