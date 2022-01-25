#pragma once
#include "CoordinateTransform.h"

namespace SharpProj {
    using System::Collections::Generic::IReadOnlyList;
    ref class CoordinateReferenceSystem;
    ref class Proj::ProjArea;

    /// <summary>
    /// Represents a <see cref="CoordinateTransform"/> which is implemented in a number of ways. The best
    /// implementation is chosen at runtime, based on some predefined settings. (pyproj name: 'TransformerGroup')
    /// </summary>
    [DebuggerDisplay("[ChooseCoordinateTransform] Option Count={Count}")]
    public ref class ChooseCoordinateTransform : CoordinateTransform, IReadOnlyList<CoordinateTransform^>
    {
    private:
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        PJ_OBJ_LIST* m_list;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        array<CoordinateTransform^>^ m_operations;
        [DebuggerBrowsable(DebuggerBrowsableState::Never)]
        CoordinateTransform^ m_last;

    internal:
        ChooseCoordinateTransform(ProjContext^ ctx, PJ* pj, PJ_OBJ_LIST* list)
            : CoordinateTransform(ctx, pj)
        {
            m_list = list;

            array<CoordinateTransform^>^ items = gcnew array<CoordinateTransform^>(proj_list_get_count(list));

            for (int i = 0; i < items->Length; i++)
            {
                items[i] = ctx->Create<CoordinateTransform^>(proj_list_get(Context, m_list, i));
            }
            m_operations = items;

            ForceUnknownInfo();
            Name = "<choose-coordinate-transform>";
        }

    private:
        !ChooseCoordinateTransform()
        {
            if (m_list)
            {
                proj_list_destroy(m_list);
                m_list = nullptr;
            }
        }

        ~ChooseCoordinateTransform()
        {
            if (m_list)
            {
                proj_list_destroy(m_list);
                m_list = nullptr;
            }
            if (m_operations)
            {
                array<CoordinateTransform^>^ ops = m_operations;
                m_operations = nullptr;
                for each (CoordinateTransform ^ o in ops)
                {
                    try
                    {
                        delete o;
                    }
                    catch (Exception^)
                    {
                    } // Already disposed, other errors, etc.
                }
            }
        }

    protected:
        virtual PPoint DoTransform(bool forward, PPoint% coordinate) override;
        virtual void DoTransform(bool forward,
            double* xVals, int xStep, int xCount,
            double* yVals, int yStep, int yCount,
            double* zVals, int zStep, int zCount,
            double* tVals, int tStep, int tCount) override;
    private:
        virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
        {
            return GetEnumerator();
        }

    public:
        int SuggestedOperation(PPoint coordinate);
        int SuggestedOperation(...array<double>^ ordinates) { return SuggestedOperation(PPoint(ordinates)); }

    public:
        // Inherited via IReadOnlyCollection
        virtual System::Collections::Generic::IEnumerator<SharpProj::CoordinateTransform^>^ GetEnumerator() sealed
        {
            return static_cast<System::Collections::Generic::IEnumerable<CoordinateTransform^>^>(m_operations)->GetEnumerator();
        }
        virtual property int Count
        {
            int get()
            {
                return m_operations->Length;
            }
        }

        property CoordinateTransform^ default[int]
        {
            virtual CoordinateTransform ^ get(int index) sealed
            {
                return m_operations[index];
            }
        }

            property bool HasInverse
        {
            virtual bool get() override sealed
            {
                for each (auto c in this)
                {
                    if (c->HasInverse)
                        return true;
                }

                return false;
            }
        }

        property virtual bool IsAvailable
        {
            virtual bool get() override sealed
            {
                for each (auto c in this)
                {
                    if (c->IsAvailable)
                        return true;
                }

                return false;
            }
        }

        property ProjType Type
        {
            virtual ProjType get() override
            {
                return ProjType::ChooseTransform;
            }
        }

    public:
        virtual IReadOnlyList<CoordinateTransform^>^ Options() override
        {
            return this;
        }

        virtual IReadOnlyList<ProjOperation^>^ ProjOperations() override
        {
            return Array::AsReadOnly(EMPTY_ARRAY(Proj::ProjOperation^));
        }
    };
}
