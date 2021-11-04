#include "myutils.h"
#include "tree2.h"
#include "layout.h"
#include "svg.h"

void cmd_draw()
	{
	const string &FileName = opt(draw);
	double OffsetX = 0;
	double OffsetY = 0;
	if (optset_offsetx)
		OffsetX = opt(offsetx);
	if (optset_offsety)
		OffsetY = opt(offsety);

	Tree2 T;
	T.FromNewickFile(FileName);

	const uint NodeCount = T.GetNodeCount();
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!T.IsRoot(Node) && T.GetEdgeLengthToParent(Node) == MISSING_LENGTH)
			Die("Missing lengths");
		}

	Layout Lay;
	Lay.m_OffsetX = OffsetX;
	Lay.m_OffsetY = OffsetY;
	if (optset_tree_width)
		Lay.m_Width = opt(tree_width);
	if (optset_tree_height)
		Lay.m_Height = opt(tree_height);
	Lay.Run(T);
	Lay.Render(opt(svg));
	}
