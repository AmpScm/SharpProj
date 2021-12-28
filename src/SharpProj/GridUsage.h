#pragma once
#include "CoordinateTransform.h"

namespace SharpProj {
    ref class CoordinateTransform;
    namespace Proj {
        [DebuggerDisplay("{Name,nq}")]
        public ref class GridUsage
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly CoordinateTransform^ m_transform;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            int m_index;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_fullname;

        internal:
            GridUsage(CoordinateTransform^ transform, int index)
            {
                m_transform = transform;
                m_index = index;
            }

        public:
            property String^ Name
            {
                String^ get()
                {
                    if (!m_name)
                    {
                        const char* pName;
                        if (proj_coordoperation_get_grid_used(m_transform->Context, m_transform, m_index,
                            &pName, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
                        {
                            m_name = Utf8_PtrToString(pName);
                        }
                    }
                    return m_name;
                }
            }

            property String^ FullName
            {
                String^ get()
                {
                    if (!m_fullname)
                    {
                        const char* pFullName;
                        if (proj_coordoperation_get_grid_used(m_transform->Context, m_transform, m_index,
                            nullptr, &pFullName, nullptr, nullptr, nullptr, nullptr, nullptr))
                        {
                            m_fullname = Utf8_PtrToString(pFullName);
                        }
                    }
                    return m_fullname;
                }
            }

            property bool IsAvailable
            {
                bool get()
                {
                    int available;

                    if (proj_coordoperation_get_grid_used(m_transform->Context, m_transform, m_index,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &available))
                    {
                        return (available != 0);
                    }
                    return false;
                }
            }
        };
    }
}