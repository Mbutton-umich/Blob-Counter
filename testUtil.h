/*EECS 300 Final Project Code Team 11: testUtil.h
Version: 1.0  Beta
Updated: FRI11FEB22

testUtil functions as a test bench for the frames code. Checking inputs, outputs, and performance time. Also has installation calculation functions.

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#ifndef testUtil_H
#define testUtil_H
#define _CRT_SECURE_NO_WARNINGS //Makes the fopen and fscan easier
#include "frames.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h> //For timing performance

//!!!!: Changed ROW COL for testing output
#define ROWS 9
#define COLS 9

//Reads in test matrices from a folder named "testMats" inside program directory, where files are named test#.txt, where testNum is #
void readTestMat(int myNums_in[][COLS], int testNum_in);

//Prints a test matrix for verifiction
void printTestMat(int myNums_in[][COLS]);

//Prints the Blob Table for a Single Frame
void printBlobTable(const float blobTable_in[][COORDDIM], const short& numBlobs_in);

//Prints the Distance Table for old and new frames
void printDistTable(const struct blob dist_in[(BLOBLIM * BLOBLIM)], const short& count_in);

//Prints the Block List used when matching up blobs and when counting exits
void printBlockList(const short blockList_in[2 * BLOBLIM], const short& taken_in);

//Prints the tally of crossings
void printCrossCount(const short& crossCount_in);

//Walkthrough of the two frame update process
void processWalkthrough(int temp_in[][COL], float oldBT_in[][COORDDIM], short& oldNum_in, float newBT_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in, short& crossCount_in, int oldTestMat, int newTestMat);

//Gets nanosecond system time (may not be actual system time)
long get_nanos();
#endif
