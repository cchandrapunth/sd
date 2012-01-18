#include "StdAfx.h"
#include <deque>
#include "math.h"
#include <gl/glut.h>

#include "ui.h"

using namespace std;

ui::ui(void)
{
	button_list = (ui_button**) malloc(sizeof(ui_button*)* MAX_LIST);
	count = 0;
	activate_menu = false;
}


//------------add_button--------------------------
void ui::add_button(const char* name, int x, int y, int ww, int hh, cb_function cb){

	//initialize button 
	ui_button *b = new ui_button(name, count, x, y,ww, hh, cb);
	button_list[count] = b;
	count++;
}

void ui::add_button(const char* name, int x, int y, cb_function cb){

	//initialize button 
	ui_button *b = new ui_button(name, count, x, y, cb);
	button_list[count] = b;
	count++;
}

//----------------------draw--------------------
void ui::draw(){
	for(int i=0; i< count; i++){
		ui_button *b = button_list[i];
		b->draw();
	}

}

//-------------------check_click------------------
void ui::check_click(int hand_x, int hand_y){
	
	for(int i=0; i< count; i++){
		ui_button *b = button_list[i];
		
		int width = b->w;
		int height = b->h;

		//inside?
		if(hand_x > b->x && hand_x < b->x+width && hand_y > b->y && hand_y < b->y+height){
			b->hand_down_handler(hand_x, hand_y);
			b->wait = b->wait+1;
			draw_circle(hand_x, hand_y, b->wait);
			if(b->wait >24){
				if(b->currently_inside) {
					b->hand_up_handler(hand_x, hand_y, true);					
				}
			}	
		} 
		else{
			b->hand_up_handler(hand_x, hand_y, false);
			b->wait = 0;
		}
	}

	if(activate_menu){
		add_panel();
	}
}

//----------------remove menu-----------------
void ui::remove_menu(){
	count = 1;
	activate_menu = false;
	button_list[0]->reactivate();
}

//---------------------add panel----------------
void ui::add_panel(){
	
	glDisable(GL_LIGHTING);
	glColor3f(0.2,0.2,0.2);
	glBegin(GL_QUADS);
	glVertex3i( 120, 100, -10);         
	glVertex3i( 650, 100, -10);
    glVertex3i( 650, 450, -10);     
	glVertex3i( 120, 450, -10);
	glEnd();
	
}
//--------------draw circle-----------------------
void ui::draw_circle(int x1, int y1, int angle){

	glDisable(GL_LIGHTING);
	glColor3f(0.3,0.8,0.8);

	glBegin(GL_LINE_STRIP);
	glVertex3f(x1, y1, 10);
	for(int a= 0; a < angle*15; a+=5){
		glVertex3f(x1 + sin((double)a) * 20, y1 + cos((double)a) * 20, 10);
	}
	glEnd();
}
