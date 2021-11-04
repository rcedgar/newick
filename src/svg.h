#pragma once

class Svg
	{
public:
	FILE *m_f = 0;
	double m_Width = 0;
	double m_Height = 0;

public:
	void Open(const string &FileName, double Width, double Height);
	void Close();

	void Line(double x1, double y1, double x2, double y2,
	  double StrokeWidth, const string &Color);

	void Rect(double x, double y, double w, double h,
	  double LineWidth, const string &LineColor,
	  const string &FillColor);

	void Text(double x, double y, const string &FontFamily,
	  double FontSize, const string &FontWeight,
	  const string &FillColor, const string &TextAnchor,
	  const string &Str);

	void Triangle(double x1, double y1, double x2, double y2,
	  double x3, double y3, double LineWidth,
	  const string &LineColor, const string &FillColor);
	};
