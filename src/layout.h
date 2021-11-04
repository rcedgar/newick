#pragma once

class Svg;
class FeatureTable;

const uint TREE_WIDTH = 1000;
const uint TREE_HEIGHT = 1000;
const uint TREE_SPACING = 300;

class Layout
	{
public:
	const Tree2 *m_T = 0;
	double m_LeafSpacingY = 10;	// spacing between leaves
	double m_ScaleX = 10;		// scaling factor for branch length

	vector<double> m_Xs;
	vector<double> m_Ys;
	vector<double> m_Midxs;
	vector<double> m_Rxs;
	vector<double> m_Rys;
	vector<double> m_Lxs;
	vector<double> m_Lys;

	double m_CurrentLeafY = 0;
	double m_MaxRootDist = 0;
	double m_Margin = 50;
	double m_Width = TREE_WIDTH;
	double m_Height = TREE_HEIGHT;
	double m_StrokeWidth = 1;
	double m_OffsetX = 0;
	double m_OffsetY = 0;

	double m_TriangleWidth = 0;
	double m_TriangleHeight = 0;

	const FeatureTable *m_FT = 0;
	uint m_ValueCount = 0;
	vector<uint> m_NodeToValueIndex;
	vector<vector<uint> > m_NodeToValueToCount;
	string m_DefaultColor = "gray";
	string m_LeafLabelColor = "black";
	vector<string> m_ValueToColor;
	double m_MajorityFract = 0.8;

	string m_Title = "";
	double m_TitleFontSize = 10;
	double m_LabelFontSize = 0;

	double m_MaxY = 0;

public:
	void Run(const Tree2 &T);
	void SetFeatures(const FeatureTable &FT);
	void RenderNode(Svg &S, uint Node);
	void Render(Svg &S);
	void Render(const string &SvgFileName);
	void RenderLegend(const string &SvgFileName) const;
	uint GetMajorityValueIndex(uint Node) const;
	void SetColors(const string &DefaultColor,
	  const vector<string> &ValueToColor);
	void GetColor(uint Node, string &Color) const;

private:
	double SetY(uint Node);
	void SetValueCounts(uint Node);
	};
