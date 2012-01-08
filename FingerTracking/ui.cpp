#include "StdAfx.h"
#include <deque>
#include "ui.h"

using namespace std;

ui::ui(void)
{
	button_list = (ui_button**) malloc(sizeof(ui_button*)* MAX_LIST);
	count = 0;
	activate_menu = false;
}


//------------add_button--------------------------
void ui::add_button(const char* name, int x, int y, bool activated){

	//initialize button 
	ui_button *b = new ui_button(name, count, x, y, activated);
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
			if(b->wait >15){
				if(b->currently_inside) {
					activate_menu = b->hand_up_handler(hand_x, hand_y, true);
				}
			}	
		} 
		else{
			b->hand_up_handler(hand_x, hand_y, false);
			b->wait = 0;
		}
	}
}
//-----------------push menu-------------------
void ui::push_menu(int fs_x, int fs_y){
	//draw panel 

	printf("pushing menu\n");

}