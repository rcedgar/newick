#include "myutils.h"
#include "tree2.h"

void cmd_relabeln()
	{
	Tree2 T;
	T.FromNewickFile(opt(relabeln));

	string &Prefix = opt(label);
	if (Prefix == "")
		Prefix = "Lab";
	const uint NodeCount = T.GetNodeCount();
	for (uint NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
		{
		if (!T.IsLeaf(NodeIndex))
			{
			T.m_Labels[NodeIndex] = "";
			continue;
			}
		string NewLabel;
		Ps(NewLabel, "%s%u", Prefix.c_str(), NodeIndex);
		T.m_Labels[NodeIndex] = NewLabel;
		}

	T.ToNewickFile(opt(output));
	}
