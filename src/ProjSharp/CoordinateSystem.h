#pragma once
#include "ProjObject.h"
namespace ProjSharp {
    using System::Collections::ObjectModel::ReadOnlyCollection;
    using System::Collections::Generic::List;

    ref class CoordinateSystem;

    public enum class CoordinateSystemType
    {
        Unknown = PJ_CS_TYPE_UNKNOWN,

        Cartesian = PJ_CS_TYPE_CARTESIAN,
        Ellipsoidal = PJ_CS_TYPE_ELLIPSOIDAL,
        Vertical = PJ_CS_TYPE_VERTICAL,
        Spherical = PJ_CS_TYPE_SPHERICAL,
        Ordinal = PJ_CS_TYPE_ORDINAL,
        Parametric = PJ_CS_TYPE_PARAMETRIC,
        DateTimeTemporal = PJ_CS_TYPE_DATETIMETEMPORAL,
        TemporalCount = PJ_CS_TYPE_TEMPORALCOUNT,
        TemporalMeasure = PJ_CS_TYPE_TEMPORALMEASURE
    };

    public ref class ProjAxis
    {
        initonly CoordinateSystem^ m_cs;
        initonly int m_idx;
        String^ m_name;
        String^ m_abbrev;
        String^ m_direction;
        double m_unit_conv_factor;
        String^ m_unit_name;
        String^ m_unit_auth_name;
        String^ m_unit_code;
    internal:
        ProjAxis(CoordinateSystem^ cs, int idx)
        {
            m_cs = cs;
            m_idx = idx;
        }

        void Ensure();

    public:
        property String^ Name
        {
            String^ get()
            {
                Ensure();
                return m_name;
            }
        }

        property String^ Abbreviation
        {
            String^ get()
            {
                Ensure();
                return m_abbrev;
            }
        }

        property String^ Direction
        {
            String^ get()
            {
                Ensure();
                return m_direction;
            }
        }

        property double UnitConversionFactor
        {
            double get()
            {
                Ensure();
                return m_unit_conv_factor;
            }
        }

        property String^ UnitName
        {
            String^ get()
            {
                Ensure();
                return m_unit_name;
            }
        }

        property String^ UnitAuthName
        {
            String^ get()
            {
                Ensure();
                return m_unit_auth_name;
            }
        }

        property String^ UnitCode
        {
            String^ get()
            {
                Ensure();
                return m_unit_code;
            }
        }
    };

    public ref class CoordinateSystem :
        public ProjObject
    {
        ReadOnlyCollection<ProjAxis^>^ m_axis;
    internal:
        CoordinateSystem(ProjContext^ ctx, PJ* pj)
            : ProjObject(ctx, pj)
        {


        }

    public:
        property CoordinateSystemType CsType
        {
            CoordinateSystemType get()
            {
                return (CoordinateSystemType)proj_cs_get_type(Context, this);
            }
        }

        property int AxisCount
        {
            int get()
            {
                return proj_cs_get_axis_count(Context, this);
            }
        }


    public:
        property ReadOnlyCollection<ProjAxis^>^ Axis
        {
            ReadOnlyCollection<ProjAxis^>^ get()
            {
                if (!m_axis)
                {
                    int cnt = AxisCount;

                    if (cnt > 0)
                    {
                        List<ProjAxis^>^ lst = gcnew List<ProjAxis^>(cnt);

                        for (int i = 0; i < cnt; i++)
                            lst->Add(gcnew ProjAxis(this, i));

                        m_axis = lst->AsReadOnly();
                    }
                }
                return m_axis;
            }
        }
    };
}