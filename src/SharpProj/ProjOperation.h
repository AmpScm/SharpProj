#pragma once
namespace SharpProj {
    ref class CoordinateTransform;
    namespace Proj {
        using System::Collections::Generic::IEnumerable;
        using System::Collections::Generic::IEnumerator;
        using System::Collections::Generic::IReadOnlyList;
        using System::Collections::Generic::IReadOnlyDictionary;
        using System::Collections::Generic::Dictionary;
        using System::Collections::Generic::KeyValuePair;
        using System::Collections::Generic::List;
        using System::Collections::ObjectModel::ReadOnlyCollection;

        ref class ProjOperation;

        [DebuggerDisplay("Count = {Count}")]
        public ref class ProjOperationList : IReadOnlyList<ProjOperation^>
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly CoordinateTransform^ m_transform;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            array<ProjOperation^>^ m_items;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            IReadOnlyList<String^>^ m_tokens;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            int m_iCommonStart;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            int m_iCommonCount;
        internal:
            ProjOperationList(CoordinateTransform^ transform);
            void Ensure();

            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            property IReadOnlyList<String^>^ Tokens
            {
                IReadOnlyList<String^>^ get()
                {
                    return m_tokens;
                }
            }

            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            property IEnumerable<String^>^ CommonTokens
            {
                IEnumerable<String^>^ get()
                {
                    return ListRange(Tokens, m_iCommonStart, m_iCommonCount);
                }
            }

        private:
            virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
            {
                    return GetEnumerator();
            }

        public:
            virtual System::Collections::Generic::IEnumerator<SharpProj::Proj::ProjOperation^>^ GetEnumerator()
            {
                Ensure();
                return static_cast<IEnumerable<ProjOperation^>^>(m_items)->GetEnumerator();
            }

            // Inherited via IReadOnlyList
            virtual property int Count
            {
                int get()
                {
                    Ensure();
                    return m_items->Length;
                }
            }
            virtual property SharpProj::Proj::ProjOperation^ default[int]
            {
                    ProjOperation ^ get(int index)
                    {
                            Ensure();

                            return m_items[index];
                    }
            }

        private:
            generic<typename T> static IEnumerable<T>^ ListRange(IReadOnlyList<T>^ list, int offset, int count)
            {
                if (count <= 0)
                    return System::Linq::Enumerable::Empty<T>();

                return System::Linq::Enumerable::Take(System::Linq::Enumerable::Skip(list, offset), count);
            }
        };

        public enum class ProjOperationType
        {
            Unknown,
            NoOperation,
            Projection,
            Transformation,
            Pipeline
        };

        [DebuggerDisplay("[{Name,nq}] {ToString(),nq}")]
        public ref class ProjOperation
        {
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly ProjOperationList^ m_list;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly int m_offset;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly int m_count;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            String^ m_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            ProjOperationType m_type;

        internal:
            ProjOperation(ProjOperationList^ list, int offset, int count)
            {
                m_list = list;
                m_offset = offset;
                m_count = count;
                m_type = (ProjOperationType)-1;
            }

        public:
            property String^ Name
            {
                String^ get()
                {
                    if (!m_name)
                        m_name = this["proj"];

                    return m_name;
                }
            }

        public:
            property IEnumerable<String^>^ Tokens
            {
                IEnumerable<String^>^ get()
                {
                    return ListRange(m_list->Tokens, m_offset, m_count);
                }
            }

            property IEnumerable<String^>^ AllTokens
            {
                IEnumerable<String^>^ get()
                {
                    return System::Linq::Enumerable::Concat(Tokens, m_list->CommonTokens);
                }
            }

            property bool IsInverse
            {
                bool get()
                {
                    return System::Linq::Enumerable::Contains(Tokens, (String^)"inv");
                }
            }

            property bool IsInverseOnly
            {
                bool get()
                {
                    return System::Linq::Enumerable::Contains(Tokens, (String^)"omit_fwd");
                }
            }

            property bool IsForwardOnly
            {
                bool get()
                {
                    return System::Linq::Enumerable::Contains(Tokens, (String^)"omit_fwd");
                }
            }

            property ProjOperationType Type
            {
                ProjOperationType get();
            }

        private:
            generic<typename T> static IEnumerable<T>^ ListRange(IReadOnlyList<T>^ list, int offset, int count)
            {
                if (count <= 0)
                    return System::Linq::Enumerable::Empty<T>();

                return System::Linq::Enumerable::Take(System::Linq::Enumerable::Skip(list, offset), count);
            }

        public:
            virtual String^ ToString() override
            {
                return String::Join(" ", Tokens);
            }

        public:
            property String^ default[String^]
            {
                String ^ get(String ^ key)
                {
                     key += "=";
                     for each (String ^ s in AllTokens) // Look in specific tokens + common tokens
                     {
                             if (s->StartsWith(key))
                                     return s->Substring(key->Length);
                     }
                     return nullptr;
                }
            }
        };

        extern "C"
        {
            typedef PJ* (*sproj_init_func)(PJ*);
        };

        [DebuggerDisplayAttribute("[{Name,nq}] {Title,nq} ({Type})")]
        public ref class ProjOperationDefinition
        {
        private:
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ m_name;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ m_title;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly String^ m_details;
            initonly ProjOperationType m_type;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            initonly sproj_init_func m_constructor;
            [DebuggerBrowsable(DebuggerBrowsableState::Never)]
            IReadOnlyList<String^>^ m_rqArgs;

        internal:
            ProjOperationDefinition(ProjOperationType type, String^ name, String^ title, String^ details, sproj_init_func constructor)
            {
                m_type = type;
                m_name = name;
                m_title = title;
                m_details = details;
                m_constructor = constructor;
            }

        public:
            property String^ Name
            {
                String^ get()
                {
                    return m_name;
                }
            }

            property String^ Title
            {
                String^ get()
                {
                    return m_title;
                }
            }

            property ProjOperationType Type
            {
                ProjOperationType get()
                {
                    return m_type;
                }
            }

            property String^ Details
            {
                String^ get()
                {
                    return m_details;
                }
            }

        public:
            property IReadOnlyList<String^>^ RequiredArguments
            {
                IReadOnlyList<String^>^ get();
            }

        public:
            ref class ProjOperationDefinitionCollection sealed : ReadOnlyCollection<ProjOperationDefinition^>
            {
            private:
                initonly Dictionary<String^, ProjOperationDefinition^>^ _dict;
                static String^ Get_Name(ProjOperationDefinition^ item)
                {
                    return item->Name;
                }

            internal:
                ProjOperationDefinitionCollection(List<ProjOperationDefinition^>^ list)
                    : ReadOnlyCollection< ProjOperationDefinition^>(list)
                {
                    _dict = System::Linq::Enumerable::ToDictionary(list,
                        gcnew System::Func<ProjOperationDefinition^, String^>(&ProjOperationDefinitionCollection::Get_Name),
                        System::StringComparer::OrdinalIgnoreCase);
                }

            public:
                property ProjOperationDefinition^ default[String^]
                {
                        virtual ProjOperationDefinition ^ get(String ^ name)
                        {
                                return _dict[name];
                        }
                }

            public:
                virtual bool TryGetValue(String^ name, [Out] ProjOperationDefinition^% value)
                {
                    return _dict->TryGetValue(name, value);
                }

                // Inherited via IReadOnlyDictionary
                property System::Collections::Generic::IEnumerable<System::String^>^ Keys
                {
                    virtual System::Collections::Generic::IEnumerable<System::String^>^ get()
                    {
                        return _dict->Keys;
                    }
                }

                virtual bool ContainsKey(System::String^ key)
                {
                    return _dict->ContainsKey(key);
                }
            };

        private:
            static ProjOperationDefinitionCollection^ LoadFuncs();

            static initonly Lazy<ProjOperationDefinitionCollection^>^ _fetchOperations
                = gcnew Lazy<ProjOperationDefinitionCollection^>(gcnew Func<ProjOperationDefinitionCollection^>(&LoadFuncs));

        public:
            static property ProjOperationDefinitionCollection^ All
            {
                ProjOperationDefinitionCollection^ get()
                {
                    return _fetchOperations->Value;
                }
            }
        };
    }
}