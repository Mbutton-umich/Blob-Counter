/*EECS 300 Final Project Code Team 11: frames.cpp
Version: 1.6 Cleaned for Speed Test
Updated: FRI19FEB22


Frames tracks blobs between caputred therma camera frames
All functions have been tested on 9x9 Arrays

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

//TODO: Refactor this to use extern globals so I'm not passing shit around and don't have mile long function parameters
//use the rBT as an example for the extern variable declare and init.


#include "frames.h"

//Initialize New Blob Table
struct blobElem newBT[BLOBLIM];
short newNum = 0;

//Initialize Old Blob Table
struct blobElem oldBT[BLOBLIM];
short oldNum = 0;

//Initialize the Distance Table
struct distElem distT[(BLOBLIM * BLOBLIM)];
short distNum = 0;

//Initialize the Cross Count
short crossNum = 0;

//Identifies if a potential island is valid (i.e only ones, no edges, nothing visited prior)
int validLoc(int temp_in[][COL], bool visited_in[][COL], short row_in, short col_in)
{
	//Check that this potential location could be part of the island we are considering
	return (row_in >= 0) && (row_in < ROW) &&
		   (col_in >= 0) && (col_in < COL) &&
	       (temp_in[row_in][col_in] > 0) &&
	       (!visited_in[row_in][col_in]);
}

//Depth first search for array islands
void DFS(int temp_in[][COL], bool visited_in[][COL], struct islandLabel& islandLabel_in,const short row_in, const short col_in)
{
	//Set current pixel as visited
	visited_in[row_in][col_in] = true;
	//Each time DFS is called island size increases, add to row column sums
	++islandLabel_in.size;
	islandLabel_in.r += row_in;
	islandLabel_in.c += col_in;
	//Array neighbor direction pairs for optimal looping
	static short rowInd[] = { -1, -1, -1,  0, 0,  1, 1, 1 };
	static short colInd[] = { -1,  0,  1, -1, 1, -1, 0, 1 };
	/*Loop recursively through each neighbor
	OPTIM: !!!!: The recursion may overfill the stack just keep in mind for large blobs make test case
	*/
	for (short k = 0; k < NEIGHNUM; ++k) 
	{
		short newRow = row_in + rowInd[k];
		short newCol = col_in + colInd[k];
		if (validLoc(temp_in, visited_in, newRow, newCol))
		{
			DFS(temp_in, visited_in, islandLabel_in, newRow, newCol);
		}
	}
}

//Process function for single frame
void singleFrame(int temp_in[][COL])
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
			if ((temp_in[r][c]==1) && (visited[r][c]==0))
			{
				DFS(temp_in, visited, freshLabel, r, c);
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

//Converts out sensed numbers to a binary array
void arr2Bin(int temp_in[][COL])
{
	//OPTIM: Could be implemented into the singleFrame loops
	for(short r = 0; r < ROW; ++r)
		for (short c = 0; c < COL; ++c)
		{
			if (temp_in[r][c] > HOTTHRESH)
			{
				temp_in[r][c] = 1;
			}
			else 
			{
				temp_in[r][c] = 0;
			}
		}
}

//Calculates distance between two points
float distCalc(const float r1_in, const float c1_in, const float r2_in, const float c2_in)
{
	float dr = r2_in - r1_in;
	float dc = c2_in - c1_in;
	//OPTIM: No sqrt because the numbers are small (Blob table limited to around 8)it just wastes time ....might be wrong
	return (dr * dr) + (dc * dc);
}

float vertDistCalc(const float r1_in, const float r2_in)
{
	return fabs((double)(r1_in - r2_in));
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
	//Distances to entrance and exit lines
	float entrDist = 0;
	float exitDist = 0;
	//Old iddex for table, need this copy because I cannot overwrite it yet, more like a running copy representing the  actual old Table while it grows or shrinks
	short oldIndex = oldNum;
	//New blobs appears, so the new Table is bigger

	//Now loop through the old table, the case of old blob disappearing ill occur here as an unmatched entry
	//If a blob gets taken out of the table, we have to adjust what index we are calculating distance for
	short delOffset = 0;
	//Loop through this larger old table
	for (short n = 0; n < oldNum; ++n)
	{
		//Adjusted index taking into account prior deletions
		short adjst = n + delOffset;
		if (!oldBT[adjst].matched)
		{
			entrDist = vertDistCalc(oldBT[adjst].r, 0);
			exitDist = vertDistCalc(oldBT[adjst].r, ROW);
			--delOffset;
			//Shift out the expired blob
			for (short p = adjst; p < oldNum; ++p)
			{
				oldBT[p].r = oldBT[p + 1].r;
				oldBT[p].c = oldBT[p + 1].c;
			}
			if (entrDist < exitDist)
			{
				--crossNum;
			}
			else
			{
				++crossNum;
			}
			//shorten the size of the old table
			//TODO can remove this?
			--oldIndex;
		}
		else
		{
			//Undo matchings
			oldBT[n].matched = false;
		}
	}
	//First loop through the new table, the case of new blob apearing will occur here as an unmatched entry
	for (short m = 0; m < newNum; ++m)
	{
		//If something is unmatched that means we must deal with it
		if (!newBT[m].matched)
		{
			oldBT[oldIndex].r = newBT[m].r;
			oldBT[oldIndex].c = newBT[m].c;
			//Now comptute the enter/exit distance on the newly registered blob and increment the crossCount
			entrDist = vertDistCalc(oldBT[oldIndex].r, 0);
			exitDist = vertDistCalc(oldBT[oldIndex].r, ROW);
			if (entrDist < exitDist)
			{
				++crossNum;
			}
			else
			{
				--crossNum;
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

void reset()
{
	//Set the new table back to empty
	newNum = 0;
	//Set the  destance table back to empty
	distNum = 0;
}

//Does the comparisons between old  and new frames
short updateLocs()
{	
	//Fill the Distance Table
	fillDist();
	
	//The block list keeps track of which nodes we already found a short distance between. Used to prevent selecting two short distances that repeated involve the same blobs
	//Must be as large as possibly two full Blob Tables to track old and new indices
	matchShortest();

	orphanCare();

	//Clear stuff out for the next frame
	reset();
	/*Actually counting exits and exits is somewhat hard to think about
	*!!!!: I don't know how robust this setup is
	*First get the sign of crossCount, this sign is basically the direction of net exits (-) and entrances (+)
	*Then we compute this term diff. Knowing that every blob we are currently tracking must have crossed one time. We subtract
	*the number of tracked blobs from the magnitude of the cross count. This term can be either negative or positve.
	*It is negative only when we have lots of blobs cancelling eachother out, so we have several people simultaneously exiting and entering
	*at one time. In the positive case that mean we had lots of crossings and are only tracking a few blobs.
	*This second case is more interesting to us because it is a roundabout way of knowing when stuff has actually left the door threshold that 
	*is to say our scan frame. If sign was the direction, than diff is the magnitude of net exits and entrances.
	*So in the negaive case we just zero it out, because there isn't enough stuff leaving the frame that we can determine is an entrance or exit
	*Once we have diff we increment or decrement the cross count based on the sign direction. We do this because the cross count is 
	*only meant to track people in the process of passing through the door, once they've made a full exit or entrance we can cut their 
	*crossings out of this running sum. Finally dPeeps is the net change to occupancy for this frame update, we take into account sign 
	*to decipher enters versus exits, and we divide by 2 because a blob must cross both the enter and exit line once to use the door.
	*/
	short sign = (crossNum < 0) ? -1 : 1;
	short diff = (sign * crossNum) - oldNum;
	diff = (diff < 0) ? 0 : diff;
	crossNum += -sign*diff;
	short dPeeps = sign*diff / 2;
	return dPeeps;
}
