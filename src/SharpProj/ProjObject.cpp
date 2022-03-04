#include "pch.h"
#include "ProjObject.h"
#include "ProjException.h"
#include "PPoint.h"
#include "CoordinateTransform.h"
#include "CoordinateTransformList.h"
#include "CoordinateReferenceSystem.h"
#include "CoordinateReferenceSystemList.h"
#include "ChooseCoordinateTransform.h"
#include "CoordinateSystem.h"
#include "Ellipsoid.h"
#include "GeographicCRS.h"
#include "PrimeMeridian.h"
#include "ReferenceFrame.h"
#include "DatumList.h"
#include "Datum.h"
#include "UsageArea.h"

ProjObject::ProjObject(ProjContext^ ctx, PJ* pj)
    : m_ctx(ctx), m_pj(*new item_wrapper<PJ_CONTEXT, ProjContext, PJ>(ctx, pj, proj_destroy))
{
    if (!ctx)
        throw gcnew ArgumentNullException("ctx");
    else if (!pj)
        throw gcnew ArgumentNullException("pj");
}

ProjObject::!ProjObject()
{
    if (_disposed)
        return;
    _disposed = true;
    m_pj.Release();
}

ProjObject::~ProjObject()
{
    if (m_usageArea)
        m_usageArea->InternalDispose();
    ProjObject::!ProjObject();
}


ProjObject^ ProjContext::Create(String^ definition)
{
    if (String::IsNullOrWhiteSpace(definition))
        throw gcnew ArgumentNullException("definition");

    std::string fromStr = utf8_string(definition);
    PJ* pj = proj_create(this, fromStr.c_str());

    if (!pj)
        throw ConstructException("Create proj from definition failed");

    return Create(pj);
}

ProjObject^ ProjContext::CreateProj4Compatible(String^ definition)
{
    if (String::IsNullOrWhiteSpace(definition))
        throw gcnew ArgumentNullException("definition");

    std::string fromStr = utf8_string(definition);
    bool wasProj4 = (0 != proj_context_get_use_proj4_init_rules(this, false));

    PJ* pj;
    proj_context_use_proj4_init_rules(this, true);
    try
    {
        pj = proj_create(this, fromStr.c_str());
    }
    finally
    {
        proj_context_use_proj4_init_rules(this, wasProj4);
    }

    if (!pj)
        throw ConstructException("Create proj from definition failed");

    return Create(pj);
}

ProjObject^ ProjContext::Create(...array<String^>^ from)
{
    char** lst = new char* [from->Length + 1];
    for (int i = 0; i < from->Length; i++)
    {
        std::string fromStr = utf8_string(from[i]);
        lst[i] = _strdup(fromStr.c_str());
    }
    lst[from->Length] = 0;

    try
    {
        PJ* pj = proj_create_argv(this, from->Length, lst);

        if (!pj)
            throw ConstructException("Create proj from definition list failed");

        return Create(pj);
    }
    finally
    {
        for (int i = 0; i < from->Length; i++)
        {
            free(lst[i]);
        }
        delete[] lst;
    }
}

ProjObject^ ProjContext::CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options)
{
    array<String^>^ wars = nullptr;

    return CreateFromWellKnownText(from, wars);
}

ProjObject^ ProjContext::CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Out] array<String^>^% warnings)
{
    if (String::IsNullOrWhiteSpace(from))
        throw gcnew ArgumentNullException("from");

    PROJ_STRING_LIST wrs = nullptr;
    PROJ_STRING_LIST errs = nullptr;
    const char* c_options[8] = {};
    int nOption = 0;

    if (options)
    {
        if (options->Strict)
        {
            c_options[nOption++] = "STRICT=YES";
        }
    }

    std::string fromStr = utf8_string(from);
    PJ* pj = proj_create_from_wkt(this, fromStr.c_str(), c_options, &wrs, &errs);

    warnings = ProjObject::FromStringList(wrs);
    array<String^>^ errors = ProjObject::FromStringList(errs);

    if (wrs)
        proj_string_list_destroy(wrs);
    if (errs)
        proj_string_list_destroy(errs);

    if (!pj)
    {
        Exception^ ex = ConstructException(nullptr);
        if (errors && errors->Length)
        {
            Exception^ ex2 = (ex && ex->Message->Length) ? ex : nullptr;

            for each (String ^ msg in errors)
            {
                ex2 = gcnew ProjException(msg, ex2);
            }

            if (ex2)
                throw ex2;
        }
        throw ex;
    }

    return Create(pj);
}

ProjObject^ ProjContext::Create(PJ* pj)
{
    if (!pj)
        throw gcnew ArgumentNullException("pj");

    switch ((ProjType)proj_get_type(pj))
    {
    case ProjType::Ellipsoid:
        return gcnew Ellipsoid(this, pj);

    case ProjType::PrimeMeridian:
        return gcnew Proj::PrimeMeridian(this, pj);

    case ProjType::GeodeticReferenceFrame:
    case ProjType::DynamicGeodeticReferenceFrame:
    case ProjType::VerticalReferenceFrame:
    case ProjType::DynamicVerticalReferenceFrame:
        return gcnew Proj::ReferenceFrame(this, pj);

    case ProjType::DatumEnsamble:
        return gcnew Proj::DatumList(this, pj);

    case ProjType::GeographicCrs: // Never used. Only inherited
    case ProjType::Geographic2DCrs:
    case ProjType::Geographic3DCrs:
        return gcnew GeographicCRS(this, pj);

    case ProjType::GeodeticCrs:
    case ProjType::GeocentricCrs:
        return gcnew GeodeticCRS(this, pj);

    case ProjType::CompoundCrs:
        return gcnew CoordinateReferenceSystemList(this, pj);

    case ProjType::CRS: // abstract
    case ProjType::VerticalCrs:
    case ProjType::ProjectedCrs:
    case ProjType::TemporalCrs:
    case ProjType::EngineeringCrs:
    case ProjType::OtherCrs:
        return gcnew CoordinateReferenceSystem(this, pj);

    case ProjType::BoundCrs:
        return gcnew BoundCRS(this, pj);

    case ProjType::Conversion:
    case ProjType::Transformation:
    case ProjType::OtherCoordinateTransform:
        return gcnew CoordinateOperation(this, pj);

    case ProjType::ConcatenatedOperation:
        return gcnew CoordinateTransformList(this, pj);

    case ProjType::TemporalDatum:
    case ProjType::EngineeringDatum:
    case ProjType::ParametricDatum:
        return gcnew Proj::Datum(this, pj);

    case ProjType::ChooseTransform:
    case ProjType::CoordinateSystem:
        throw gcnew InvalidOperationException(); // Never returned by proj, but needed for sensible API

    case ProjType::Unknown:
    default:
        CoordinateSystemType cst = (CoordinateSystemType)proj_cs_get_type(this, pj);

        if (cst != CoordinateSystemType::Unknown)
            return gcnew CoordinateSystem(this, pj);

        ClearError(pj);


        if (!strcmp(proj_get_name(pj), "Transformation pipeline manager"))
        {
            // Should be a ChooseCoordinateTransform instance, based on the inner data
            return gcnew CoordinateOperation(this, pj);
        }

        return gcnew ProjObject(this, pj);
    }
}

generic<typename T> where T : ProjObject
T ProjContext::Create(PJ* pj)
{
    ProjObject^ o = Create(pj);

    return safe_cast<T>(o);
}

ProjObject^ ProjObject::Create(String^ definition, [Optional]ProjContext^ ctx)
{
    bool createdCtx = false;
    if (!ctx)
    {
        ctx = gcnew ProjContext();
        createdCtx = true;
    }

    try
    {
        return ctx->Create(definition);
    }
    catch (Exception^)
    {
        if (createdCtx)
            delete ctx;

        throw;
    }
}

ProjObject^ ProjObject::Create(array<String^>^ from, [Optional]ProjContext^ ctx)
{
    bool createdCtx = false;
    if (!ctx)
    {
        ctx = gcnew ProjContext();
        createdCtx = true;
    }

    try
    {
        return ctx->Create(from);
    }
    catch (Exception^)
    {
        if (createdCtx)
            delete ctx;

        throw;
    }
}

ProjObject^ ProjObject::CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Optional] ProjContext^ ctx)
{
    bool createdCtx = false;
    if (!ctx)
    {
        ctx = gcnew ProjContext();
        createdCtx = true;
    }

    try
    {
        return ctx->CreateFromWellKnownText(from, options);
    }
    catch (Exception^)
    {
        if (createdCtx)
            delete ctx;

        throw;
    }
}

ProjObject^ ProjObject::CreateFromWellKnownText(String^ from, CreateFromWKTOptions^ options, [Out] array<String^>^% warnings, [Optional] ProjContext^ ctx)
{
    bool createdCtx = false;
    if (!ctx)
    {
        ctx = gcnew ProjContext();
        createdCtx = true;
    }

    try
    {
        return ctx->CreateFromWellKnownText(from, options, warnings);
    }
    catch (Exception^)
    {
        if (createdCtx)
            delete ctx;

        throw;
    }
}

IdentifierList^ ProjObject::Identifiers::get()
{
    if (!m_idList && !m_noProj && proj_get_id_code(this, 0))
        m_idList = gcnew IdentifierList(this);

    return m_idList;
}

String^ ProjObject::Remarks::get()
{
    if (!m_remarks && !m_noProj)
    {
        const char* remarks = proj_get_remarks(this);

        m_remarks = Utf8_PtrToString(remarks);
    }

    if (m_remarks && m_remarks->Length == 0)
        return nullptr;
    else
        return m_remarks;
}

String^ ProjObject::Scope::get()
{
    if (!m_scope && !m_noProj)
    {
        const char* scope = proj_get_scope(this);

        m_scope = Utf8_PtrToString(scope);
    }
    return m_scope;
}

