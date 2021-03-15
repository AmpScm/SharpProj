#pragma once
namespace SharpProj {
	ref class CoordinateTransform;
	namespace Proj {
		using System::Collections::Generic::IEnumerable;
		using System::Collections::Generic::IReadOnlyList;
		using System::Diagnostics::DebuggerBrowsableAttribute;
		using System::Diagnostics::DebuggerBrowsableState;
		ref class ProjStep;

		[System::Diagnostics::DebuggerDisplayAttribute("Count = {Count}")]
		public ref class ProjStepList : IReadOnlyList<ProjStep^>
		{
		private:
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			initonly CoordinateTransform^ m_transform;
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			array<ProjStep^>^ m_items;
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			IReadOnlyList<String^>^ m_tokens;
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			int m_iCommonStart;
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			int m_iCommonCount;
		internal:
			ProjStepList(CoordinateTransform^ transform);
			void Ensure();

			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			property IReadOnlyList<String^>^ Tokens
			{
				IReadOnlyList<String^>^ get()
				{
					return m_tokens;
				}
			}

			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			property IEnumerable<String^>^ CommonTokens
			{
				IEnumerable<String^>^ get()
				{
					return ArrayRange(Tokens, m_iCommonStart, m_iCommonCount);
				}
			}

		private:
			virtual System::Collections::IEnumerator^ Obj_GetEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
			{
				return GetEnumerator();
			}

		public:
			virtual System::Collections::Generic::IEnumerator<SharpProj::Proj::ProjStep^>^ GetEnumerator()
			{
				Ensure();
				return static_cast<IEnumerable<ProjStep^>^>(m_items)->GetEnumerator();
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
			virtual property SharpProj::Proj::ProjStep^ default[int]
			{
				ProjStep ^ get(int index)
				{
					Ensure();

					return m_items[index];
				}
			}

		private:
			generic<typename T> static IEnumerable<T>^ ArrayRange(array<T>^ array, int offset, int count)
			{
				if (count == 0)
					return System::Linq::Enumerable::Empty<T>();

				IEnumerable<T>^ arrayEnum = static_cast<IEnumerable<T>^>(array);
				return System::Linq::Enumerable::Take(System::Linq::Enumerable::Skip(arrayEnum, offset), count);
			}

			generic<typename T> static IEnumerable<T>^ ArrayRange(IReadOnlyList<T>^ array, int offset, int count)
			{
				if (count == 0)
					return System::Linq::Enumerable::Empty<T>();

				IEnumerable<T>^ arrayEnum = static_cast<IEnumerable<T>^>(array);
				return System::Linq::Enumerable::Take(System::Linq::Enumerable::Skip(arrayEnum, offset), count);
			}
		};

		[System::Diagnostics::DebuggerDisplayAttribute("[{Name,nq}] {ToString(),nq}")]
		public ref class ProjStep
		{
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			initonly ProjStepList^ m_list;
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			initonly int m_offset;
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			initonly int m_count;
			[DebuggerBrowsableAttribute(DebuggerBrowsableState::Never)]
			String^ m_name;

		internal:
			ProjStep(ProjStepList^ list, int offset, int count)
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
					{
						auto tokens = m_list->Tokens;
						int o = m_offset;

						while (o < tokens->Count && tokens[o] == "inv")
							o++;

						if (o < tokens->Count)						
							m_name = m_list->Tokens[o]->Substring(5);
					}
					return m_name;
				}
			}

		public:
			property IEnumerable<String^>^ Tokens
			{
				IEnumerable<String^>^ get()
				{
					return ArrayRange(m_list->Tokens, m_offset, m_count);
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
					return m_list->Tokens[m_offset] == "inv";
				}
			}

			property bool SkipForward
			{
				bool get()
				{
					return System::Linq::Enumerable::Contains(Tokens, "omit_fwd");
				}
			}

			property bool SkipInverse
			{
				bool get()
				{
					return System::Linq::Enumerable::Contains(Tokens, "omit_inv");
				}
			}

		private:
			generic<typename T> static IEnumerable<T>^ ArrayRange(array<T>^ array, int offset, int count)
			{
				if (count == 0)
					return System::Linq::Enumerable::Empty<T>();

				IEnumerable<T>^ arrayEnum = static_cast<IEnumerable<T>^>(array);
				return System::Linq::Enumerable::Take(System::Linq::Enumerable::Skip(arrayEnum, offset), count);
			}

			generic<typename T> static IEnumerable<T>^ ArrayRange(IReadOnlyList<T>^ array, int offset, int count)
			{
				if (count == 0)
					return System::Linq::Enumerable::Empty<T>();

				IEnumerable<T>^ arrayEnum = static_cast<IEnumerable<T>^>(array);
				return System::Linq::Enumerable::Take(System::Linq::Enumerable::Skip(arrayEnum, offset), count);
			}

		public:
			virtual String^ ToString() override
			{
				return String::Join(" ", Tokens);
			}
		};
	}
}