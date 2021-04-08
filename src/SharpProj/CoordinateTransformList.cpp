#include "pch.h"
#include "CoordinateTransformList.h"

CoordinateTransformList::CoordinateTransformList(ProjContext^ ctx, PJ* pj)
	: CoordinateTransform(ctx, pj)
{

}

CoordinateTransformList::~CoordinateTransformList()
{
	if (m_steps)
	{
		auto steps = m_steps;
		m_steps = nullptr;

		for each (auto v in steps)
		{
			if ((Object^)v)
				delete v;
		}
	}
}

System::Collections::Generic::IEnumerator<SharpProj::CoordinateTransform^>^ SharpProj::CoordinateTransformList::GetEnumerator()
{
	for (int i = 0; i < Count; i++)
	{
		if (!m_steps[i])
			GC::KeepAlive(this[i]);
	}

	return static_cast<System::Collections::Generic::IEnumerable<CoordinateTransform^>^>(m_steps)->GetEnumerator();
}

