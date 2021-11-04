#include "myutils.h"
#include "tree2.h"
#include "layout.h"
#include "featuretable.h"
#include "svg.h"

void ColorsFromFile(const string &FileName,
  vector<string> &Values, vector<string> &Colors);
void StringsFromFile(const string &FileName, vector<string> &Strings);
void TreesFromFile2(const string &FileName, vector<Tree2 *> &Trees);

void cmd_drawfs()
	{
	const string &FileName = opt(drawfs);
	asserta(optset_colors);

	vector<Tree2 *> Tree2s;
	TreesFromFile2(FileName, Tree2s);
	const uint TreeCount = SIZE(Tree2s);
	asserta(TreeCount > 0);

	uint TreeWidth = TREE_WIDTH;
	uint TreeHeight = TREE_HEIGHT;
	uint TreeSpacing = TREE_SPACING;
	if (optset_tree_width)
		TreeWidth = opt(tree_width);
	if (optset_tree_height)
		TreeHeight = opt(tree_height);
	if (optset_tree_spacing)
		TreeSpacing = opt(tree_spacing);
	uint TitleFontSize = 10;
	if (optset_title_font_size)
		TitleFontSize = opt(title_font_size);
	string DefaultColor = "gray";
	if (optset_default_color)
		DefaultColor = opt(default_color);

	uint StrokeWidth = 1;
	if (optset_strokewidth)
		StrokeWidth = opt(strokewidth);

	uint TreesPerRow = 4;
	if (optset_trees_per_row)
		TreesPerRow = opt(trees_per_row);

	//vector<string> TreeFileNames;
	//StringsFromFile(FileName, TreeFileNames);
	//const uint TreeCount = SIZE(TreeFileNames);
	const uint RowCount = (TreeCount + TreesPerRow - 1)/TreesPerRow;

	double FigWidth = TreesPerRow*(TreeWidth + TreeSpacing) + 2*TreeSpacing; 
	double FigHeight = RowCount*(TreeHeight + TreeSpacing) + 2*TreeSpacing;
	double OffsetX = TreeSpacing;
	double OffsetY = TreeSpacing;

	FeatureTable FT;
	FT.FromFile(opt(features));

	vector<string> Values;
	vector<string> Colors;
	ColorsFromFile(opt(colors), Values, Colors);

	uint ValueCount = FT.GetValueCount();
	vector<string> ValueToColor(ValueCount, "black");
	const uint n = SIZE(Values);
	for (uint i = 0; i < n; ++i)
		{
		const string &Value = Values[i];
		uint ValueIndex = FT.GetValueIndex(Value);
		if (ValueIndex != UINT_MAX)
			{
			asserta(ValueIndex < ValueCount);
			ValueToColor[ValueIndex] = Colors[i];
			}
		}

	Svg S;
	asserta(optset_svg);
	S.Open(opt(svg), FigWidth, FigHeight);

	vector<string> Titles16x;
	if (opt(title16x))
		{
		asserta(TreeCount == 16);
		Titles16x.push_back("none.0");
		Titles16x.push_back("none.1");
		Titles16x.push_back("none.2");
		Titles16x.push_back("none.3");

		Titles16x.push_back("abc.0");
		Titles16x.push_back("abc.1");
		Titles16x.push_back("abc.2");
		Titles16x.push_back("abc.3");

		Titles16x.push_back("acb.0");
		Titles16x.push_back("acb.1");
		Titles16x.push_back("acb.2");
		Titles16x.push_back("acb.3");

		Titles16x.push_back("bca.0");
		Titles16x.push_back("bca.1");
		Titles16x.push_back("bca.2");
		Titles16x.push_back("bca.3");
		}

	uint Col = 0;
	for (uint TreeIndex = 0; TreeIndex < TreeCount; ++TreeIndex)
		{
		string Title;
		Ps(Title, "Tree%u", TreeIndex);

		Tree2 T = *Tree2s[TreeIndex];
		T.Ladderize(false);

		string RootLabel = T.GetLabel(T.m_Root);
		if (RootLabel != "")
			Title = RootLabel;

		if (opt(title16x))
			Title = Titles16x[TreeIndex];

		const uint NodeCount = T.GetNodeCount();
		if (!opt(unitlengths))
			{
			uint LengthCount = 0;
			for (map<pair<uint, uint>, double>::const_iterator p = T.m_EdgeToLength.begin();
			  p != T.m_EdgeToLength.end(); ++p)
				{
				if (p->second != MISSING_LENGTH)
					++LengthCount;
				}
			if (LengthCount < NodeCount/2)
				{
				Warning("Missing lengths");
				opt_unitlengths = true;
				optset_unitlengths = true;
				}
			}

		Layout Lay;
		Lay.m_OffsetX = OffsetX;
		Lay.m_OffsetY = OffsetY - 25;
		Lay.m_Width = TreeWidth;
		Lay.m_Height = TreeHeight;
		Lay.Run(T);
		Lay.SetFeatures(FT);
		Lay.SetColors(DefaultColor, ValueToColor);
		Lay.m_StrokeWidth = StrokeWidth;
		Lay.m_Title = Title;
		Lay.m_TitleFontSize = TitleFontSize;
		Lay.Render(S);
		if (TreeIndex == 0 && optset_legend)
			Lay.RenderLegend(opt(legend));

		++Col;
		if (Col == TreesPerRow)
			{
			Col = 0;
			OffsetY += TreeHeight + TreeSpacing;
			OffsetX = TreeSpacing;
			}
		else
			OffsetX += TreeWidth + TreeSpacing;
		}

// Draw grid
	for (uint i = 0; i <= RowCount; ++i)
		{
		uint y = TreeSpacing + i*(TreeSpacing + TreeHeight) - TreeSpacing/2;
		uint w = TreesPerRow*(TreeSpacing + TreeWidth);
		S.Line(TreeSpacing/2, y, TreeSpacing/2 + w, y, 3, "darkgray");
		}

	uint ytop = TreeSpacing/2;
	uint ybottom = TreeSpacing/2 + RowCount*(TreeSpacing + TreeHeight);
	for (uint i = 0; i <= TreesPerRow; ++i)
		{
		uint x = TreeSpacing/2 + i*(TreeSpacing + TreeWidth);
		S.Line(x, ytop, x, ybottom, 3, "darkgray");
		}

	S.Close();
	}
