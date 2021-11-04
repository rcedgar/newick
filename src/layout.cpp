#include "myutils.h"
#include "tree2.h"
#include "layout.h"
#include "svg.h"

double Layout::SetY(uint Node)
	{
	asserta(Node != UINT_MAX);
	asserta(Node < SIZE(m_Ys));
	if (m_T->IsLeaf(Node))
		{
		double Y = m_CurrentLeafY;
		m_Ys[Node] = Y;
		m_CurrentLeafY += m_LeafSpacingY;
		return Y;
		}

	uint Left = m_T->GetLeft(Node);
	uint Right = m_T->GetRight(Node);
	double LeftY = SetY(Left);
	double RightY = SetY(Right);
	double Mid = (LeftY + RightY)/2;
	m_Ys[Node] = Mid;
	return Mid;
	}

void Layout::Run(const Tree2 &T)
	{
	m_T = &T;
	asserta(m_T->IsRooted());
	const uint NodeCount = m_T->GetNodeCount();
	const uint LeafCount = m_T->GetLeafCount();
	m_LeafSpacingY = m_Height/LeafCount;
	m_LabelFontSize = (m_LeafSpacingY*3)/4;
	if (m_LabelFontSize > 10)
		m_LabelFontSize = 10;
	if (optset_label_font_size)
		m_LabelFontSize = opt(label_font_size);

	if (optset_triangles)
		{
		vector<string> Fields;
		Split(opt(triangles), Fields, '+');
		asserta(SIZE(Fields) == 2);
		m_TriangleWidth = StrToUint(Fields[0]);
		m_TriangleHeight = StrToUint(Fields[1]);
		}
	else
		{
		m_TriangleHeight = 0;
		m_TriangleWidth = 0;
		}

	m_Xs.clear();
	m_Ys.clear();
	m_Midxs.clear();
	m_Rxs.clear();
	m_Rys.clear();

	m_Xs.resize(NodeCount, DBL_MAX);
	m_Ys.resize(NodeCount, DBL_MAX);
	m_Midxs.resize(NodeCount, DBL_MAX);
	m_Lxs.resize(NodeCount, DBL_MAX);
	m_Lys.resize(NodeCount, DBL_MAX);
	m_Rxs.resize(NodeCount, DBL_MAX);
	m_Rys.resize(NodeCount, DBL_MAX);

	double RootY = SetY(m_T->m_Root);

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double Dist = (opt(unitlengths) ?
		  m_T->GetNodeCountToRoot(Node) : m_T->GetRootDist(Node));
		m_MaxRootDist = max(Dist, m_MaxRootDist);
		}
	m_ScaleX = m_Width/m_MaxRootDist;

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double Dist = (opt(unitlengths) ?
		  m_T->GetNodeCountToRoot(Node) : m_T->GetRootDist(Node));
		double X = Dist*m_ScaleX;
		m_Xs[Node] = X;
		}

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		m_Xs[Node] += m_OffsetX;
		m_Ys[Node] += m_OffsetY;
		}

	for (uint Node = 0; Node < NodeCount; ++Node)
		{
		double X = m_Xs[Node];
		double Y = m_Ys[Node];
		const string &Label = m_T->GetLabel(Node);

		double Lx = DBL_MAX;
		double Ly = DBL_MAX;

		double Rx = DBL_MAX;
		double Ry = DBL_MAX;

		double Midx = DBL_MAX;

		uint Left = m_T->GetLeft(Node);
		uint Right = m_T->GetRight(Node);

		bool IsLeaf = m_T->IsLeaf(Node);
		if (!IsLeaf)
			{
			asserta(Left != UINT_MAX && Right != UINT_MAX);

			Lx = m_Xs[Left];
			Ly = m_Ys[Left];

			Rx = m_Xs[Right];
			Ry = m_Ys[Right];

			double Minx = min(Lx, Rx);
			Midx = (X + Minx)/2;
			}

		m_Xs[Node] = X;
		m_Ys[Node] = Y;
		m_Midxs[Node] = Midx;
		m_Lxs[Node] = Lx;
		m_Lys[Node] = Ly;
		m_Rxs[Node] = Rx;
		m_Rys[Node] = Ry;
		}
	}

void Layout::Render(const string &SvgFileName)
	{
	Svg S;
	S.Open(SvgFileName, m_Width + 2*m_Margin + m_OffsetX,
	  m_Height + 2*m_Margin + m_OffsetY);
	Render(S);
	S.Close();
	}

/***
Y  ^         
Ly |       --b--------(L)
   |      |
   |      |
 Y |     (N) Node
   |      |
   |      d
Ry |       --c----(R)
   |
   ----------------------------->
         X         Rx  Lx       X
***/
void Layout::RenderNode(Svg &S, uint Node)
	{
	bool IsLeaf = m_T->IsLeaf(Node);
	double X = m_Xs[Node];
	double Y = m_Ys[Node];
	m_MaxY = max(Y, m_MaxY);
	double Rx = m_Rxs[Node];
	double Ry = m_Rys[Node];
	double Lx = m_Lxs[Node];
	double Ly = m_Lys[Node];
	if (IsLeaf)
		{
		asserta(Lx == DBL_MAX);
		asserta(Ly == DBL_MAX);
		asserta(Rx == DBL_MAX);
		asserta(Ry == DBL_MAX);
		const string &Label = m_T->GetLabel(Node);
		if (opt(draw_leaf_labels) && Label != "")
			S.Text(X+5, Y+1, "Arial", m_LabelFontSize,
				"normal", m_LeafLabelColor, "start", Label);
		if (m_TriangleWidth > 0)
			{
			double WW = m_TriangleWidth;
			double HH = m_TriangleHeight;
			string Color;
			GetColor(Node, Color);
			S.Triangle(X-2, Y+1, X+WW, Y-HH, X+WW, Y+HH, 1, Color, Color);
			}
		return;
		}

	asserta(Lx != DBL_MAX);
	asserta(Ly != DBL_MAX);
	asserta(Rx != DBL_MAX);
	asserta(Ry != DBL_MAX);

	string Color;
	uint Left = m_T->GetLeft(Node);
	uint Right = m_T->GetRight(Node);
	if (m_TriangleWidth > 0)
		Color = m_DefaultColor;
	else
		GetColor(Node, Color);
	S.Line(X, Ly, Lx, Ly, m_StrokeWidth, Color);		// b
	S.Line(X, Ry, Rx, Ry, m_StrokeWidth, Color);		// c
	S.Line(X, Ly, X, Ry, m_StrokeWidth, Color);			// d

	if (opt(draw_internal_labels))
		{
		string Label = m_T->GetLabel(Node);
		if (Label != "")
			{
			if (opt(internal_labels_pct))
				{
				if (IsValidFloatStr(Label))
					{
					double Fract = StrToFloat(Label);
					asserta(Fract >= 0 && Fract <= 1);
					Ps(Label, "%u", uint(Fract*100));
					}
				}
			double LabelX = X + 10;
			double LabelY = Y + 5;
			S.Text(LabelX, LabelY, "Arial", m_LabelFontSize,
				"normal", m_LeafLabelColor, "start", Label);
			}
		}
	}

void Layout::Render(Svg &S)
	{
	const uint NodeCount = m_T->GetNodeCount();
	m_MaxY = 0;
	for (uint Node = 0; Node < NodeCount; ++Node)
		RenderNode(S, Node);

	if (m_Title != "" && m_TitleFontSize > 0)
		S.Text(m_OffsetX, m_MaxY + m_Margin + m_TitleFontSize/2,
		  "Arial", m_TitleFontSize, "normal", "black", "start", m_Title);
	}
