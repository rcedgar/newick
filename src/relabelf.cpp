#include "myutils.h"
#include "treen.h"
#include "featuretable.h"

void cmd_relabelf()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(relabelf), Trees);

	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];
		asserta(T.IsNormalized());

		FeatureTable FT;
		FT.FromFile(opt(features));

		const uint NodeCount = T.GetNodeCount();
		for (uint Node = 0; Node < NodeCount; ++Node)
			{
			if (!T.IsLeaf(Node))
				continue;
			string Label = T.GetLabel(Node);
			if (opt(accs))
				GetAccFromLabel(Label, Label);
			if (Label == "")
				continue;
			string Value;
			FT.GetValue_ByLabel(Label, Value, false);
			string NewLabel = Label;
			if (Value != "")
				NewLabel += "." + Value;
			T.UpdateLabel(Node, NewLabel);
			}
		}
	TreesToFile(Trees, opt(output));
	}
