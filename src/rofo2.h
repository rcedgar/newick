#pragma once

#include "biffer2.h"

class RoFo
	{
public:
// Input
	const Biffer2 *m_Biffer1 = 0;
	const Biffer2 *m_Biffer2 = 0;

// Result
	vector<bool> m_SameVec;
	uint m_BifCount = 0;
	uint m_SameCount = 0;

	void Calc(const Biffer2 &Biffer1, const Biffer2 &Biffer2);
	};
