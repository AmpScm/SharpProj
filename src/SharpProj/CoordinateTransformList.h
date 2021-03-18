#pragma once
#include "CoordinateTransform.h"
namespace SharpProj {
	using System::Collections::Generic::IEnumerable;

	/// <summary>
	/// Represents a <see cref="CoordinateTransform"/> which is implemented in a number of discrete steps
	/// </summary>
	[DebuggerDisplay("[ChooseCoordinateTransform] Step Count={Count}")]
	public ref class CoordinateTransformList : CoordinateTransform, IReadOnlyList<CoordinateTransform^>
	{
	private:
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
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

		virtual IReadOnlyList<ProjOperation^>^ ProjOperations() override
		{
			auto ops = System::Linq::Enumerable::SelectMany<CoordinateTransform^, ProjOperation^>(this, gcnew System::Func<CoordinateTransform^, IEnumerable<ProjOperation^>^>(&Select_Steps));
			return System::Linq::Enumerable::ToList(ops)->AsReadOnly();
		}

		virtual CoordinateTransform^ CreateInverse([Optional]ProjContext^ ctx) override
		{
			if (!ctx)
				ctx = Context;

			if (!HasInverse)
				throw gcnew InvalidOperationException();

			return gcnew CoordinateOperation(ctx, proj_coordoperation_create_inverse(ctx, this));
		}

	private:
		static IEnumerable<ProjOperation^>^ Select_Steps(CoordinateTransform^ transform)
		{
			return transform->ProjOperations();
		}
	};

}