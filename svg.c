#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "svg.h"
char* svgColorMap[] = {"white", "black", "gray", "red", "blue", "yellow", "orange", "purple", "green"};
/*double fmin(double a, double b){
	if(a < b) return a;
	return b;
}
double fmax(double a, double b){
	if(a > b) return a;
	return b;
}*/
void calculateSvgFraming(svg* t){
	double diffX = t->maxX - t->minX;
	double diffY = t->maxY - t->minY;
	t->scale = 100/fmax(diffX, diffY);
	t->dX = (-t->minX)+(fmax(diffX, diffY)-diffX)/2;
	t->dY = (-t->minY)+(fmax(diffX, diffY)-diffY)/2;
	//printf("Framing:\ndifX\t%lf\ndifY\t%lf\ndX\t%lf\ndY\t%lf\nScl\t%lf\n", diffX, diffY, t->dX, t->dY, t->scale);
}
svg* newSvg(){
	svg* ret = malloc(sizeof(svg));
	ret->first = NULL;
	ret->minX = INFINITY;//These variables are used to squeeze the output in the viewport.
	ret->maxX = -INFINITY;
	ret->minY = INFINITY;
	ret->maxY = -INFINITY;
	return ret;
}

void printSvgObj(FILE* fp, svgObj* t, double dX, double dY, double scale){
	char paint[80];
	if(t->type == 'R'){
		struct svgRect* r = (&t->rect);
		//printf("rectangle: x: %lf y: %lf w: %lf h: %lf\n", r->x1, r->y1, r->x2-r->x1, r->y2-r->y1);
		if(r->fill){
			sprintf(paint, "fill=\"%s\"", svgColorMap[t->c]);
		}else{
			sprintf(paint, "fill=\"none\" stroke=\"%s\"", svgColorMap[t->c]);
		}
		fprintf(fp, "<rect x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" %s stroke-width=\"%lf\"/>\n", (r->x1 + dX)*scale, (r->y1 + dY)*scale, (r->x2 - r->x1)*scale, (r->y2 - r->y1)*scale, paint, 1.0*scale);
	}else if(t->type == 'L'){
		struct svgLine* l = (&t->line);
		fprintf(fp, "<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" stroke-width=\"%lf\" stroke=\"%s\"/>\n", (l->x1 + dX)*scale, (l->y1 + dY)*scale, (l->x2 + dX)*scale, (l->y2 + dY)*scale, 1.0*scale, svgColorMap[t->c]);
	}else if(t->type == 'C'){
		struct svgCircle* c = (&t->circle);
		if(c->fill){
			sprintf(paint, "fill=\"%s\"", svgColorMap[t->c]);
		}else{
			sprintf(paint, "fill=\"none\" stroke=\"%s\"", svgColorMap[t->c]);
		}
		fprintf(fp, "<circle cx=\"%lf\" cy=\"%lf\" r=\"%lf\" %s stroke-width=\"%lf\"/>\n", (c->x + dX)*scale, (c->y + dY)*scale, c->r*scale, paint, 1.0*scale);
	}else if(t->type == 'S'){
		struct svgRef* r = (&t->ref);
		fprintf(fp, "<image fill=\"freeze\" x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" transform=\"rotate(%lf %lf %lf)\" xlink:href=\"%s\"/>\n", (r->x + dX)*scale, (r->y +dY)*scale, r->w * scale, r->h * scale, r->r*180/M_PI, (r->x+0.5*r->w+dX)*scale, (r->y+0.5*r->h+dY)*scale, r->path);
	}
}

void saveSvg(svg* target){
	printf("Saving SVG %s\n", target->path);
	FILE* fp = fopen(target->path, "w");
	if(fp == NULL){
		puts("File opening failed!");
		return;
	}
	calculateSvgFraming(target);
	fprintf(fp, "<?xml version=\"1.0\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.2\" baseProfile=\"tiny\"\n\tviewBox=\"0 0 100 100\">\n<desc>SVG Output</desc>");
	for(svgObj* curr = target->first; curr != NULL; curr = curr->next){
		printSvgObj(fp, curr, target->dX, target->dY, target->scale);
	}
	fprintf(fp, "</svg>");
	fclose(fp);
}
void freeSvg(svg* target){
	svgObj* next;
	for(svgObj* curr = target->first; curr != NULL; curr = next){
		next = curr->next;
		free(curr);
	}
	free(target);
}

void addSvgObj(svg* t, svgObj* obj){
	if(t->first == NULL){
		t->first = obj;
		obj->next = NULL;
	}else{
		svgObj** prevPtr = &(t->first);
		svgObj* cond = t->first;
		while(cond != NULL && cond->depth > obj->depth){
			prevPtr = &(cond->next);
			cond = cond->next;
		}
		obj->next = cond;
		*prevPtr = obj;
	}
	double x1, y1, x2, y2;
	if(obj->type == 'R'){
		x1 = obj->rect.x1;
		y1 = obj->rect.y1;
		x2 = obj->rect.x2;
		y2 = obj->rect.y2;
		if(x1 < t->minX){
			t->minX = x1;
		}
		if(y1 < t->minY){
			t->minY = y1;
		}
		if(x2 > t->maxX){
			t->maxX = x2;
		}
		if(y2 > t->maxY){
			t->maxY = y2;
		}
	}else if (obj->type == 'L'){
		x1 = obj->line.x1;
		y1 = obj->line.y1;
		x2 = obj->line.x2;
		y2 = obj->line.y2;
		if(fmin(x1, x2) < t->minX){
			t->minX = fmin(x1, x2);
		}
		if(fmin(y1, y2) < t->minY){
			t->minY = fmin(y1, y2);
		}
		if(fmax(x1, x2) > t->maxX){
			t->maxX = fmax(x1, x2);
		}
		if(fmax(y1, y2) > t->maxY){
			t->maxY = fmax(y1, y2);
		}
	}else if (obj->type == 'C'){
		x1 = obj->circle.x;
		y1 = obj->circle.y;
		if(x1 - obj->circle.r < t->minX){
			t->minX = x1 - obj->circle.r;
		}
		if(y1 - obj->circle.r < t->minY){
			t->minY = y1 - obj->circle.r;
		}
		if(x1 + obj->circle.r > t->maxX){
			t->maxX = x1 + obj->circle.r;
		}
		if(y1 + obj->circle.r > t->maxY){
			t->maxY = y1 + obj->circle.r;
		}
	}else if (obj->type == 'S'){
		x1 = obj->ref.x+obj->ref.w/2;
		y1 = obj->ref.y+obj->ref.h/2;
		double ra = sqrt(pow(obj->ref.w/2, 2)+pow(obj->ref.h/2, 2));
		if(x1 - ra < t->minX){
			t->minX = x1 - ra;
		}
		if(y1 - ra < t->minY){
			t->minY = y1 - ra;
		}
		if(x1 + ra > t->maxX){
			t->maxX = x1 + ra;
		}
		if(y1 + ra < t->maxY){
			t->maxY = y1 + ra;
		}
	}
}
svgObj* newSvgLine(double x1, double y1, double x2, double y2, svgColor c, int depth){
	svgObj* ret = malloc(sizeof(svgObj));
	ret->type = 'L';
	ret->depth = depth;
	ret->c = c;
	ret->line.x1 = x1;
	ret->line.x2 = x2;
	ret->line.y1 = y1;
	ret->line.y2 = y2;
	return ret;
}
svgObj* newSvgCircle(double x, double y, double r, svgColor c, int fill, int depth){
	svgObj* ret = malloc(sizeof(svgObj));
	ret->type = 'C';
	ret->depth = depth;
	ret->circle.x = x;
	ret->circle.y = y;
	ret->circle.r = r;
	ret->c = c;
	ret->circle.fill = fill;
	return ret;
}
svgObj* newSvgRect(double x1, double y1, double x2, double y2, svgColor c, int fill, int depth){
	svgObj* ret = malloc(sizeof(svgObj));
	ret->type = 'R';
	ret->depth = depth;
	ret->rect.x1 = fmin(x1, x2);
	ret->rect.x2 = fmax(x1, x2);
	ret->rect.y1 = fmin(y1, y2);
	ret->rect.y2 = fmax(y1, y2);
	ret->c = c;
	ret->rect.fill = fill;
	return ret;
}
svgObj* newSvgRef(char* path, double x, double y, double w, double h, double r, svgColor c, int depth){
	svgObj* ret = malloc(sizeof(svgObj));
	strncpy(ret->ref.path, path, 21);
	ret->ref.path[20] = 0;
	ret->type = 'S';
	ret->depth = depth;
	ret->ref.x = x;
	ret->ref.y = y;
	ret->ref.w = w;
	ret->ref.h = h;
	ret->ref.r = r;
	ret->c = c;
	return ret;
}
