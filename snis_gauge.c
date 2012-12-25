#include <string.h>
#include "math.h"
#include <gtk/gtk.h>

#define SNIS_GAUGE_GLOBALS
#include "mathutils.h"
#include "snis_font.h"
#include "snis_typeface.h"
#include "snis_graph.h"
#include "snis_gauge.h"

struct gauge {
	int x, y, r;
	gauge_monitor_function sample;
	gauge_monitor_function sample2;
	double r1,r2;
	double start_angle, angular_range;
	int needle_color, dial_color, needle_color2;
	int ndivs;
	char title[16]; 
};

void gauge_add_needle(struct gauge *g, gauge_monitor_function sample, int color)
{
	g->needle_color2 = color;
	g->sample2 = sample;
}

void gauge_init(struct gauge *g, 
			int x, int y, int r, double r1, double r2,
			double start_angle, double angular_range,
			int needle_color, int dial_color, int ndivs, char *title,
			gauge_monitor_function gmf)
{
	g->x = x;
	g->y = y;
	g->r = r;
	g->r1 = r1;
	g->r2 = r2;
	g->start_angle = start_angle;
	g->angular_range = angular_range;
	g->needle_color = needle_color;
	g->dial_color = dial_color;
	g->ndivs = ndivs;
	g->sample = gmf;
	strncpy(g->title, title, sizeof(g->title) - 1);
	g->sample2 = NULL;
}

void draw_gauge_needle(GdkDrawable *drawable, GdkGC *gc,
		gint x, gint y, gint r, double a)
{
	int x1, y1, x2, y2, x3, y3, x4, y4;

	x1 = r *  sin(a) * 0.9 + x;
	y1 = r * -cos(a) * 0.9 + y;
	x2 = r * -sin(a) * 0.2 + x;
	y2 = r *  cos(a) * 0.2 + y;
	x3 = r *  sin(a + M_PI / 2.0) * 0.05 + x;
	y3 = r * -cos(a + M_PI / 2.0) * 0.05 + y;
	x4 = r *  sin(a - M_PI / 2.0) * 0.05 + x;
	y4 = r * -cos(a - M_PI / 2.0) * 0.05 + y;

	sng_current_draw_line(drawable, gc, x1, y1, x3, y3);
	sng_current_draw_line(drawable, gc, x3, y3, x2, y2);
	sng_current_draw_line(drawable, gc, x2, y2, x4, y4);
	sng_current_draw_line(drawable, gc, x4, y4, x1, y1);
}

void gauge_draw(GtkWidget *w, GdkGC *gc, struct gauge *g)
{
	int i;
	double a, ai;
	int x1, y1, x2, y2, x3, y3;
	double value;
	double inc, v;
	char buffer[10], buf2[10];

	sng_set_foreground(g->dial_color);
	sng_draw_circle(w->window, gc, g->x, g->y, g->r);

	ai = g->angular_range / g->ndivs;
	normalize_angle(&ai);

	v = g->r1;
	inc = (double) (g->r2 - g->r1) / (double) g->ndivs;
	for (i = 0; i <= g->ndivs; i++) {
		a = (ai * (double) i) + g->start_angle;
		normalize_angle(&a);
		x1 = g->r * sin(a);
		x2 = 0.9 * x1;
		y1 = g->r * -cos(a);
		y2 = 0.9 * y1;
		x3 = 0.7 * x1 - 20;
		y3 = 0.7 * y1;

		x1 = (x1 + g->x);
		x2 = (x2 + g->x);
		x3 = (x3 + g->x);
		y1 = (y1 + g->y);
		y2 = (y2 + g->y);
		y3 = (y3 + g->y);
		sng_current_draw_line(w->window, gc, x1, y1, x2, y2);
		sprintf(buf2, "%3.0lf", v);
		v += inc;
		sng_abs_xy_draw_string(w, gc, buf2, NANO_FONT, x3, y3);
	}
	sng_abs_xy_draw_string(w, gc, g->title, TINY_FONT,
			(g->x - (g->r * 0.5)), (g->y + (g->r * 0.5)));
	value = g->sample();
	sprintf(buffer, "%4.2lf", value);
	sng_abs_xy_draw_string(w, gc, buffer, TINY_FONT,
			(g->x - (g->r * 0.5)), (g->y + (g->r * 0.5)) + 15);

	a = ((value - g->r1) / (g->r2 - g->r1))	* g->angular_range + g->start_angle;
	sng_set_foreground(g->needle_color);
	draw_gauge_needle(w->window, gc, g->x, g->y, g->r, a); 

	if (g->sample2) {
		a = ((g->sample2() - g->r1) / (g->r2 - g->r1)) * g->angular_range + g->start_angle;
		sng_set_foreground(g->needle_color2);
		draw_gauge_needle(w->window, gc, g->x, g->y, g->r * 0.8, a); 
	}
}

/*
 * end gauge related functions/types
 */

