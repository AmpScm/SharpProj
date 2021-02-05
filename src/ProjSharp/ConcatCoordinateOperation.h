#pragma once
#include "CoordinateOperation.h"
namespace ProjSharp {
	public ref class ConcatCoordinateOperation : CoordinateOperation, IReadOnlyList<CoordinateOperation^>
	{
	private:
		array<CoordinateOperation^>^ m_steps;

	internal:
		ConcatCoordinateOperation(ProjContext^ ctx, PJ* pj)
			:CoordinateOperation(ctx, pj)
		{

		}
	private:
		virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
		{
			return GetEnumerator();
		}

	public:
		// Inherited via IReadOnlyCollection
		virtual System::Collections::Generic::IEnumerator<ProjSharp::CoordinateOperation^>^ GetEnumerator();

		// Inherited via IReadOnlyList
		virtual property int Count
		{
			int get()
			{
				if (!m_steps)
				{
					int n = proj_concatoperation_get_step_count(Context, this);

					if (n > 0)
						m_steps = gcnew array<CoordinateOperation^>(n);
					else
						m_steps = Array::Empty<CoordinateOperation^>();
				}
				return m_steps->Length;
			}
		}
		virtual property CoordinateOperation^ default[int]
		{
			CoordinateOperation ^ get(int index)
			{
				if (index < 0 || index >= Count)
					throw gcnew IndexOutOfRangeException();

				if (!m_steps[index])
					m_steps[index] = static_cast<CoordinateOperation^>(Context->Create(proj_concatoperation_get_step(Context, this, index)));

				return m_steps[index];
			}
		}
	};

}