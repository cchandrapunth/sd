#include "stdafx.h"
#include "smoothing.h"

#define SMOOTHSIZE 10
#define GRABTHRES 0.6
int grabList[SMOOTHSIZE];
int index =0;
int sum;
bool first_time = true;

/********************************************
				Hand smoothing
********************************************/
void init(){
	for(int i=0;i<SMOOTHSIZE; i++){
		grabList[i] = 1;
	}
	sum = 30;
}

void setNext(){
	if(index == SMOOTHSIZE-1) index-=(SMOOTHSIZE-1);
	else index ++;
}

//average number from list
float findAverage(){
	sum = 0;
	for(int i=0; i< SMOOTHSIZE; i++){
		sum= sum+ grabList[i];
	}
	return (float)sum/SMOOTHSIZE;
}


//maintain the list of 30 recent hand gesture
int smoothHand(int i){
	if(first_time){
		init();
		first_time = false;
	}
	grabList[index] = i;
	setNext();
	float avg = findAverage();
	
	if(avg > 0.6) return -1;
	else return 1;
}

/********************************************
				Track smoothing
********************************************/
