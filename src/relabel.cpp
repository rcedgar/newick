#include "myutils.h"
#include "treeN.h"

void GetAccFromLabel(const string &Label, string &Acc);

static void Relabel1(TreeN &T, const map<string, string> &OldLabelToNewLabel,
  set<string> &FoundLabels)
	{
	uint NotFound = 0;
	uint Replaced = 0;
	asserta(T.IsNormalized());
	const uint NodeCount = T.GetNodeCount();
	for (uint NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
		{
		if (!T.IsLeaf(NodeIndex))
			continue;
		string OldLabel = T.GetLabel(NodeIndex);
		if (opt(accs))
			GetAccFromLabel(OldLabel, OldLabel);
		map<string, string>::const_iterator p =
		  OldLabelToNewLabel.find(OldLabel);
		if (p == OldLabelToNewLabel.end())
			{
			++NotFound;
			if (NotFound < 10)
				ProgressLog("Not found >%s\n", OldLabel.c_str());
			else if (NotFound == 10)
				ProgressLog("10+ Not found\n");
			continue;
			}
		FoundLabels.insert(OldLabel);
		const string &NewLabel = p->second;
		T.UpdateLabel(NodeIndex, NewLabel);
		++Replaced;
		}
	}

void cmd_relabel()
	{
	vector<TreeN *> Trees;
	TreesFromFile(opt(relabel), Trees);

	FILE *f = OpenStdioFile(opt(labels2));
	string Line;
	vector<string> Fields;
	map<string, string> OldLabelToNewLabel;
	uint LabelCount = 0;
	while (ReadLineStdioFile(f, Line))
		{
		Split(Line, Fields, '\t');
		if (SIZE(Fields) != 2)
			Die("Expected 2 fields in line '%s'", Line.c_str());
		string OldLabel = Fields[0];
		if (opt(accs))
			GetAccFromLabel(OldLabel, OldLabel);
		const string &NewLabel = Fields[1];
		if (OldLabelToNewLabel.find(OldLabel) != OldLabelToNewLabel.end())
			Die("Dupe label >%s", OldLabel.c_str());
		OldLabelToNewLabel[OldLabel] = NewLabel;
		++LabelCount;
		}
	CloseStdioFile(f);

	set<string> FoundLabels;
	const uint TreeCount = SIZE(Trees);
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		ProgressStep(TreeIndex, TreeCount, "Processing");
		TreeN *T = Trees[TreeIndex];
		Relabel1(*T, OldLabelToNewLabel, FoundLabels);
		}
	TreesToFile(Trees, opt(output));

	ProgressLog("%u / %u labels found\n", SIZE(FoundLabels), LabelCount);
	}
