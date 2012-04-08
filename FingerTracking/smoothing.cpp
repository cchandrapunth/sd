#include "stdafx.h"
#include "smoothing.h"
#include <map>
#include <list>

#define SMOOTHSIZE 10
#define GRABTHRES 0.6
int grabList[SMOOTHSIZE];
int index =0;
int sum;

static std::map<int, std::list<int>> smoothlist;

/********************************************************
					Hand smoothing
********************************************************/
void smoothHand(int i, int id){

	smoothlist[id].push_front(i);
	// Keep size of history buffer
	if (smoothlist[id].size() > SMOOTHSIZE)
		smoothlist[id].pop_back();
}

bool isGrab(int id){
	//find everage 
	std::map<int, std::list<int> >::const_iterator HandIterator;
	int sum =0, count = 0;

	for (HandIterator = smoothlist.begin();HandIterator != smoothlist.end();++HandIterator) {		
		if(HandIterator->first == id){
			std::list<int>::const_iterator grabIterator;
			for (grabIterator = HandIterator->second.begin();
				grabIterator != HandIterator->second.end();
				++grabIterator){

					sum += *grabIterator;
					count++;
			}
			break;
		}
	}
	if(sum/count > GRABTHRES) return true;
	else return false;
}

void deleteSmHand(int id){

}