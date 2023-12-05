#include <stdio.h>
#include "DebugPrefTools.h"

#define TRUE 1
#define FALSE 0

struct PrefData
{
	int start;
	int end;
	int totalTime;
	int numCalls;
	char* tokenName;
}  ;

int lastInx = -1;

struct PrefData dataArr[100];

///////////////////////////////////////////////////////////
//
int startPrefAction(char* tokenName)
{
	int i, found = FALSE;

	for (i = lastInx; i >= 0; i--)
		if (!strcmp(tokenName, dataArr[i].tokenName))
		{
			found = TRUE;
			break;
		}

	if (!found)
	{
		i = ++lastInx;
		dataArr[i].tokenName = tokenName;
		dataArr[i].numCalls = 0;
		dataArr[i].totalTime = 0;
	}

	dataArr[i].start = millis();
	dataArr[i].numCalls++;

	return i;
}

///////////////////////////////////////////////////////////
//
int endPrefAction(int idx)
{
	int time;
	dataArr[idx].end = millis();
	time = dataArr[idx].end - dataArr[idx].start;
	dataArr[idx].totalTime = dataArr[idx].totalTime + time;

	return time;
}

///////////////////////////////////////////////////////////
//
int getAvgPrefTime(int idx)
{
	endPrefAction(idx);
	return (dataArr[idx].totalTime / dataArr[idx].numCalls);
}