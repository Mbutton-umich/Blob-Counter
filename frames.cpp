/*EECS 300 Final Project Code Team 11: frames.cpp
Version: 3.1 Reset Added, Now Does Sideways Shit
Updated: MAR22MAR22

Frames tracks blobs between caputred therma camera frames
All functions have been tested on 9x9 Arrays

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#include "frames.h"

//Initialize the Temp Matrix
bool temp[ROW][COL];

//Initialize the DFS Stack
coord stack[STACKMAX];
short top = -1;
//Initialize New Blob Table
struct blobElem newBT[BLOBLIM];
short newNum = 0;
//Initialize Old Blob Table
struct blobElem oldBT[BLOBLIM];
short oldNum = 0;
//Initialize the Distance Table
struct distElem distT[BLOBLIM * BLOBLIM];
short distNum = 0;
//Initialize the Cross Count
short deltaPeeps = 0;

//Converts a 1D Sensor Float Vector to 2D Boolean Array 
void DtoDD(float frame_in[STACKMAX])
{
	short k = 0;
	for (short i = 0; i < ROW; ++i)
	{
		for (short j = 0; j < COL; ++j)
		{
			temp[i][j] = (frame_in[k] > HOT);
			++k;
		}
	}
}
//Push method for DFS Stack
void push(short r_in, short c_in)
{
	//!!!!:No overflow cases because it wastes time and I know it won't happen, DFS is O(m*n) for spacec
	++top;
	stack[top] = { r_in, c_in };
}

//Pop method for the DFS Stack
coord pop()
{
	//!!!!: No overflow cases because it wastes time and I know it won't happen, DFS is O(m*n) for spacec
	--top;
	return stack[top + 1];
}

//Empty check method for DFS
bool isEmpty()
{
	if (top == -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Identifies if a potential island is valid (i.e only ones, no edges, nothing visited prior)
int validLoc(bool visited_in[][COL], short row_in, short col_in)
{
	//Check that this potential location could be part of the island we are considering
	return (row_in >= 0) && (row_in < ROW) &&
		   (col_in >= 0) && (col_in < COL) &&
	       (temp[row_in][col_in]) &&
	       (!visited_in[row_in][col_in]);
}

//Iterative Depth Fist Search
/* OPTIM: I was using a recursive DFS which was faster but it overruns the ESP32 call stack. 
/ This makes a false stack (I think in the heap), And although slower, speed testing shows only like 3 times slower...and still fast enough. Shame
*/
void IDFS(bool visited_in[][COL], struct islandLabel& islandLabel_in, const short row_in, const short col_in)
{
	//Array neighbor direction pairs for optimal looping
	static short rowInd[] = { -1, -1, -1,  0, 0,  1, 1, 1 };
	static short colInd[] = { -1,  0,  1, -1, 1, -1, 0, 1 };
	//Put the first 1 of the island into the stack
	push(row_in, col_in);
	while (!isEmpty())
	{
		//Pop the top
		struct coord current = pop();
		//Set  pixel as visited
		visited_in[current.r][current.c] = true;
		//Increase island size, add to row column sums
		++islandLabel_in.size;
		islandLabel_in.r += current.r;
		islandLabel_in.c += current.c;
		for (short k = 0; k < NEIGHNUM; ++k)
		{
			//if neightbor valid add it to the stak
			if (validLoc(visited_in, current.r + rowInd[k], current.c + colInd[k]))
			{
				push(current.r + rowInd[k], current.c + colInd[k]);
			}
		}
	}
}

//Process function for single frame
void singleFrame()
{
	newNum = 0;
	//Make visited matrix
	bool visited[ROW][COL] = { 0 };
	/*Label for DFS to report size and center r,c coordinate
	{size, row (sum/center), col (sum/center)}
	*/
	struct islandLabel freshLabel = { 0, 0, 0 };
	for (short r = 0; r < ROW; ++r)
		for (short c = 0; c < COL; ++c)
			//Loop through every pixel, if not yet visited and hot (1) then do a DFS
			if ((temp[r][c]==true) && (visited[r][c]==false))
			{
				IDFS(visited, freshLabel, r, c);
				//Increment number of blobs
				/*
				OPTIM: To descriminate based on blob size (May not be issue)
				minBlobSize = x
				if(freshLabel.size < x)
				{
				}
				*/
				++newNum;
				//Add blob location to table
				newBT[newNum - 1].r = freshLabel.r / freshLabel.size;
				newBT[newNum - 1].c = freshLabel.c / freshLabel.size;
				//Reset label to zero for next island
				freshLabel.r = 0;
				freshLabel.c = 0;
				freshLabel.size = 0;
				//If we reach our blob limit break out and just exit
				if (newNum == BLOBLIM)
				{
					return;
				}
			}
}

//Calculates distance between two points
//!!!!: This actually does distance squared for speed
float distCalc(const float r1_in, const float c1_in, const float r2_in, const float c2_in)
{
	float dr = r2_in - r1_in;
	float dc = c2_in - c1_in;
	//OPTIM: No sqrt because the numbers are small (Blob table limited to around 8)it just wastes time ....might be wrong
	return (dr * dr) + (dc * dc);
}

//Computes horizontal distance to either exit or entrance lines used in orphanCare to aid counting enters/exits
float horDistCalc(const float c1_in, const float c2_in)
{
	//OPTIM: IT gives me warning if I don't cast but casting probably wastes time
	//I realize I return a float and all reasults will be integers but all distances have been floats so to be consistent returns a float
	return fabs((double)(c1_in - c2_in));
}

//Comparator for distance matrix computation using qsort()
int distComp(const void *a,  const void *b)
{
	//qsort() must have that function header, so we have to cast these 'void pointers' into 'blob pointers' to acccess inner elements
	const float distA = ((distElem*)a)->dist;
	const float distB = ((distElem*)b)->dist;
	//A negative result means row A comes before row B
	//Explicit casr  from float to int, comparator must return an int
	return (int)(distA - distB);
}

//Finds and arranges intra-blob distance table in ascending order
void fillDist()
{
	for (int i = 0; i < oldNum; ++i)
	{
		for (int j = 0; j < newNum; ++j)
		{
			distT[distNum].dist = distCalc(oldBT[i].r, oldBT[i].c, newBT[j].r, newBT[j].c);
			distT[distNum].oldInd = i;
			distT[distNum].newInd = j;
			++distNum;
		}
	}
	//O(log(N*M)) to sort our distances for a N and M sized new and old tables
	qsort(distT, distNum, sizeof(distT[0]), distComp);
}

//Matches the new blobs with their closest old blobs and overwrites the old table
void matchShortest()
{
	/*So at this point we have sorted the distance table, now we go through and pick distances that connect a new blob with its old counterpart
	* Each time we pick a pair, we mark it as matched in the table where it came from
	*/
	short matchNum = 0;
	//The number of blobs to match, the min(old, new) blob table sizes
	short minMatch;
	if (oldNum < newNum)
	{
		minMatch = oldNum;
	}
	else
	{
		minMatch = newNum;
	}
	//Loop through the Distance Table
	for (short k = 0; k < distNum; ++k)
	{
		//If we matched the required number of blobs we are done, or if the distance goes beyond the indra blob limit 
		if ((matchNum / 2) == minMatch || distT[k].dist > INTRADISTLIM)
		{
			return;
		}
		//Check that we haven't already found a shorter distance involving this distance table pair
		if(!oldBT[distT[k].oldInd].matched && !newBT[distT[k].newInd].matched)
		{
			//Overwrite the old table with correctly matched new table r and c, 
			oldBT[distT[k].oldInd].r = newBT[distT[k].newInd].r;
			oldBT[distT[k].oldInd].c = newBT[distT[k].newInd].c;
			//Mark them in their tables as matched
			oldBT[distT[k].oldInd].matched = true;
			newBT[distT[k].newInd].matched = true;
			matchNum += 2;
		}
	}
}

//Takes care of cases when the new Blob Table is smaller or larger than the old, determines enters/exits
void orphanCare()
{
	//reset delta peeps
	deltaPeeps = 0;
	//Distances to entrance and exit lines
	float entrDist = 0;
	float exitDist = 0;
	//Old iddex for table, need this copy because I cannot overwrite it yet, more like a running copy representing the  actual old Table while it grows or shrinks
	short oldIndex = oldNum;
	//If a blob gets taken out of the table, we have to adjust what index we are calculating distance for
	short delOffset = 0;
	//Loop through the old table, the case of old blob disappearing will occur here as an unmatched entry
	for (short n = 0; n < oldNum; ++n)
	{
		//Adjusted index taking into account prior deletions that may have occured
		short adjst = n + delOffset;
		if (!oldBT[adjst].matched)
		{
			entrDist = horDistCalc(oldBT[adjst].c, 0);
			exitDist = horDistCalc(oldBT[adjst].c, COL);
			if (entrDist < exitDist)
			{
				oldBT[adjst].top = true;
				if (oldBT[adjst].top && oldBT[adjst].bottom)
				{
					--deltaPeeps;
				}
			}
			else
			{
				oldBT[adjst].bottom = true;
				if (oldBT[adjst].top && oldBT[adjst].bottom)
				{
					++deltaPeeps;
				}
			}
			//Shift out the expired blob
			--delOffset;
			for (short p = adjst; p < oldIndex; ++p)
			{
				oldBT[p].r = oldBT[p + 1].r;
				oldBT[p].c = oldBT[p + 1].c;
				oldBT[p].matched = oldBT[p + 1].matched;
				oldBT[p].top = oldBT[p + 1].top;
				oldBT[p].bottom = oldBT[p + 1].bottom;
			}
			//shorten the size of the old table
			--oldIndex;
		}
		else
		{
			//Undo matchings
			oldBT[adjst].matched = false;
		}
	}
	//Loop through the new table, the case of new blob apearing will occur here as an unmatched entry
	for (short m = 0; m < newNum; ++m)
	{
		//If something is unmatched that means we must deal with it
		if (!newBT[m].matched)
		{
			oldBT[oldIndex].r = newBT[m].r;
			oldBT[oldIndex].c = newBT[m].c;
			oldBT[oldIndex].matched = false;
			oldBT[oldIndex].top = false;
			oldBT[oldIndex].bottom = false;
			//Now comptute the enter/exit distance on the newly registered blob and increment the crossCount
			entrDist = horDistCalc(oldBT[oldIndex].c, 0);
			exitDist = horDistCalc(oldBT[oldIndex].c, COL);
			if (entrDist < exitDist)
			{
				oldBT[oldIndex].top = true;
			}
			else
			{
				oldBT[oldIndex].bottom = true;
			}
			//Extend size of the old Table
			++oldIndex;
		}
		else
		{
			//Unmatch everything in the table matched prior
			newBT[m].matched = false;
		}
	}
	//Resize the Old Table finally
	oldNum = newNum;
	return;
}

//Resets for  next frame
void reset()
{
	//Set the new table back to empty
	newNum = 0;
	//Set the  distance table back to empty
	distNum = 0;
}

//Does the comparisons between old  and new frames
short updateLocs()
{	
	//Fill the Distance Table
	fillDist();
	
	//Match old and new blobs
	matchShortest();

	//Deal with unmatched blobs
	orphanCare();

	//Clear stuff out for the next frame
	reset();

	//Return the change in room occupancy
	return deltaPeeps;
}

void zeroize()
{
	//Zeroize the temperature array and the DFS Stack
	short k = 0;
	for (short i = 0; i < ROW; ++i)
	{
		for (short j = 0; j < COL; ++j)
		{
			temp[i][j] = false;
			stack[k].r = 0;
			stack[k].c = 0;
		}
	}
	//Move top of stack back to -1
	top = -1;

	//Zeroize the new and old blob tables
	for (short l = 0; l < BLOBLIM; ++l)
	{
		//New
		newBT[l].r = 0;
		newBT[l].c = 0;
		newBT[l].matched = false;
		newBT[l].top = false;
		newBT[l].bottom = false;
		//Old
		oldBT[l].r = 0;
		oldBT[l].c = 0;
		oldBT[l].matched = false;
		oldBT[l].top = false;
		oldBT[l].bottom = false;
	}

	//Zeroize the distance table
	for (short m = 0; m < BLOBLIM * BLOBLIM; ++m)
	{
		distT[m].dist = 0;
		distT[m].oldInd = 0;
		distT[m].newInd = 0;
	}

	//Zeroize delta persons
	deltaPeeps = 0;
}
