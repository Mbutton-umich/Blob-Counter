/*EECS 300 Final Project Code Team 11: testUtil.h
Version: 3.0 Done Deal
Updated: FRI26FEB22

testUtil functions as a test bench for the frames code. Checking inputs, outputs, and performance time. Also has installation calculation functions.

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#pragma once
#ifndef testUtil_H
#define _CRT_SECURE_NO_WARNINGS
#define testUtil_H

#include "frames.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h> //For timing performance

//Reads in test matrices from a folder named "testMats" inside program directory, where files are named test#.txt, where testNum is #
void readTestMat(bool myNums_in[][COL], int testNum_in);
//Prints a test matrix for verifiction
void printTestMat(bool myNums_in[][COL]);
//Swaps new blob table into old, useful for testing single frame
void blobSwap();
//Prints the Blob Table for a Single Frame
void printBlobTable(const struct blobElem blobTable_in[BLOBLIM], const short blobNum_in);
//Prints the Distance Table for old and new frames
void printDistTable();
//Prints the tally of crossings
void printDeltaPeeps();
//Prints the number of people inside the room
void printNumPeep(const short& numPeeps_in);
//Walkthrough of the two frame update process
void processWalkthrough(bool temp_in[][COL], int oldTestNum_in, int newTestNum_in);
//Does up to a 10 test frame simulation, prints out roomoccupancy after each frame
void frameSeqTest(int seqLen_in, int startNum_in, bool temp_in[][COL], short& peepNum_in);
//Takes in a sequence of test matrices and puts them into C form to put into headerfile to loaod onto the ESP32
void printTestMats2HeaderForm(int seqLen_in, int startNum_in);
//Gives you install dimensions for a given door width
void installDims();
//Gets nanosecond system time (may not be actual system time)
long get_nanos();

#endif
