/*EECS 300 Final Project Code Team 11: frames.h
Version: 3.0 Done Deal
Updated: FRI26FEB22

Frames tracks blobs between caputred therma camera frames
All functions have been tested on 9x9 Arrays

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#pragma once

#ifndef FRAMES_H
#define FRAMES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

//Augmentable CONSTANTS
//!!!!: Check ROW and COL before testing matrices
#define ROW 32
#define COL 24
//The number of neighbor pixels checked during the DFS
#define NEIGHNUM 8
//Max size of the  stack for the DFS
#define STACKMAX ROW*COL
//Max number of blobs we can track at once
#define BLOBLIM 8
//The maximum scan space distance we can connect an old and new blob 
//!!!!: This is actually distance squared because of how I lazily compute distance
#define INTRADISTLIM ROW*ROW/4

//Coordinatess that fit into the stack during DFS
struct coord
{
	short r;
	short c;
};
//A blank label for marking islands while searching for blobs inside a frame
struct islandLabel
{
	float r;
	float c;
	//Made to be float for centroid division later !!!!: If we need to filter by blob size use this
	float size;
};
//Struct for the Blob Table entries
struct blobElem
{
	float r;
	float c;
	//Tells where in the current fram this blob has been involved in a match
	bool matched;
	bool top;
	bool bottom;
};
//Struct for the Distance Table entries
struct distElem
{
	float dist;
	short oldInd;
	short newInd;

};

///****************************Comment out extern stuff for proper encapsulation when not debugging
//Declare the stack
extern coord stack[STACKMAX];
extern short top;

//Declare structure and counter of the New Blob Table
extern struct blobElem newBT[BLOBLIM];
extern short newNum;

//Declare structure and counter of the Old Blob Table
extern struct blobElem oldBT[BLOBLIM];
extern short oldNum;

//Declare structure and counter for the Distance Table
extern struct distElem distT[(BLOBLIM * BLOBLIM)];
extern short distNum;

//Declare Crossing Counter
extern short deltaPeeps;

///****************************Comment out extern stuff for proper encapsulation when not debugging

//Push method for DFS Stack
void push(short r_in, short c_in);
//Pop method for the DFS Stack
coord pop();
//Empty check method for DFS
bool isEmpty();
//Checks the validity of  DFS island pixel indices, called by DFS 
int validLoc(bool temp_in[][COL], bool visited_in[][COL], short row_in, short col_in);
//Does IDFS to explore blob finding size and centroid of a single island
void IDFS(bool temp_in[][COL], bool visited_in[][COL], struct islandLabel& islandLabel_in, const short row_in, const short col_in);
//Distills a single frame down to a Blob Location Table
void singleFrame(bool temp_in[][COL]);
//Computes the distance squared, used in fillDist find distances between old and new frames
float distCalc(const float r1_in, const float c1_in, const float r2_in, const float c2_in);
//Computes vertical distance to either exit or entrance lines used in orphanCare to aid counting enters/exits
float vertDistCalc(const float r1_in, const float r2_in);
//Comparator used by qsort() to arrange the distance matrix in ascendign order
int distComp(const void* a, const void* b);
//Takes a new and old Blob Location table and computes the distances between all entries
void fillDist();
//Matches the new blobs with their closest old blob
void matchShortest();
//Takes care of cases when the new Blob Table is smaller or larger than the old, determines enters/exits
void orphanCare();
//Resets stuff for next frame
void reset();
//Manages the full process of updating the old Blob Table to the new Blob Table and counts enters/exits happening during this update
short updateLocs();

#endif
