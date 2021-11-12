#pragma once
#include "CoordinateReferenceSystem.h"
namespace SharpProj {
	using System::Collections::Generic::IReadOnlyList;

	/// <summary>
	/// A compound CoordinateReference system, based on a combination of more (typically two) base CoordinateReferenceSystems
	/// </summary>
	[DebuggerDisplayAttribute("Count={Count}")]
	public ref class CoordinateReferenceSystemList :
		public CoordinateReferenceSystem, IReadOnlyList<CoordinateReferenceSystem^>
	{
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		array<CoordinateReferenceSystem^>^ m_crs;
		[DebuggerBrowsable(DebuggerBrowsableState::Never)]
		Proj::AxisCollection^ m_axis;
	internal:
		CoordinateReferenceSystemList(ProjContext^ ctx, PJ* pj)
			: CoordinateReferenceSystem(ctx, pj)
		{

		}

	private:
		~CoordinateReferenceSystemList()
		{
			CoordinateReferenceSystemList::!CoordinateReferenceSystemList();
		}
		!CoordinateReferenceSystemList()
		{
			if (m_crs)
				try
			{
				for each (CoordinateReferenceSystem ^ cr in m_crs)
				{
					if (cr)
						delete cr;
				}
			}
			finally
			{
				m_crs = nullptr;
			}
		}

	private:
		// Inherited via IEnumerable
		virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
		{
			return GetEnumerator();
		}

	public:

		// Inherited via IReadOnlyCollection
		virtual System::Collections::Generic::IEnumerator<SharpProj::CoordinateReferenceSystem^>^ GetEnumerator()
		{
			array<CoordinateReferenceSystem^>^ crs;

			if (!Count)
				crs = EMPTY_ARRAY(CoordinateReferenceSystem^);
			else
				crs = m_crs;

			return static_cast<System::Collections::Generic::IEnumerable<CoordinateReferenceSystem^>^>(crs)->GetEnumerator();
		}


		// Inherited via IReadOnlyList
		virtual property int Count
		{
			int get()
			{
				if (!m_crs)
				{
					m_crs = gcnew array<CoordinateReferenceSystem^>{
						Context->Create<CoordinateReferenceSystem^>(proj_crs_get_sub_crs(Context, this, 0)),
							Context->Create<CoordinateReferenceSystem^>(proj_crs_get_sub_crs(Context, this, 1)),
					};
				}

				return m_crs ? m_crs->Length : 0;
			}
		}

		virtual property SharpProj::CoordinateReferenceSystem^ default[int]
		{
			CoordinateReferenceSystem ^ get(int index)
			{
				if (index < 0 || index >= Count)
					throw gcnew IndexOutOfRangeException();

				return m_crs[index];
			}
		}

			virtual property int AxisCount
		{
			int get() override;
		}

		property Proj::AxisCollection^ Axis
		{
			virtual Proj::AxisCollection^ get() override;
		}
	};
}