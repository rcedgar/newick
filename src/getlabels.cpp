#include "myutils.h"
#include "tree2.h"

void cmd_getlabels()
	{
	asserta(optset_output);
	Tree2 T;
	T.FromNewickFile(opt(getlabels));
	FILE *f = CreateStdioFile(opt(output));
	vector<string> Labels;
	for (uint Node = 0; Node < T.GetNodeCount(); ++Node)
		{
		if (T.IsLeaf(Node))
			{
			string Label;
			T.GetLabel(Node, Label);
			Labels.push_back(Label);
			}
		}
	sort(Labels.begin(), Labels.end());
	for (uint i = 0; i < SIZE(Labels); ++i)
		{
		const char *Label = Labels[i].c_str();
		fprintf(f, "%s\n", Label);
		}
	CloseStdioFile(f);
	}
