/*EECS 300 Final Project Code Team 11: frames.cpp
Version: 1.0  Beta
Updated: FRI11FEB22

Frames tracks blobs between caputred therma camera frames
All functions have been tested on 9x9 Arrays

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#include "frames.h"

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
void DFS(int temp_in[][COL], bool visited_in[][COL], float blobLabel_in[LABELLEN], short row_in, short col_in)
{
	//Set current pixel as visited
	visited_in[row_in][col_in] = true;

	//Each time DFS is called island size increases, add to row column sums
	++blobLabel_in[0];
	blobLabel_in[1] += row_in;
	blobLabel_in[2] += col_in;

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
			DFS(temp_in, visited_in, blobLabel_in, newRow, newCol);
		}
	}
}

//Process function for single frame
void singleFrame(int temp_in[][COL], short &numBlobs_in, float blobTable_in[][COORDDIM])
{
	numBlobs_in = 0;
	//Make visited matrix
	bool visited[ROW][COL] = { 0 };

	/*Label for DFS to report size and center r,c coordinate
	{size, row (sum/center), col (sum/center)}
	*/
	float blobLabel[LABELLEN] = { 0 };
	for (short r = 0; r < ROW; ++r)
		for (short c = 0; c < COL; ++c)
			//Loop through every pixel, if not yet visited and hot (1) then do a DFS
			if ((temp_in[r][c]==1) && (visited[r][c]==0))
			{
				DFS(temp_in, visited, blobLabel, r, c);
				//Increment number of blobs
				/*
				OPTIM: To descriminate based on blob size (May not be issue)
				minBlobSize = x
				if(blobLabel[0] < x)
				{

				}
				*/
				++numBlobs_in;
				//Add blob location to table
				blobTable_in[numBlobs_in - 1][0] = blobLabel[1] / blobLabel[0];
				blobTable_in[numBlobs_in - 1][1] = blobLabel[2] / blobLabel[0];
				//Reset label to zero for next island
				for (short k = 0; k < LABELLEN; ++k)
				{
					blobLabel[k] = 0;
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
float distCalc(float r1_in, float c1_in, float r2_in, float c2_in)
{
	float dr = r2_in - r1_in;
	float dc = c2_in - c1_in;
	//OPTIM: No sqrt because the numbers are small (Blob table limited to around 8)it just wastes time ....might be wrong
	return (dr * dr) + (dc * dc);
}

float vertDistCalc(float r1_in, float r2_in)
{
	return fabs(r1_in - r2_in);
}

//Comparator for distance matrix computation using qsort()
int distComp(const void *a,  const void *b)
{
	//qsort() must have that function header, so we have to cast these 'void pointers' into 'blob pointers' to acccess inner elements
	const float distA = ((blob*)a)->dist;
	const float distB = ((blob*)b)->dist;
	//A negative result means row A comes before row B
	//!!!!: Implicit conversion from float to int must not change sign here, there is a warning!
	return distA - distB;
}

//Comparator for qsort() for blockList
int shortComp(const void* a, const void* b)
{
	const short aVal = *(short*)a;
	const short bVal = *(short*)b;
	return aVal - bVal;
}

//Finds and arranges intra-blob distance table in ascending order
void fillDist(float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in)
{
	for (int i = 0; i < oldNum_in; ++i)
	{
		for (int j = 0; j < newNum_in; ++j)
		{
			dist_in[count_in].dist = distCalc(oldTable_in[i][0], oldTable_in[i][1], newTable_in[j][0], newTable_in[j][1]);
			dist_in[count_in].oldInd = i;
			dist_in[count_in].newInd = j;
			++count_in;
		}
	}
	//O(log(N*M)) to sort our distances for a N and M sized new and old tables
	qsort(dist_in, count_in, sizeof(dist_in[0]), distComp);
}

//Matches the new blobs with their closest old blobs and overwrites the old table
void matchShortest(float oldTable_in[][COORDDIM], short& oldNum_in,float newTable_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in, short blockList_in[BLOBLIM], short& taken_in)
{
	/*TODO: OPTIM: !!!!: This is basically nearest neighbor, so O(N^2) where N is the size of the distance matrix...so O((NxM)^2) where N,M are the blob matrices
	so worst case O(BLOBLIM^4)...FUCKING WRETCHED. I've tried looking up D&C for finding closest unique pairs of two sets, only seeing overly complex KD-Trees and Voronoi Diagrams
	open to ideas on optimizing this luckily BLOBLIMIT is small
	*/
	bool unique = true;
	int minMatch;
	//Find the minimum number of values to match
	if (shortComp(&newNum_in, &oldNum_in) <= 0)
	{
		minMatch = newNum_in;
	}
	else
	{
		minMatch = oldNum_in;
	}
	for (short k = 0; k < count_in; ++k)
	{
		for (short l = 0; l < taken_in; ++l)
		{
			/*Check that we haven't already found a shorter distance involving this node
			The block list is filled somewhat awkwardly because I differentiate between old and new indices with a negative sign.
			So this block list is the set of all blobs for which we have already found a closest partner.
			Unfortunately due to zero indexing and there not being a -0 I have to put things into the blockList with a +1 offset 
			so for example if old blob 0 and new blob 1 are deemed closest to eachother then the block list will contain {-1 2}
			*/
			if (!(dist_in[k].oldInd + 1 == -blockList_in[l] || dist_in[k].newInd + 1 == blockList_in[l]))
			{
				unique = true;
			}
			else
			{
				unique = false;
			}
		}
		//if we haven't already used either node we can overwrite the old table
		if (unique)
		{
			//reset the unique check
			unique = false;
			//Overwrite the old table with correctly matched new table r and c
			oldTable_in[dist_in[k].oldInd][0] = newTable_in[dist_in[k].newInd][0];
			oldTable_in[dist_in[k].oldInd][1] = newTable_in[dist_in[k].newInd][1];
			//add newly taken nodes to the blocklist negate matched old values as a trick to use one array to track used nodes
			//!!!!: Careful of offset here built into the block list
			blockList_in[taken_in] = -(dist_in[k].oldInd + 1);
			blockList_in[taken_in + 1] = (dist_in[k].newInd + 1);
			taken_in += 2;
		}
		//If the number of new points has been matched we are done
		if ((taken_in/2) == minMatch)
		{
			break;
		}
	}
}

void orphanCare(float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, short blockList_in[BLOBLIM], short& taken_in, short& crossCount_in)
{
	//Distances to entrane and exit lines
	float entrDist = 0;
	float exitDist = 0;
	//Old imdex for table, need this copy because I cannot overwrite it yet, more like a running copy representing the  actual old Table while it grows or shrinks
	short oldIndex = oldNum_in;
	//Used for indexing into partitions of block list
	short half = taken_in/2;

	/*Sort the block list, remember the negatives are old table indices while postitives are new table indices. This simplifies finding the unmatched blobs 
	!!!Dont forget the offset built into the block list
	*/
	qsort(blockList_in, taken_in, sizeof(blockList_in[0]), shortComp);

	//New blobs appears, so the new Table is bigger
	if (oldNum_in < newNum_in)
	{
		//Loop through this larger new table
		for (short m = 0; m < newNum_in; ++m)
		{
			//Check if entry is in the block list via binary search only need to search top half of array
			short* result;
			short key = m + 1;
			//!!!!: Visual Studio doesn't like the pointer math
			result = (short*)bsearch(&key, blockList_in + half, taken_in, sizeof(blockList_in[0]), shortComp);
			if (result == NULL)
			{
				oldTable_in[oldIndex][0] = newTable_in[m][0];
				oldTable_in[oldIndex][1] = newTable_in[m][1];
				//Noew comptute the enter/exit distance on the newly registered blob and increment the crossCount
				entrDist = vertDistCalc(oldTable_in[oldIndex][0], 0);
				exitDist = vertDistCalc(oldTable_in[oldIndex][0], 31);
				if (entrDist <= exitDist)
				{
					++crossCount_in;
				}
				else
				{
					--crossCount_in;
				}
				//Extend size of the old Table
				++oldIndex;
			}
		}
	}
	//Old blobs disappeared
	if (oldNum_in > newNum_in)
	{
		//Loop through this larger old table
		for (short m = 0; m < oldNum_in; ++m)
		{
			//Check if entry is in the block list via binary search only need to search bottom half of array, negtive entries
			short* result;
			short key = -(m + 1);
			//!!!!: Visual Studio doesn't like the pointer math
			result = (short*)bsearch(&key, blockList_in, half, sizeof(blockList_in[0]), shortComp);
			if (result == NULL)
			{
				entrDist = vertDistCalc(oldTable_in[m][0], 0);
				exitDist = vertDistCalc(oldTable_in[m][0], 31);
				//Shift out the expired blob
				for (short p = m; p < oldNum_in; ++p)
				{
					oldTable_in[p][0] = oldTable_in[p + 1][0];
					oldTable_in[p][1] = oldTable_in[p + 1][1];
				}

				//Invert the crossCount increments
				if (entrDist <= exitDist)
				{
					--crossCount_in;
				}
				else
				{
					++crossCount_in;
				}
				//shorten the size of the old table
				--oldIndex;
			}
		}
	}
	else
	{
		return;
	}
	//Resize the oldTable finally
	oldNum_in = newNum_in;
	return;
}

//Does the comparisons between old  and new frames
short updateLocs(float oldTable_in[][COORDDIM], short &oldNum_in, float newTable_in[][COORDDIM], short &newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in,  short& crossCount_in)
{	
	fillDist(oldTable_in, oldNum_in, newTable_in, newNum_in, dist_in, count_in);
	
	//The block list keeps track of which nodes we already found a short distance between. Used to prevent selecting two short distances that repeated involve the same blobs
	//Must be as large as possibly two full Blob Tables to track old and new indices
	short blockList[2 * BLOBLIM] = { 0 };
	//Taken tracks the block list size
	short taken = 0;
	matchShortest(oldTable_in, oldNum_in, newTable_in, newNum_in, dist_in, count_in, blockList, taken);

	orphanCare(oldTable_in, oldNum_in, newTable_in, newNum_in, blockList, taken, crossCount_in);

	//Determine number of entrances or exits after this update
	short dPeeps = (crossCount_in - oldNum_in) / 2;
	//Adjust the cross count
	crossCount_in += 2*dPeeps;
	return dPeeps;
}
