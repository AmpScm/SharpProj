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
    std::string body_name;
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
            body_name = ::utf8_string(filter->CelestialBodyName);
            params->celestial_body_name = body_name.c_str();
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

ReadOnlyCollection<GeoidModelInfo^>^ CoordinateReferenceSystemInfo::GetGeoidModels()
{
    if (!_geoidModels)
    {
        std::string auth = utf8_string(Authority);
        std::string code = utf8_string(Code);
        PROJ_STRING_LIST geoid_list = proj_get_geoid_models_from_database(_ctx, auth.c_str(), code.c_str(), nullptr);

        if (!geoid_list)
            throw _ctx->ConstructException("GetGeoidModels");

        array<String^>^ geoids = ProjObject::FromStringList(geoid_list);
        proj_string_list_destroy(geoid_list);

        List<GeoidModelInfo^>^ gmi = gcnew List<GeoidModelInfo^>(geoids->Length);

        for each (auto s in geoids)
            gmi->Add(gcnew GeoidModelInfo(Authority, s));

        _geoidModels = gmi->AsReadOnly();
    }

    return _geoidModels;
}

private ref class CelestialBodyComparer : System::Collections::Generic::IComparer<CelestialBodyInfo^>
{
public:
    // Inherited via IComparer
    virtual int Compare(SharpProj::Proj::CelestialBodyInfo^ x, SharpProj::Proj::CelestialBodyInfo^ y)
    {
        if (x->IsEarth != y->IsEarth)
        {
            return (y->IsEarth ? 1 : 0) - (x->IsEarth ? 1 : 0);
        }

        int n = StringComparer::OrdinalIgnoreCase->Compare(x->Authority, y->Authority);
        if (n != 0)
            return n;

        return StringComparer::OrdinalIgnoreCase->Compare(x->Name, y->Name);
    }

    static initonly CelestialBodyComparer^ Instance = gcnew CelestialBodyComparer();
};


System::Collections::ObjectModel::ReadOnlyCollection<CelestialBodyInfo^>^ ProjContext::GetCelestialBodies()
{
    int count;

    array<CelestialBodyInfo^>^ result;
    PROJ_CELESTIAL_BODY_INFO** infoList = proj_get_celestial_body_list_from_database(this, nullptr /* auth_name*/, &count);
    try
    {
        result = gcnew array<CelestialBodyInfo^>(count);

        for (int i = 0; i < count; i++)
            result[i] = gcnew CelestialBodyInfo(infoList[i]);
    }
    finally
    {
        proj_celestial_body_list_destroy(infoList);
    }

    Array::Sort(result, CelestialBodyComparer::Instance);
    return Array::AsReadOnly(result);
}

System::Collections::ObjectModel::ReadOnlyCollection<String^>^ ProjContext::GetAuthorities()
{
    array<String^>^ result;
    auto infoList = proj_get_authorities_from_database(this);
    try
    {
        const char** pItem = const_cast<const char**>(infoList);
        int count = 0;

        while (*pItem)
        {
            count++;
            pItem++;
        }

        result = gcnew array<String^>(count);

        for (int i = 0; i < count; i++)
            result[i] = Utf8_PtrToString(infoList[i]);
    }
    finally
    {
        proj_string_list_destroy(infoList);
    }

    return Array::AsReadOnly(result);
}

System::Collections::ObjectModel::ReadOnlyCollection<UnitOfMeasurement^>^ ProjContext::GetUnitsOfMeasurement()
{
    return GetUnitsOfMeasurement(gcnew UnitOfMeasurementFilter());
}

private ref class UnitOfMeasurementComparer : System::Collections::Generic::IComparer<UnitOfMeasurement^>
{
public:
    // Inherited via IComparer
    virtual int Compare(SharpProj::Proj::UnitOfMeasurement^ x, SharpProj::Proj::UnitOfMeasurement^ y)
    {
        int n = StringComparer::OrdinalIgnoreCase->Compare(x->Category, y->Category);

        if (n != 0)
            return n;

        if (x->IsDeprecated != y->IsDeprecated)
        {
            return (x->IsDeprecated ? 1 : -1);
        }

        if ((x->ConversionFactor == 1.0) != (y->ConversionFactor == 1.0))
        {
            return ((x->ConversionFactor == 1.0) ? -1 : 0);
        }

        return StringComparer::OrdinalIgnoreCase->Compare(x->Name, y->Name);
    }

    static initonly UnitOfMeasurementComparer^ Instance = gcnew UnitOfMeasurementComparer();
};

System::Collections::ObjectModel::ReadOnlyCollection<UnitOfMeasurement^>^ ProjContext::GetUnitsOfMeasurement(UnitOfMeasurementFilter^ filter)
{
    if (!filter)
        throw gcnew ArgumentNullException("filter");

    std::string auth_name;
    std::string category;

    if (filter->Authority)
        auth_name = ::utf8_string(filter->Authority);
    if (filter->Category)
        category = ::utf8_string(filter->Category);

    int count;
    auto r = proj_get_units_from_database(this,
        auth_name.length() ? auth_name.c_str() : nullptr,
        category.length() ? category.c_str() : nullptr,
        !filter->NoDeprecated, &count);

    array< UnitOfMeasurement^>^ result;
    try
    {
        result = gcnew array<UnitOfMeasurement^>(count);

        for (int i = 0; i < count; i++)
            result[i] = gcnew UnitOfMeasurement(r[i]);
    }
    finally
    {
        proj_unit_list_destroy(r);
    }

    Array::Sort(result, UnitOfMeasurementComparer::Instance);

    return Array::AsReadOnly(result);
}