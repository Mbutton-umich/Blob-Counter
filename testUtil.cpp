/*EECS 300 Final Project Code Team 11: testUtil.cpp
Version: 1.4  Sequence Tested
Updated: TUE15FEB22

testUtil functions as a test bench for the frames code. Checking inputs, outputs, and performance time. Also has installation calculation functions.

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "testUtil.h"

//!!!!: Must declare the ROOT and NILL globals for RBT here outside of main()
//struct node* ROOT = NULL;
//struct node* NILL = NULL;

//Reads in test matrices from a folder named "testMats", where files are named test#.txt and testNum is #
void readTestMat(int myNums_in[][COLS], int testNum_in)
{
	char  fileName[] = ".\\testMats\\test#.txt";
	fileName[15] = '0' + testNum_in;
	FILE* fp;
	fp = fopen(fileName, "r");
	for (int i = 0; i <= 9; i++)
		for (int j = 0;j<9 ; j++)
		{
			fscanf(fp, " %d", &myNums_in[i][j]);
		}
	fclose(fp);
}

//Prints a test matrix for verification
void printTestMat(int myNums_in[][COLS])
{
	printf("Printing Test Matrix Below:\n");
	for (int r = 0; r < ROWS; r++)
	{
		for (int c = 0; c < COLS; c++)
		{
			printf("%i ", myNums_in[r][c]);
		}
		printf("\n");
	}
	printf("\n");
		
}

//Prints the Blob Table for a Single Frame
void printBlobTable(const float blobTable_in[][COORDDIM], const short& numBlobs_in)
{
	printf("Printing Blob Table Below:\n");
	for (int k = 0; k < numBlobs_in; ++k)
	{
		printf("Blob #%i @\nRow: %f\nCol: %f\n", k, blobTable_in[k][0], blobTable_in[k][1]);
	}
	printf("\n");
}

void printDistTable(const struct blob dist_in[(BLOBLIM * BLOBLIM)], const short& count_in)
{
	printf("Printing Distance Table Below:\n");
	for (int k = 0; k < count_in; ++k)
	{
		printf("Old#: %i  New#: %i  Dist: %f\n", dist_in[k].oldInd, dist_in[k].newInd, dist_in[k].dist);
	}
	printf("\n");
}

void printCrossCount(const short& crossCount_in)
{
	printf("Current Cross Count: %i \n", crossCount_in);
}


void printNumPeep(const short& numPeeps_in)
{
	printf("\nThere are %i people inside the room!\n", numPeeps_in);
}

void recurPrintTree(struct node* node_in)
{
	if (node_in != NILL)
	{
		recurPrintTree(node_in->left);
		printf("%d\t", node_in->data);
		recurPrintTree(node_in->right);
	}
}

void printTree(struct node* node_in)
{
	printf("Printing Tree Contents Below (Should be in Order):\n");
	recurPrintTree(node_in);
	printf("\n");
}


//Tests the RBT tree stuff
void testTree()
{
	//Test making the tree
	initTree();

	//Fill the tree with random values
	int n = 100;
	for (int i = 0; i < n; i++)
	{
		insert((short)(rand() % 100));
	}

	//Print them
	
	printTree(ROOT);

	//Verify that a random number is in the tree
	short missing = (short)(rand() % 100);
	bool res = contains(ROOT, missing);
	printf("\nIt's %d that %i is in the tree.\n", res, missing);

	//Test frees 
	destroyTree(ROOT);
	printf("\nThe tree should be empty, nothing below.\n");
	printTree(ROOT);
}

//Walkthrough of the two frame update process
//Best to set ROW/COL CONSTANTS in both headers to 9x9 and 
// use test[5:6] (no appearing or disappearing blobs) or 
// use test[3:4] (single exit)
// use test[4:3] (single entry)
void processWalkthrough(int temp_in[][COL], float oldBT_in[][COORDDIM], short& oldNum_in, float newBT_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in, short& crossCount_in, int oldTestMat, int newTestMat)
{
	//Test getting old frame
	readTestMat(temp_in, oldTestMat); 
	singleFrame(temp_in, oldNum_in, oldBT_in);
	printBlobTable(oldBT_in, oldNum_in);

	//Get second new frame
	readTestMat(temp_in, newTestMat);
	singleFrame(temp_in, newNum_in, newBT_in);
	printBlobTable(newBT_in, newNum_in);

	//Get blob distances between both frames
	fillDist(oldBT_in, oldNum_in, newBT_in, newNum_in, dist_in, count_in);
	printDistTable(dist_in, count_in);

	//Setup the exclusion tree 
	initTree();
	matchShortest(oldBT_in, oldNum_in, newBT_in, newNum_in, dist_in, count_in);
	printTree(ROOT);

	//OLd table hasn't been resized yet so use newTable number
	printBlobTable(oldBT_in, newNum_in);
	orphanCare(oldBT_in, oldNum_in, newBT_in, newNum_in, crossCount_in);
	printBlobTable(oldBT_in, oldNum_in);
	printCrossCount(crossCount_in);

	//Determine number of entrances or exits after this update
	short dPeeps = (crossCount_in - oldNum_in) % 2;
	//Adjust the cross count
	crossCount_in += dPeeps;
}

void frameSeqTest(int seqLen_in, int startNum_in, int temp_in[][COL],  float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in, short& crossCount_in, short& numPeeps_in)
{
	for (int i = startNum_in; i < (startNum_in + seqLen_in); ++i)
	{
		readTestMat(temp_in, i);
		singleFrame(temp_in, newNum_in, newTable_in);
		numPeeps_in += updateLocs(oldTable_in, oldNum_in, newTable_in, newNum_in, dist_in, count_in, crossCount_in);
		printf("\nAfter Frame %i:", i);
		printNumPeep(numPeeps_in);
	}
}

//Gets nanosecond system time (may not be actual  time)
long get_nanos() 
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

int main()
{
	//Temp index from sensor (volatile eventually)
	int temp[ROW][COL];
	//printTestMat(temp);

	//Blob table there wil be two old and new, as well as number tracking amount
	float oldBT[BLOBLIM][COORDDIM] = { 0 };
	short oldNum = 0;

	float newBT[BLOBLIM][COORDDIM] = { 0 };
	short newNum = 0;

	//Distance matrix
	struct blob dist[(BLOBLIM * BLOBLIM)] = { 0 };
	short count = 0;

	//crossCount deals with enter/exit crossings
	short crossCount = 0;

	//this is the actually # of people in the room
	short numPeeps = 0;

	frameSeqTest(9, 0, temp, oldBT, oldNum, newBT, newNum, dist, count, crossCount, numPeeps);

	//
	//TODO: !!!!: Regular C doesn't do the generic function pointers so I cannot easily make this a function, sad bruh
	//Time Performance put code to evaluate in here, put code between start and stop. Does code 100 times and computes average ns
	/*long duration = 0;
	for (int i = 0; i < 100; ++i)
	{
		long start = get_nanos();
		readTestMat(temp, 7);
		//Time Test Start

		
		singleFrame(temp, newNum, newBT);
		updateLocs(oldBT, oldNum, newBT, newNum, dist, count, crossCount);
		printTree(ROOT);

		//Time Test End
		long stop = get_nanos();
		newNum = 0;//Have to reset numBlobs otherwise we keep adding to the same table each loop
		count = 0;
		duration += stop - start;
	}
	duration /= 100;
	printf("The Code took: %ld ns to complete", duration);

	//TODO use valgrind on slimmed version of program to get MEM performance
	*/
}
