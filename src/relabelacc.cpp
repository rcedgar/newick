#include "myutils.h"
#include "treen.h"

void GetAccFromLabel(const string &Label, string &Acc);
void MakeAccMaps(const vector<string> &Labels,
  map<string, string> &LabelToAcc,
  map<string, string> &AccToLabel);

void cmd_relabelacc()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(relabelacc), Trees);
	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		TreeN &T = *Trees[TreeIndex];
		asserta(T.IsNormalized());
		const uint NodeCount = T.GetNodeCount();
		for (uint NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
			{
			if (!T.IsLeaf(NodeIndex))
				continue;
			const string OldLabel = T.GetLabel(NodeIndex);
			if (OldLabel == "")
				continue;
			string Acc;
			GetAccFromLabel(OldLabel, Acc);
			T.UpdateLabel(NodeIndex, Acc);
			}
		}

	TreesToFile(Trees, opt(output));
	}
