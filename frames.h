/*EECS 300 Final Project Code Team 11: frames.h
Version: 1.0  Beta
Updated: FRI11FEB22

Frames tracks blobs between caputred therma camera frames
All functions have been tested on 9x9 Arrays

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#ifndef FRAMES_H
#define FRAMES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


#define NEIGHNUM 8
#define LABELLEN 3
#define COORDDIM 2

//Augmentable CONSTANTS
//!!!!: Check ROW and COL before testing matrices
#define ROW 9 
#define COL 9 
#define BLOBLIM 8
#define HOTTHRESH 0.5

struct blob
{
	float dist;
	short oldInd;
	short newInd;

};

//Checks the validity of  DFS island pixel indices, called by DFS 
int validLoc(int temp_in[][COL], bool visited_in[][COL], short row_in, short col_in);
//Does DFS to explore blob finding size and centroid of a single island
void DFS(int temp_in[][COL], bool visited_in[][COL], float blobLabel_in[LABELLEN], short row_in, short col_in);
//Distills a single frame down to a Blob Location Table
void singleFrame(int temp_in[][COL], short& numBlobs_in, float blobTable_in[][COORDDIM]);
//Converts an array of raw sensor values to binary based on HOTTHRESH
void arr2Bin(int temp_in[][COL]);
//Computes the distance squared, used in fillDist find distances between old and new frames
float distCalc(float r1_in, float c1_in, float r2_in, float c2_in);
//Computes vertical distance to either exit or entrance lines used in orphanCare to aid counting enters/exits
float vertDistCalc(float r1_in, float r2_in);
//Comparator used by qsort() to arrange the distance matrix in ascendign order
int distComp(const void* a, const void* b);
//Comparator for qsort() to arrange blockList
int shortComp(const void* a, const void* b);
//Takes a new and old Blob Location table and computes the distances between all entries
void fillDist(float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in);
//Matches the new blobs with their closest old blob
void matchShortest(float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in, short blockList_in[BLOBLIM], short& taken_in);
//Takes care of cases when the new Blob Table is smaller or larger than the old, determines enters/exits
void orphanCare(float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, short blockList_in[BLOBLIM], short& taken_in, short& crossCount_in);
//MAnages the full process of updating the old Blob Table to the new Blob Table and counts enters/exits happening during this update
short updateLocs(float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in, short& crossCount_in);

#endif
