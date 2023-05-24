#include "myutils.h"
#include "tree2.h"
#include "layout.h"
#include "featuretable.h"
#include "svg.h"

void ColorsFromFile(const string &FileName,
  vector<string> &Values, vector<string> &Colors);
void SetFeatureTable2(const Tree2 &T2, FeatureTable &FT);

void Layout::GetColor(uint Node, string &Color) const
	{
	Color = "black";
	if (m_FT == 0)
		return;

	uint ValueIndex = GetMajorityValueIndex(Node);
	if (ValueIndex == UINT_MAX)
		{
		Color = m_DefaultColor;
		return;
		}
	asserta(ValueIndex < SIZE(m_ValueToColor));
	Color = m_ValueToColor[ValueIndex];
	}

uint Layout::GetMajorityValueIndex(uint Node) const
	{
	asserta(Node < SIZE(m_NodeToValueToCount));
	const vector<uint> &ValueToCount = m_NodeToValueToCount[Node];
	uint MajorityValueIndex = UINT_MAX;
	uint TotalCount = 0;
	uint MaxCount = 0;
	uint BestIndex = 0;
	for (uint ValueIndex = 0; ValueIndex < m_ValueCount; ++ValueIndex)
		{
		uint Count = ValueToCount[ValueIndex];
		TotalCount += Count;
		if (Count > MaxCount)
			{
			BestIndex = ValueIndex;
			MaxCount = Count;
			}
		}
	if (TotalCount > 0)
		{
		asserta(MaxCount > 0);
		if (double(MaxCount)/double(TotalCount) >= m_MajorityFract)
			MajorityValueIndex = BestIndex;
		}
	return MajorityValueIndex;
	}

void Layout::SetValueCounts(uint Node)
	{
	asserta(Node < SIZE(m_NodeToValueToCount));
	asserta(m_NodeToValueToCount[Node].empty());
	m_NodeToValueToCount[Node].resize(m_ValueCount, 0);
	if (m_T->IsLeaf(Node))
		{
		uint ValueIndex = m_NodeToValueIndex[Node];
		if (ValueIndex != UINT_MAX)
			{
			asserta(ValueIndex < m_ValueCount);
			m_NodeToValueToCount[Node][ValueIndex] = 1;
			}
		return;
		}

	uint Left = m_T->GetLeft(Node);
	uint Right = m_T->GetRight(Node);
	asserta(Left != UINT_MAX && Right != UINT_MAX);
	SetValueCounts(Left);
	SetValueCounts(Right);

	asserta(Left < SIZE(m_NodeToValueToCount));
	asserta(Right < SIZE(m_NodeToValueToCount));
	const vector<uint> &LeftCounts = m_NodeToValueToCount[Left];
	const vector<uint> &RightCounts = m_NodeToValueToCount[Right];
	asserta(SIZE(LeftCounts) == m_ValueCount);
	asserta(SIZE(RightCounts) == m_ValueCount);

	for (uint ValueIndex = 0; ValueIndex < m_ValueCount; ++ValueIndex)
		{
		uint n = LeftCounts[ValueIndex] + RightCounts[ValueIndex];
		m_NodeToValueToCount[Node][ValueIndex] = n;
		}
	}

void Layout::SetFeatures(const FeatureTable &FT)
	{
	m_FT = &FT;
	m_ValueCount = m_FT->GetValueCount();

	uint NodeCount = m_T->GetNodeCount();
	uint LeafCount = m_T->GetNodeCount();
	m_NodeToValueIndex.clear();
	m_NodeToValueIndex.resize(NodeCount, UINT_MAX);
	uint LabelNotFoundCount = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		if (!m_T->IsLeaf(Node))
			continue;

		string Label = m_T->GetLabel(Node);
		if (opt(accs) && m_T->IsLeaf(Node))
			GetAccFromLabel(Label, Label);
		uint LabelIndex = m_FT->GetLabelIndex(Label);
		if (LabelIndex == UINT_MAX)
			{
			Log("Not found >%s\n", Label.c_str());
			++LabelNotFoundCount;
			continue;
			}
		uint ValueIndex = m_FT->GetValueIndex_ByLabelIndex(LabelIndex);
		m_NodeToValueIndex[Node] = ValueIndex;
		}
	if (LabelNotFoundCount > 0)
		ProgressLog("%u / %u labels not found\n", LabelNotFoundCount, LeafCount);
	m_NodeToValueToCount.resize(NodeCount);
	SetValueCounts(m_T->m_Root);
	}

void Layout::SetColors(const string &DefaultColor,
  const vector<string> &ValueToColor)
	{
	asserta(SIZE(ValueToColor) == m_ValueCount);
	m_DefaultColor = DefaultColor;
	m_ValueToColor = ValueToColor;
	}

void Layout::RenderLegend(const string &SvgFileName) const
	{
	Svg S;
	S.Open(SvgFileName, 500, 500);
	const uint ValueCount = m_FT->GetValueCount();
	asserta(SIZE(m_ValueToColor) == ValueCount);
	double Y = 50;
	double X = 50;
	for (uint ValueIndex = 0; ValueIndex < ValueCount; ++ValueIndex)
		{
		const string &Value = m_FT->GetValue(ValueIndex);
		const string &Color = m_ValueToColor[ValueIndex];
		if (Color == m_DefaultColor)
			continue;

		S.Rect(50, Y, 25, 8, 1, Color, Color);
		S.Text(80, Y+8, "Arial", 10, "normal", "black", "start", Value);

		Y += 20;
		}

	S.Close();
	}

void cmd_drawf()
	{
	const string &FileName = opt(drawf);
	asserta(optset_colors);
	double OffsetX = 0;
	double OffsetY = 0;
	if (optset_offsetx)
		OffsetX = opt(offsetx);
	if (optset_offsety)
		OffsetY = opt(offsety);
	string Title;
	if (optset_title)
		Title = opt(title);
	uint TitleFontSize = 10;
	if (optset_title_font_size)
		TitleFontSize = opt(title_font_size);

	Tree2 T;
	T.FromNewickFile(FileName);
	T.Ladderize(false);

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

	//uint StrokeWidth = 1;
	//if (optset_strokewidth)
	//	StrokeWidth = opt(strokewidth);

	FeatureTable FT;
	if (optset_features)
		FT.FromFile(opt(features));
	else
		SetFeatureTable2(T, FT);

	string DefaultColor = "gray";
	if (optset_default_color)
		DefaultColor = opt(default_color);
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

	Layout Lay;
	Lay.m_OffsetX = OffsetX;
	Lay.m_OffsetY = OffsetY;
	Lay.m_Title = Title;
	Lay.m_TitleFontSize = TitleFontSize;
	Lay.Run(T);
	Lay.SetFeatures(FT);
	Lay.SetColors(DefaultColor, ValueToColor);
	//Lay.m_StrokeWidth = StrokeWidth;
	Lay.Render(opt(svg));
	if (optset_legend)
		Lay.RenderLegend(opt(legend));
	}
