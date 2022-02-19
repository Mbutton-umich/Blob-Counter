/*EECS 300 Final Project Code Team 11: testUtil.cpp
Version: 1.6 Cleaned for Speed Test
Updated: FRI19FEB22

testUtil functions as a test bench for the frames code. Checking inputs, outputs, and performance time. Also has installation calculation functions.

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "testUtil.h"

int main()
{
	//Temp index from sensor (volatile eventually)
	int temp[ROW][COL];
	//printTestMat(temp);

	//this is the actually # of people in the room
	short peepNum = 0;

	//Currently setup for test folder 1.6 where there are nine files it goes through, make sure they are in the same folder as this program running
	//frameSeqTest(8, 0, temp, peepNum);
	//printTestMats2HeaderForm(3, 0);
	installDims();
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

//Reads in test matrices from a folder named "testMats", where files are named test#.txt and testNum is #
void readTestMat(int myNums_in[][COL], int testNum_in)
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
void printTestMat(int myNums_in[][COL])
{
	printf("Printing Test Matrix Below:\n");
	for (int r = 0; r < ROW; r++)
	{
		for (int c = 0; c < COL; c++)
		{
			printf("%i ", myNums_in[r][c]);
		}
		printf("\n");
	}
	printf("\n");
		
}

//Swaps new blob table into old, useful for testing single frame
void blobSwap()
{
	for (int i = 0; i < newNum; ++i)
	{
		oldBT[i] = newBT[i];
	}
	oldNum = newNum;
	newNum = 0;
}

//Prints the Blob Table for a Single Frame
void printBlobTable(const struct blobElem blobTable_in[BLOBLIM], const short blobNum_in)
{
	printf("Printing Blob Table Below:\n");
	for (int k = 0; k < blobNum_in; ++k)
	{
		printf("Blob #%i @\nRow: %f\nCol: %f\n", k, blobTable_in[k].r, blobTable_in[k].c);
	}
	printf("\n");
}

void printDistTable()
{
	printf("Printing Distance Table Below:\n");
	for (int k = 0; k < distNum; ++k)
	{
		printf("Old#: %i  New#: %i  Dist: %f\n", distT[k].oldInd, distT[k].newInd, distT[k].dist);
	}
	printf("\n");
}

void printCrossCount()
{
	printf("Current Cross Count: %i \n", crossNum);
}

void printNumPeep(const short& peepNum_in)
{
	printf("\nThere are %i people inside the room!\n", peepNum_in);
}

//Walkthrough of the two frame update process
//Best to set ROW/COL CONSTANTS in both headers to 9x9 and 
// use test[5:6] (no appearing or disappearing blobs) or 
// use test[3:4] (single exit)
// use test[4:3] (single entry)
void processWalkthrough(int temp_in[][COL], int oldTestNum_in, int newTestNum_in)
{
	//Test getting old frame
	readTestMat(temp_in, oldTestNum_in);
	singleFrame(temp_in);
	printBlobTable(newBT, newNum);
	//Artificially move it to the old table
	blobSwap();

	//Get second new frame
	readTestMat(temp_in, newTestNum_in);
	singleFrame(temp_in);
	printBlobTable(newBT, newNum);

	//Get blob distances between both frames
	fillDist();
	printDistTable();

	//Setup the exclusion tree 
	matchShortest();

	//OLd table hasn't been resized yet so use newTable number
	printBlobTable(oldBT, newNum);
	orphanCare();
	printBlobTable(oldBT, oldNum);
	printCrossCount();

	short sign = (crossNum < 0) ? -1 : 1;
	short diff = (sign * crossNum) - oldNum;
	diff = (diff < 0) ? 0 : diff;
	crossNum += -sign * diff;
	short dPeeps = sign * diff / 2;
}

void frameSeqTest(int seqLen_in, int startNum_in, int temp_in[][COL], short& peepNum_in)
{
	for (int i = startNum_in; i < (startNum_in + seqLen_in); ++i)
	{
		readTestMat(temp_in, i);
		singleFrame(temp_in);
		peepNum_in += updateLocs();
		printf("\nAfter Frame %i:", i);
		printNumPeep(peepNum_in);
	}
}

//Takes in a sequence of test matrices and puts them into C form to put into headerfile to loaod onto the ESP32
void printTestMats2HeaderForm(int seqLen_in, int startNum_in)
{
	for (int i = startNum_in; i < seqLen_in; ++i)
	{
		char  fileName[] = ".\\testMats\\test#.txt";
		fileName[15] = '0' + i;
		FILE* fp;
		fp = fopen(fileName, "r");
		char c;
		int colNum = 1;
		int rowNum = 0;
		printf("short Test%d[ROW][COL] = {\n{", i);
		while ((c = fgetc(fp)) != EOF)
		{
			if (c != ' ' && c != '\n')
			{
				if (colNum == (COL))
				{
					if (rowNum == ROW - 1)
					{
						printf(" %c} };", c);
						break;
					}
					else
					{
						printf(" %c},\n{", c);
						colNum = 0;
						++rowNum;
					}
				}
				else
				{
					printf(" %c,", c);
				}
				++colNum;
			}
		}
		printf("\n\n");
		fclose(fp);
	}
}

//Gives you install dimensions for a given door width
void installDims()
{
	printf("Enter door width in meters: ");
	double dw;
	scanf(" %lf", &dw);
	printf("\nEnter door height in meters: ");
	double dh;
	scanf( " %lf", &dh);
	while (getchar() != '\n');
	//Aerage human height
	double ha = 1.683;
	double pi = 3.1415926535897;
	double fovRadWide = (75.0 / 180.0) * pi;
	double fovRadLong = (110.0 / 180.0) * pi;
	double halfTan = tan(fovRadWide / 2.0);
	double hl = ((dw / 2) / (tan(fovRadWide / 2.0))) + ha;
	double dd = (hl - dh) / (tan(fovRadLong / 2.0));
	printf("\n\nThe lens mount height (m) from the floor is: %f", hl);
	printf("\n\nThe lens depth offset (m) from the wall is: %f\n\n", dd);
}

//Gets nanosecond system time (may not be actual  time)
long get_nanos() 
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

