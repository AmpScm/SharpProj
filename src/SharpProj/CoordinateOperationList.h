#pragma once
#include "CoordinateOperation.h"

namespace SharpProj {
	using System::Collections::Generic::IReadOnlyList;
	ref class CoordinateReferenceSystem;
	ref class ProjArea;

	/// <summary>
	/// Represents a <see cref="CoordinateOperation"/> which is implemented in a number of ways. The best
	/// implementation is chosen at runtime, based on some predefined settings.
	/// </summary>
	public ref class CoordinateOperationList : CoordinateOperation, IReadOnlyList<CoordinateOperation^>
	{
	private:
		PJ_OBJ_LIST* m_list;
		array<CoordinateOperation^>^ m_operations;
		CoordinateOperation^ m_last;

	internal:
		CoordinateOperationList(ProjContext^ ctx, PJ* pj, PJ_OBJ_LIST* list)
			: CoordinateOperation(ctx, pj)
		{
			m_list = list;

			array<CoordinateOperation^>^ items = gcnew array<CoordinateOperation^>(proj_list_get_count(list));

			for (int i = 0; i < items->Length; i++)
			{
				items[i] = static_cast<CoordinateOperation^>(ctx->Create(proj_list_get(Context, m_list, i)));
			}
			m_operations = items;

			ForceUnknownInfo();
		}

	private:
		~CoordinateOperationList()
		{
			if (m_list)
			{
				proj_list_destroy(m_list);
				m_list = nullptr;
			}
			if (m_operations)
			{
				array<CoordinateOperation^>^ ops = m_operations;
				m_operations = nullptr;
				for each (CoordinateOperation ^ o in ops)
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
		virtual ProjCoordinate DoTransform(bool forward, ProjCoordinate% coordinate) override;
	private:
		virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
		{
			return GetEnumerator();
		}

	public:
		int SuggestedOperation(ProjCoordinate coordinate);
		int SuggestedOperation(...array<double>^ coordinate) { return SuggestedOperation(ProjCoordinate::FromArray(coordinate)); }

	public:
		// Inherited via IReadOnlyCollection
		virtual System::Collections::Generic::IEnumerator<SharpProj::CoordinateOperation^>^ GetEnumerator() sealed
		{
			return static_cast<System::Collections::Generic::IEnumerable<CoordinateOperation^>^>(m_operations)->GetEnumerator();
		}
		virtual property int Count
		{
			int get()
			{
				return m_operations->Length;
			}
		}

		property CoordinateOperation^ default[int]
		{
			virtual CoordinateOperation ^ get(int index) sealed
			{
				return m_operations[index];
			}
		}

		property bool HasInverse
		{
			virtual bool get() override sealed
			{
				return false; // Unable to fetch if the operation is really inverted
			}
		}
	};
}
