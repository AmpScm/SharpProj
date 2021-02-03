#pragma once
#include "ProjDatum.h"
namespace ProjSharp {
    using System::Collections::Generic::IReadOnlyList;

    public ref class ProjDatumList :
        public ProjDatum, IReadOnlyList<ProjDatum^>
    {
    private:
        array<ProjDatum^>^ m_Items;
    internal:
        ProjDatumList(ProjContext^ ctx, PJ* pj)
            : ProjDatum(ctx, pj)
        {
            m_Items = gcnew array<ProjDatum^>(proj_datum_ensemble_get_member_count(ctx, pj));

            for (int i = 0; i < m_Items->Length; i++)
                m_Items[i] = gcnew ProjDatum(ctx, proj_datum_ensemble_get_member(ctx, pj, i));
        }

    private:
        ~ProjDatumList()
        {
            if (m_Items)
            {
                array<ProjDatum^>^ items = m_Items;
                m_Items = nullptr;

                for each (ProjDatum ^ d in items)
                    delete d;
            }
        }

    public:
        property ProjDatum^ default[int]
        {
            virtual ProjDatum^ get(int index) sealed
            {
                return m_Items[index];
            }
        }

            // Inherited via IEnumerable
        // Inherited via IReadOnlyCollection
        virtual System::Collections::Generic::IEnumerator<ProjDatum^>^ GetEnumerator() sealed
        {
            return static_cast<System::Collections::Generic::IEnumerable<ProjDatum^>^>(m_Items)->GetEnumerator();
        }
        property int Count
        {
            virtual int get() sealed
            {
                return m_Items ? m_Items->Length : 0;
            }
        }

    private:
        virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
        {
            return GetEnumerator();
        }


    };
}
