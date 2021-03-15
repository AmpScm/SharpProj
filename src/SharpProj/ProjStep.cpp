#include "pch.h"
#include "ProjStep.h"
#include "CoordinateTransform.h"

IReadOnlyList<ProjStep^>^ CoordinateTransform::ProjSteps()
{
	return gcnew ProjStepList(this);
}

ProjStepList::ProjStepList(CoordinateTransform^ transform)
{
	if (!transform)
		throw gcnew ArgumentNullException("transform");

	m_transform = transform;
}

void ProjStepList::Ensure()
{
	if (m_items)
		return;

	PJ_PROJ_INFO info = proj_pj_info(m_transform);

	const char* def = info.definition;
	List<String^>^ tokens = gcnew List<String^>();
	
	while (*def)
	{
		while (*def && Char::IsWhiteSpace(*def))
			def++;

		if (!*def)
			break;

		const char* start = def;
		bool in_str = false;

		while (*def && (in_str || !Char::IsWhiteSpace(*def)))
		{
			if (*def == '\"')
				in_str = !in_str;

			*def++;
		}

		tokens->Add(Utf8_PtrToString(start, def - start)->Replace("\"\"", "\""));
	}

	auto tokenArray = tokens->ToArray();
	m_tokens = Array::AsReadOnly(tokenArray);

	if (tokenArray->Length && tokenArray[0] == "proj=pipeline")
	{
		int iStep = Array::IndexOf(tokenArray, "step");
		m_iCommonStart = 1;
		m_iCommonCount = iStep - 1;

		List<ProjStep^>^ steps = gcnew List<ProjStep^>();
		int iNext;
		do
		{
			iNext = Array::IndexOf(tokenArray, "step", iStep + 1);

			if (iNext < 0)
				iNext = tokenArray->Length;

			steps->Add(gcnew ProjStep(this, iStep + 1, iNext - iStep - 1));
			iStep = iNext;
		} 		
		while (iNext < tokenArray->Length);

		m_items = steps->ToArray();
	}
	else
	{
		m_items = gcnew array<ProjStep^> { gcnew ProjStep(this, 0, m_items->Length)};
	}
}

