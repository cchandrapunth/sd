
#include "ui_button.h"

#pragma once
#define MAX_LIST 10

class ui{
public:

	ui_button** button_list;
	int count;	//size of the list
	bool activate_menu;

	ui(void);
	void add_button(const char* name, int x, int y, bool activated);
	void draw();
	void check_click(int hand_x, int hand_y);

	void push_menu(int fs_x, int fs_y);
};

extern ui* Master_ui;