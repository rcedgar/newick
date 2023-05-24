#include "myutils.h"
#include "tree2.h"
#include "layout.h"
#include "featuretable.h"
#include "svg.h"

void ColorsFromFile(const string &FileName,
  vector<string> &Values, vector<string> &Colors);
void TreesFromFile3(const string &FileName, vector<Tree2 *> &Trees);
void TreesFromFile2(const string &FileName, vector<Tree2 *> &Trees);
void StringsFromFile(const string &FileName, vector<string> &Strings);
void SetFeatureTable2(const Tree2 &T2, FeatureTable &FT);

static double HUE = 0.5;
static double SAT = 0.95;
static double h = 0;
static double Hue = 0;

static void hsv_to_rgb(double h, double s, double v,
  byte &R, byte &G, byte &B)
	{
	int h_i = int(h*6);
	double f = h*6 - h_i;
	double p = v * (1 - s);
	double q = v * (1 - f*s);
	double t = v * (1 - (1 - f) * s);
	double r = 0;
	double g = 0;
	double b = 0;
	if (h_i == 0)
		{
		//r, g, b = v, t, p
		r = v;
		v = t;
		b = p;
		}
	else if ( h_i == 1)	
		{
		//r, g, b = q, v, p
		r = q;
		g = v;
		b = p;
		}
	else if ( h_i == 2)	
		{
		//r, g, b = p, v, t
		r = p;
		g = v;
		b = t;
		}
	else if ( h_i == 3)	
		{
		//r, g, b = p, q, v
		r = p;
		g = q;
		b = v;
		}
	else if ( h_i == 4)	
		{
		//r, g, b = t, p, v
		r = t;
		g = p;
		b = v;
		}
	else if ( h_i == 5)	
		{
		//r, g, b = v, p, q
		r = v;
		g = p;
		b = q;
		}
	else
		asserta(false);

	R = byte(r*256);
	G = byte(g*256);
	B = byte(b*256);
	}

void GetRandomColor(string &Color)
	{
	h += 0.618033988749895;
	h = fmod(h, 1.0);
	if (HUE == 0)
		{
		uint r = randu32()%1000;
		Hue = 0.5 + r/2000.0;
		}
	else
		Hue = HUE;
	byte R, G, B;
	hsv_to_rgb(h, Hue, SAT, R, G, B);
	Ps(Color, "#%02x%02x%02x", R, G, B);
	}

static void DrawTrees(const vector<Tree2 *> &Tree2s)
	{
	asserta(optset_colors);

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
	string DefaultColor = "lightgray";
	if (optset_default_color)
		DefaultColor = opt(default_color);

	uint StrokeWidth = 1;
	if (optset_strokewidth)
		StrokeWidth = opt(strokewidth);

	uint TreesPerRow = 4;
	if (optset_trees_per_row)
		TreesPerRow = opt(trees_per_row);

	const uint RowCount = (TreeCount + TreesPerRow - 1)/TreesPerRow;

	double FigWidth = TreesPerRow*(TreeWidth + TreeSpacing) + 2*TreeSpacing; 
	double FigHeight = RowCount*(TreeHeight + TreeSpacing) + 2*TreeSpacing;
	double OffsetX = TreeSpacing;
	double OffsetY = TreeSpacing;

	FeatureTable FT;
	if (optset_features)
		FT.FromFile(opt(features));

	vector<string> Values;
	vector<string> Colors;
	if (optset_colors)
		ColorsFromFile(opt(colors), Values, Colors);
	else
		{
		Values = FT.m_Values;
		const uint ValueCount = SIZE(Values);
		for (uint i = 0; i < ValueCount; ++i)
			{
			string Color;
			GetRandomColor(Color);
			Colors.push_back(Color);
			}
		}

	vector<string> Titles;
	if (optset_titles)
		{
		StringsFromFile(opt(titles), Titles);
		asserta(SIZE(Titles) == TreeCount);
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
		else if (optset_titles)
			Title = Titles[TreeIndex];

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

		if (!optset_features)
			SetFeatureTable2(T, FT);
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

		Layout Lay;
		Lay.m_OffsetX = OffsetX;
		Lay.m_OffsetY = OffsetY - 25;
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

void cmd_drawfs()
	{
// FileName is Newick file with one tree per line
	const string &FileName = opt(drawfs);
	//asserta(optset_colors);

	vector<Tree2 *> Tree2s;
	TreesFromFile2(FileName, Tree2s);
	DrawTrees(Tree2s);
	}

void cmd_drawfsp()
	{
// FileName is text file with one pathname per line
	const string &FileName = opt(drawfsp);
	//asserta(optset_colors);

	vector<Tree2 *> Tree2s;
	TreesFromFile3(FileName, Tree2s);
	DrawTrees(Tree2s);
	}
