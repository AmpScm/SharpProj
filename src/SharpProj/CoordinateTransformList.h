#pragma once
#include "CoordinateTransform.h"
namespace SharpProj {
	/// <summary>
	/// Represents a <see cref="CoordinateTransform"/> which is implemented in a number of discrete steps
	/// </summary>
	public ref class CoordinateTransformList : CoordinateTransform, IReadOnlyList<CoordinateTransform^>
	{
	private:
		array<CoordinateTransform^>^ m_steps;

	internal:
		CoordinateTransformList(ProjContext^ ctx, PJ* pj)
			:CoordinateTransform(ctx, pj)
		{

		}
	private:
		virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
		{
			return GetEnumerator();
		}

	public:
		// Inherited via IReadOnlyCollection
		virtual System::Collections::Generic::IEnumerator<SharpProj::CoordinateTransform^>^ GetEnumerator();

		// Inherited via IReadOnlyList
		virtual property int Count
		{
			int get()
			{
				if (!m_steps)
				{
					int n = proj_concatoperation_get_step_count(Context, this);

					if (n > 0)
						m_steps = gcnew array<CoordinateTransform^>(n);
					else
						m_steps = Array::Empty<CoordinateTransform^>();
				}
				return m_steps->Length;
			}
		}
		virtual property CoordinateTransform^ default[int]
		{
			CoordinateTransform ^ get(int index)
			{
				if (index < 0 || index >= Count)
					throw gcnew IndexOutOfRangeException();

				if (!m_steps[index])
					m_steps[index] = Context->Create<CoordinateTransform^>(proj_concatoperation_get_step(Context, this, index));

				return m_steps[index];
			}
		}

	public:
		virtual IReadOnlyList<CoordinateTransform^>^ Steps() override
		{
			return this;
		}
	};

}