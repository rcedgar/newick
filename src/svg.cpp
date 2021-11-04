#include "myutils.h"
#include "svg.h"

void Svg::Triangle(double x1, double y1, double x2, double y2,
  double x3, double y3, double LineWidth,
  const string &LineColor, const string &FillColor)
	{
	fprintf(m_f, 
	  "<path d=\""
	  " M %.8g %.8g"
	  " L %.8g %.8g"
	  " L %.8g %.8g"
	  " Z\""
	  " stroke-width=\"%.8g\""
	  " stroke=\"%s\""
	  " fill=\"%s\""
	  " />",
	  x1, y1, x2, y2, x3, y3, LineWidth, LineColor.c_str(), FillColor.c_str());
	}

void Svg::Rect(double x, double y, double w, double h,
  double LineWidth, const string &LineColor,
  const string &FillColor)
	{
	if (m_f == 0)
		return;

	fprintf(m_f, 
	  "<rect"
		" x=\"%.8g\""
	    " y=\"%.8g\""
	    " width=\"%.8g\""
	    " height=\"%.8g\""
	    " stroke-width=\"%.8g\""
	    " stroke=\"%s\""
	    " fill=\"%s\""
		" />\n",
		x, y, w, h, LineWidth, LineColor.c_str(), FillColor.c_str());
	}

void Svg::Text(double x, double y, const string &FontFamily,
  double FontSize, const string &FontWeight,
  const string &FillColor, const string &TextAnchor,
  const string &Str)
	{
	if (m_f == 0)
		return;

	fprintf(m_f, 
	  "<text"
		" x=\"%.8g\""
	    " y=\"%.8g\""
		" font-family=\"%s\""
	    " font-size=\"%.8g\""
		" font-weight=\"%s\""
		" fill=\"%s\""
		" text-anchor=\"%s\""
		">"
		"%s"
		"</text>\n",
	  x, y, FontFamily.c_str(),
	  FontSize, FontWeight.c_str(), FillColor.c_str(),
	  TextAnchor.c_str(), Str.c_str());
	}

void Svg::Line(double x1, double y1, double x2, double y2,
  double StrokeWidth, const string &Color)
	{
	if (m_f == 0)
		return;

	fprintf(m_f, 
	  "<line"
		" x1=\"%.8g\""
	    " y1=\"%.8g\""
		" x2=\"%.8g\""
		" y2=\"%.8g\""
		" stroke-width=\"%.8g\""
		" stroke=\"%s\""
		" />\n",
		x1, y1, x2, y2, StrokeWidth, Color.c_str());
	}

void Svg::Open(const string &FileName, double Width, double Height)
	{
	m_f = CreateStdioFile(FileName);
	m_Width = Width;
	m_Height = Height;
	fprintf(m_f, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	fprintf(m_f, "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"%.8g\" height=\"%.8g\" >\n",
	  m_Width, m_Height);
	}

void Svg::Close()
	{
	if (m_f == 0)
		return;
	fprintf(m_f, "</svg>\n");
	CloseStdioFile(m_f);
	m_f = 0;
	}
