#include "pch.h"
#include "CoordinateTransformList.h"

System::Collections::Generic::IEnumerator<SharpProj::CoordinateTransform^>^ SharpProj::CoordinateTransformList::GetEnumerator()
{
	for (int i = 0; i < Count; i++)
	{
		if (!m_steps[i])
			GC::KeepAlive(this[i]);
	}

	return static_cast<System::Collections::Generic::IEnumerable<CoordinateTransform^>^>(m_steps)->GetEnumerator();
}

