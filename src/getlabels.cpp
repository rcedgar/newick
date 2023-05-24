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

static FILE *g_fOut;

static void OnNode(const Tree2 &T, uint Node)
	{
	if (T.IsLeaf(Node))
		{
		string Label;
		T.GetLabel(Node, Label);
		Pf(g_fOut, "%s\n", Label.c_str());
		}
	}

void cmd_getlabels_treeorder()
	{
	asserta(optset_output);
	Tree2 T;
	T.FromNewickFile(opt(getlabels_treeorder));
	asserta(T.IsRooted());
	uint Root = T.GetRoot();

	string Order = "in"; // in pre post
	if (optset_order)
		Order = opt(order);

	g_fOut = CreateStdioFile(opt(output));
	if (Order == "in")
		T.Inorder(Root, OnNode);
	else if (Order == "pre")
		T.Preorder(Root, OnNode);
	else if (Order == "post")
		T.Postorder(Root, OnNode);
	else
		asserta(false);

	CloseStdioFile(g_fOut);
	}
