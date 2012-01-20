#include "StdAfx.h"
#include <gl/glut.h>
#include "ui.h"

#include <string.h>

void *font = GLUT_BITMAP_TIMES_ROMAN_24;

ui_button::ui_button(const char *name, long id, int begin_x, int begin_y, int width, int height, cb_function cb)
{
	h = height;
	w = width;

	butt_name = name;
	user_id = id;
	currently_inside = false;
	x = begin_x;
	y = begin_y;
	callback = cb;

	wait = 0;
}

ui_button::ui_button(const char *name, long id, int begin_x, int begin_y, cb_function cb)
{
	common_init();
	butt_name = name;
	user_id = id;
	currently_inside = false;
	x = begin_x;
	y = begin_y;
	callback = cb;

	wait = 0;
}

int  ui_button::hand_down_handler( int local_x, int local_y ){
	currently_inside = true;
	draw();

	return false;
}

void ui_button::reactivate(){
	activate = true;
}

int  ui_button::hand_up_handler( int local_x, int local_y, bool inside ){
	currently_inside = false; 
	draw();

	if ( inside ) {
		if(!execute){
			activate = false;
			execute = true;
			
			callback();

			return true;
		}
	}
	else{
		execute = false; 
	}
	return false;
}


void ui_button::draw(){
	if(activate){
		glDisable(GL_TEXTURE_2D);
		if(currently_inside) draw_pressed();
		else {
			draw_text(0);
		}
	}
}

void ui_button::draw_pressed(){

  draw_text( 1 );
  glColor3f( 0.0, 0.0, 0.0 );
  glBegin( GL_LINE_LOOP );
  glVertex2i( x, y );         glVertex2i( x+w, y );
  glVertex2i( x+w, y+h );         glVertex2i( x, y+h );
  glEnd();

  glBegin( GL_LINE_LOOP );
  glVertex2i( x+1, y+1 );         glVertex2i( x+w-1, y+1 );
  glVertex2i( x+w-1, y+h-1 );     glVertex2i( x+1, y+h-1 );
  glEnd();
}

void ui_button::draw_text(int sunken){

  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);

  if(sunken == 1) {
	glColor3f(1.0,1.0,1.0);
  }
  else{
	glColor3f(0.5,0.5,0.5);
  }

  glBegin( GL_QUADS );
  glVertex3i( x+2, y+2, -5);         glVertex3i( x+w-2, y+2, -5);
  glVertex3i( x+w-2, y+h-2, -5);     glVertex3i( x+2, y+h-2, -5);
  glEnd();

  glColor3f(0.0, 0.0, 0.0);
  output(x+w/5, y+h/4, (char*) butt_name);
}


void ui_button::output(int x, int y, char *string)
{
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}