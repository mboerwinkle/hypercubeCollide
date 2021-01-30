#ifndef SVG_H
#define SVG_H
typedef enum svgColor{
	white, black, gray,
	red, blue, yellow,
	orange, purple, green
}svgColor;

struct svgLine{
	double x1, y1, x2, y2;
};
struct svgRect{
	double x1, y1, x2, y2;
	int fill;
};
struct svgCircle{
	double x, y, r;
	int fill;
};
struct svgRef{
	double x, y, w, h, r;
	char path[21];
};
typedef struct svgObj{
	char type;//*L*ine *R*ect *C*ircle *S*VG
	svgColor c;
	int depth;//2 is behind 1, 3 behind 2, etc
	struct svgObj* next;
	union{
		struct svgLine line;
		struct svgRect rect;
		struct svgCircle circle;
		struct svgRef ref;
	};
}svgObj;
typedef struct svg{
	char path[80];
	double minX, minY, maxX, maxY;
	double dX, dY, scale;//these are calculated from min/max X/Y
	svgObj* first;
}svg;
extern svg* newSvg();
extern void saveSvg(svg* target);
extern void freeSvg(svg* target);
extern void addSvgObj(svg* target, svgObj* obj);
extern svgObj* newSvgRect(double x1, double y1, double x2, double y2, svgColor c, int fill, int depth);
extern svgObj* newSvgLine(double x1, double y1, double x2, double y2, svgColor c, int depth);
extern svgObj* newSvgCircle(double x, double y, double r, svgColor c, int fill, int depth);
extern svgObj* newSvgRef(char* path, double x, double y, double w, double h, double r, svgColor c, int depth);
#endif
