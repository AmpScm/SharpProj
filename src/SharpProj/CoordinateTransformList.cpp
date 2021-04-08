#include "pch.h"
#include "CoordinateTransformList.h"


CoordinateTransformList::~CoordinateTransformList()
{
	if (m_steps)
	{
		try
		{
			for each (auto v in m_steps)
			{
				if (v)
				{
					try
					{
						delete v;
					}
					catch (Exception^)
					{}
				}
			}
		}
		finally
		{
			m_steps = nullptr;
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

