#include "pch.h"
#include "MultiCoordinateOperation.h"

System::Collections::Generic::IEnumerator<SharpProj::CoordinateOperation^>^ SharpProj::MultiCoordinateOperation::GetEnumerator()
{
	for (int i = 0; i < Count; i++)
	{
		if (!m_steps[i])
			GC::KeepAlive(this[i]);
	}

	return static_cast<System::Collections::Generic::IEnumerable<CoordinateOperation^>^>(m_steps)->GetEnumerator();
}
