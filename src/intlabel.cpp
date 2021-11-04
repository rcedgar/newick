#include "myutils.h"
#include "tree2.h"

void cmd_intlabel()
	{
	asserta(optset_output);
	Tree2 T;
	T.FromNewickFile(opt(intlabel));

	const uint NodeCount = T.GetNodeCount();
	for (uint NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
		{
		if (T.IsLeaf(NodeIndex))
			{
			string Label;
			T.GetLabel(NodeIndex, Label);
			if (Label == "")
				{
				Warning("Empty leaf label");
				Ps(Label, "_leaf_%u", NodeIndex);
				T.m_Labels[NodeIndex] = Label;
				}
			continue;
			}

		string NewLabel;
		if (!opt(delete_labels))
			Ps(NewLabel, "_node_%u", NodeIndex);
		T.m_Labels[NodeIndex] = NewLabel;
		}

	T.ToNewickFile(opt(output));
	}
