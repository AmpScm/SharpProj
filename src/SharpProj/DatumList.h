#pragma once
#include "Datum.h"
namespace SharpProj {
    using System::Collections::Generic::IReadOnlyList;

    namespace ProjDetaile {
        public ref class DatumList :
            public Datum, IReadOnlyList<Datum^>
        {
        private:
            array<Datum^>^ m_Items;
        internal:
            DatumList(ProjContext^ ctx, PJ* pj)
                : Datum(ctx, pj)
            {
                m_Items = gcnew array<Datum^>(proj_datum_ensemble_get_member_count(ctx, pj));

                for (int i = 0; i < m_Items->Length; i++)
                    m_Items[i] = gcnew Datum(ctx, proj_datum_ensemble_get_member(ctx, pj, i));
            }

        private:
            ~DatumList()
            {
                if (m_Items)
                {
                    array<Datum^>^ items = m_Items;
                    m_Items = nullptr;

                    for each (Datum ^ d in items)
                        delete d;
                }
            }

        public:
            property Datum^ default[int]
            {
                virtual Datum ^ get(int index) sealed
                {
                    return m_Items[index];
                }
            }

                // Inherited via IEnumerable
            // Inherited via IReadOnlyCollection
                virtual System::Collections::Generic::IEnumerator<Datum^>^ GetEnumerator() sealed
            {
                return static_cast<System::Collections::Generic::IEnumerable<Datum^>^>(m_Items)->GetEnumerator();
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

    using ProjDetaile::DatumList;
}
