namespace SharpProj {
    using System::Collections::Generic::IReadOnlyList;

    namespace Proj {

        [DebuggerDisplay("{ToString(),nq}")]
        public ref class Identifier sealed : IEquatable<Identifier^>
        {
        internal:
            initonly ProjObject^ m_object;
            initonly int m_index;
            String^ m_authority;
            String^ m_code;

            Identifier(ProjObject^ object, int index)
            {
                m_object = object;
                m_index = index;
            }

        public:
            Identifier(String^ authority, String^ code)
            {
                if (String::IsNullOrEmpty(authority))
                    throw gcnew ArgumentNullException("authority");
                if (String::IsNullOrEmpty(code))
                    throw gcnew ArgumentNullException("code");

                m_authority = authority;
                m_code = code;
            }

            Identifier(String^ authority, int code)
                : Identifier(authority, code.ToString())
            {}

        public:
            property String^ Authority
            {
                String^ get();
            }

            property String^ Code
            {
                String^ get();
            }

            [Obsolete("Use .Code"), DebuggerBrowsable(DebuggerBrowsableState::Never)]
            property String^ Name
            {
                String^ get() { return Code; }
            }

            virtual String^ ToString() override
            {
                return Authority + ":" + Code;
            }

            virtual bool Equals(Object^ other) override
            {
                return Equals(dynamic_cast<Identifier^>(other));
            }

            virtual bool Equals(Identifier^ otherId)
            {
                if (otherId == nullptr)
                    return false;
                else
                    return otherId->Authority == Authority && otherId->Name == Name;
            }

            virtual int GetHashCode() override
            {
                return Code ? Code->GetHashCode() : 0;
            }
        };

        [DebuggerDisplay("Count = {Count}")]
        public ref class IdentifierList : IReadOnlyList<Identifier^>
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly ProjObject^ m_object;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            array<Identifier^>^ m_Items;

        internal:
            IdentifierList(ProjObject^ obj)
            {
                if (!obj)
                    throw gcnew ArgumentNullException("obj");

                m_object = obj;
            }

        private:
            virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
            {
                    return GetEnumerator();
            }

        public:
            // Inherited via IReadOnlyCollection
            virtual System::Collections::Generic::IEnumerator<Identifier^>^ GetEnumerator();

            // Inherited via IReadOnlyList
            virtual property int Count
            {
                int get();
            }
            virtual property Identifier^ default[int]
            {
                    Identifier ^ get(int index);
            }
        };
    }
}