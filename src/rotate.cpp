#include "myutils.h"
#include "tree2.h"

void cmd_rotate()
	{
	asserta(optset_output);
	Tree2 T;
	T.FromNewickFile(opt(rotate));
	if (!optset_node)
		Die("-node required");
	const uint Node = opt(node);
	if (!T.IsRooted())
		Die("Rooted tree required");
	if (T.IsLeaf(Node))
		Die("Cannot rotate leaf node");

	T.Rotate(Node);
	T.ToNewickFile(opt(output));
	}
