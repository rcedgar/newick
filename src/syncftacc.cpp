#include "myutils.h"
#include "tree2.h"
#include <set>

void MakeAccMaps(const vector<string> &Labels,
  map<string, string> &LabelToAcc,
  map<string, string> &AccToLabel)
	{
	LabelToAcc.clear();
	AccToLabel.clear();

	for (uint i = 0; i < SIZE(Labels); ++i)
		{
		const string &Label = Labels[i];
		string Acc;
		GetAccFromLabel(Label, Acc);
		if (LabelToAcc.find(Label) != LabelToAcc.end())
			Die("Dup label >%s", Label.c_str());
		if (AccToLabel.find(Acc) != AccToLabel.end())
			Die("Dup acc >%s", Acc.c_str());
		AccToLabel[Acc] = Label;
		LabelToAcc[Label] = Acc;
		}
	}

void cmd_syncftacc()
	{
	const string &FN = opt(syncftacc);
	FILE *fOut = CreateStdioFile(opt(output));
	if (fOut == 0)
		Die("Failed to create output file");

	Tree2 T;
	T.FromNewickFile(opt(tree));
	set<string> TreeLabels;
	vector<string> TreeLabelVec;

	const uint NodeCount = T.GetNodeCount();
	for (uint NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
		{
		if (!T.IsLeaf(NodeIndex))
			continue;
		const string Label = T.GetLabel(NodeIndex);
		if (TreeLabels.find(Label) != TreeLabels.end())
			Die("Dupe label >%s", Label.c_str());
		TreeLabels.insert(Label);
		TreeLabelVec.push_back(Label);
		}

	map<string, string> TreeAccToLabel;
	map<string, string> TreeLabelToAcc;
	MakeAccMaps(TreeLabelVec, TreeLabelToAcc, TreeAccToLabel);

	FILE *fIn = OpenStdioFile(FN);

	string Line;
	vector<string> Fields;
	set<string> FoundLabels;
	uint Found = 0;
	uint NotFound = 0;
	uint CatCount = UINT_MAX;
	string HdrLine;
	ReadLineStdioFile(fIn, HdrLine);
	fprintf(fOut, "%s\n", HdrLine.c_str());
	while (ReadLineStdioFile(fIn, Line))
		{
		Split(Line, Fields, '\t');
		uint FieldCount = SIZE(Fields);
		if (CatCount == UINT_MAX)
			CatCount = FieldCount - 1;
		else
			asserta(FieldCount == CatCount + 1);

		const string &Label = Fields[0];
		string Acc;
		GetAccFromLabel(Label, Acc);
		map<string, string>::const_iterator p = TreeAccToLabel.find(Acc);
		if (p == TreeAccToLabel.end())
			{
			++NotFound;
			ProgressLog("Deleted >%s\n", Label.c_str());
			continue;
			}
		++Found;
		const string &TreeLabel = p->second;
		FoundLabels.insert(TreeLabel);
		fprintf(fOut, "%s", TreeLabel.c_str());
		for (uint i = 1; i < FieldCount; ++i)
			fprintf(fOut, "\t%s", Fields[i].c_str());
		fprintf(fOut, "\n");
		}
	asserta(SIZE(FoundLabels) == Found);

	set<string> Missing;
	for (set<string>::const_iterator p = TreeLabels.begin();
	  p != TreeLabels.end(); ++p)
		{
		const string &TreeLabel = *p;
		if (FoundLabels.find(TreeLabel) != FoundLabels.end())
			continue;
		fprintf(fOut, "%s", TreeLabel.c_str());
		for (uint i = 0; i < CatCount; ++i)
			fprintf(fOut, "\t.");
		fprintf(fOut, "\n");
		ProgressLog("Inserted >%s\n", TreeLabel.c_str());
		}

	CloseStdioFile(fIn);
	}
