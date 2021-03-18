#pragma once
namespace SharpProj {
	ref class CoordinateTransform;
	namespace Proj {
		using System::Collections::Generic::IEnumerable;
		using System::Collections::Generic::IReadOnlyList;
		using System::Diagnostics::DebuggerBrowsableAttribute;
		using System::Diagnostics::DebuggerBrowsableState;
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

		internal:
			ProjOperation(ProjOperationList^ list, int offset, int count)
			{
				m_list = list;
				m_offset = offset;
				m_count = count;
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
	}
}