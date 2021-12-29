#include "pch.h"
#include "ProjObject.h"

Identifier^ IdentifierList::default::get(int index)
{
    if (index < 0 || m_Items ? (index >= m_Items->Length) : !proj_get_id_code(m_object, index))
        throw gcnew IndexOutOfRangeException();

    if (m_Items)
    {
        if (!m_Items[index])
            m_Items[index] = gcnew Identifier(m_object, index);

        return m_Items[index];
    }
    else
    {
        // Count still unknown
        return gcnew Identifier(m_object, index);
    }
}

int IdentifierList::Count::get()
{
    if (m_Items)
        return m_Items->Length;

    for (int i = 0; i < 256 /* some MAX */; i++)
    {
        if (!proj_get_id_code(m_object, i))
        {
            m_Items = gcnew array<Identifier^>(i);
            return i;
        }
    }
    m_Items = EMPTY_ARRAY(Identifier^);
    return 0;
}

System::Collections::Generic::IEnumerator<Identifier^>^ IdentifierList::GetEnumerator()
{
    for (int i = 0; i < Count /* initializes m_Items if necessary */; i++)
    {
        if (!m_Items[i])
            m_Items[i] = gcnew Identifier(m_object, i);
    }

    return Array::AsReadOnly(m_Items)->GetEnumerator();
}

String^ Identifier::Authority::get()
{
    if (!m_authority && m_object)
    {
        const char* auth = proj_get_id_auth_name(m_object, m_index);

        m_authority = auth ? Utf8_PtrToString(auth) : nullptr;
    }
    return m_authority;
}

String^ Identifier::Code::get()
{
    if (!m_code && m_object)
    {
        const char* code = proj_get_id_code(m_object, m_index);

        m_code = Utf8_PtrToString(code);
    }
    return m_code;
}
