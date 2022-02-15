/*EECS 300 Final Project Code Team 11: frames.cpp
Version: 1.2  RBT Update
Updated: MON14FEB22

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
void matchShortest(float oldTable_in[][COORDDIM], short& oldNum_in,float newTable_in[][COORDDIM], short& newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in)
{
	/*So at this point we have sorted the distance table, now we go through and pick distances that connect a new blob with its old counterpart
	* Each time we pick a pair, we put both blobs into a RBT as an exclusion list to make sure we don't select another distance involving those 
	* blobs again. Then when we go to the next shortest distance in the table we first check the tree (which is sorted and balanced by its nature)
	* and use a binary tree search to verify that this next distance doesnt involve any priorly matched blobs
	*/
	//Bool to check that a new distance involves a pair of yet unused blobs
	bool unique = true;
	//The number of blobs to match, the min(old, new) blob table sizes
	short minMatch;
	if (oldNum_in < newNum_in)
	{
		minMatch = oldNum_in;
	}
	else
	{
		minMatch = newNum_in;
	}
	for (short k = 0; k < count_in; ++k)
	{
		/*Check that we haven't already found a shorter distance involving this node
		*The excluson tree is filled somewhat awkwardly because I differentiate between old and new indices with a negative sign.
		*So this exclusion tree is the set of all blobs for which we have already found a closest partner.
		*Unfortunately due to zero indexing and there not being a -0 I have to put things into the blockList with a +1 offset
		*so for example if old blob 0 and new blob 1 are deemed closest to eachother then the tree will contain {-1 2}
		*/
		if (!(contains(ROOT, -(dist_in[k].oldInd + 1)) || contains(ROOT, dist_in[k].newInd + 1)))
		{
			unique = true;
		}
		else
		{
			unique = false;
		}
		//if we haven't already used either node we can overwrite the old table
		if (unique)
		{
			//reset the unique check
			unique = false;
			//Overwrite the old table with correctly matched new table r and c
			oldTable_in[dist_in[k].oldInd][0] = newTable_in[dist_in[k].newInd][0];
			oldTable_in[dist_in[k].oldInd][1] = newTable_in[dist_in[k].newInd][1];
			//Add newly updated blobs to the tree negating matched old values as a trick to use one tree
			//!!!!: Careful of offset here built into the block list
			insert(-(dist_in[k].oldInd + 1));
			insert(dist_in[k].newInd + 1);
			treeCount += 2;
		}
		//If we matched the required number of blobs we are done
		if ((treeCount/2) == minMatch)
		{
			break;
		}
	}
}

void orphanCare(float oldTable_in[][COORDDIM], short& oldNum_in, float newTable_in[][COORDDIM], short& newNum_in, short& crossCount_in)
{
	//Distances to entrance and exit lines
	float entrDist = 0;
	float exitDist = 0;
	//Old iddex for table, need this copy because I cannot overwrite it yet, more like a running copy representing the  actual old Table while it grows or shrinks
	short oldIndex = oldNum_in;
	//Used for indexing into partitions of exclusion RBT
	short half = treeCount/2;
	//!!!!: Dont forget the offset built into the exclusion tree data entries, emember the negatives are old table indices while postitives are new table indices.
	//New blobs appears, so the new Table is bigger
	if (oldNum_in < newNum_in)
	{
		//Loop through this larger new table
		for (short m = 0; m < newNum_in; ++m)
		{
			//Check if entry is in the tree via binary search
			short key = m + 1;
			if (!contains(ROOT, key))
			{
				insert(key);
				oldTable_in[oldIndex][0] = newTable_in[m][0];
				oldTable_in[oldIndex][1] = newTable_in[m][1];
				//Now comptute the enter/exit distance on the newly registered blob and increment the crossCount
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
	else if (oldNum_in > newNum_in)
	{
		//Loop through this larger old table
		for (short m = 0; m < oldNum_in; ++m)
		{
			//Check if entry is in the block list via tree search
			short key = -(m + 1);
			if (!contains(ROOT, key))
			{
				insert(key);
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

void reset(short& newNum_in, short& count_in)
{
	//Set the new table back to empty
	newNum_in = 0;
	//Set the  destance table back to empty
	count_in = 0;
	//reset the tree
	destroyTree(ROOT);
}

//Does the comparisons between old  and new frames
short updateLocs(float oldTable_in[][COORDDIM], short &oldNum_in, float newTable_in[][COORDDIM], short &newNum_in, struct blob dist_in[(BLOBLIM * BLOBLIM)], short& count_in,  short& crossCount_in)
{	
	fillDist(oldTable_in, oldNum_in, newTable_in, newNum_in, dist_in, count_in);
	
	//The block list keeps track of which nodes we already found a short distance between. Used to prevent selecting two short distances that repeated involve the same blobs
	//Must be as large as possibly two full Blob Tables to track old and new indices
	initTree();
	matchShortest(oldTable_in, oldNum_in, newTable_in, newNum_in, dist_in, count_in);

	orphanCare(oldTable_in, oldNum_in, newTable_in, newNum_in, crossCount_in);

	//Clear stuff out for the next frame
	reset(newNum_in, count_in);
	//Determine number of entrances or exits after this update
	
	short dPeeps = (crossCount_in - oldNum_in) / 2;
	//Adjust the cross count
	crossCount_in += 2*dPeeps;
	return dPeeps;
}
