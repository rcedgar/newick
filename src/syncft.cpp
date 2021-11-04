#include "myutils.h"
#include "tree2.h"
#include <set>

void cmd_syncft()
	{
	const string &FN = opt(syncft);
	FILE *fOut = CreateStdioFile(opt(output));
	if (fOut == 0)
		Die("Failed to create output file");

	Tree2 T;
	T.FromNewickFile(opt(tree));
	set<string> TreeLabels;

	const uint NodeCount = T.GetNodeCount();
	for (uint NodeIndex = 0; NodeIndex < NodeCount; ++NodeIndex)
		{
		if (!T.IsLeaf(NodeIndex))
			continue;
		const string Label = T.GetLabel(NodeIndex);
		if (TreeLabels.find(Label) != TreeLabels.end())
			Die("Dupe label >%s", Label.c_str());
		TreeLabels.insert(Label);
		}

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
		if (TreeLabels.find(Label) == TreeLabels.end())
			{
			++NotFound;
			ProgressLog("Deleted >%s\n", Label.c_str());
			continue;
			}
		++Found;
		FoundLabels.insert(Label);
		fprintf(fOut, "%s\n", Line.c_str());
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
