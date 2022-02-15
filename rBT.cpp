/*EECS 300 Final Project Code Team 11: rBT.cpp
Version: 1.4  Sequence Tested
Updated: TUE15FEB22

This implements a Red-Black Tree that will be used for the block list in the frames program, 
makes checking already used blobs when picking matches or finding orphans faster

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#include "rBT.h"

//Actually initialized the globals declared in header
struct node* ROOT = NULL;
struct node* NILL = NULL;
short treeCount = 0;

//Initializes the RBT that will be used for the block list
void initTree()
{
	//Make a single NILL
	NILL = (struct node*)malloc(sizeof(struct node));
	//Make it black
	NILL->color = false;
	ROOT = NILL;
	return;
}

//Performs an internal left rotaton at the given node
void leftRot(struct node* node_in)
{
	struct node* temp;
	//Make the temp left child the right child of node_in
	temp = node_in->right;
	node_in->right = temp->left;
	if (temp->left != NILL)
	{
		temp->left->parent = node_in;
	}
	//Make the parent of node_in the parent of temp and maeke temp the child oof the parent of x
	temp->parent = node_in->parent;
	if (temp->parent == NILL)
	{
		ROOT = temp;
	}
	else if (node_in == node_in->parent->left)
	{
		node_in->parent->left = temp;
	}
	else
	{
		node_in->parent->right = temp;
	}
	//Make node_in the left child of temp and temp the parent of node_in
	temp->left = node_in;
	node_in->parent = temp;
}

//Performs an internal right rotaton at the given node
void rightRot(struct node* node_in)
{
	struct node* temp;
	//Make the right child of temp the current left child of node_in
	temp = node_in->left;
	node_in->left = temp->right;
	if (temp->right != NILL)
	{
		temp->right->parent = node_in;
	}
	//Make the parent of node_in the parent of temp, and make temp the child of node_in old parent
	temp->parent = node_in->parent;
	if (temp->parent == NILL)
	{
		ROOT = temp;
	}
	else if (node_in == node_in->parent->left)
	{
		node_in->parent->left = temp;
	}
	else
	{
		node_in->parent->right = temp;
	}
	//Make temp the parent of node_in and make node_in the child of temp
	temp->right = node_in;
	node_in->parent = temp;
}

//After a node has been naively inserted this does a bottom up fix to maintain Red-Black properties
void fixup(struct node* node_in)
{
	//Whilst the parent of node_in is red
	while (node_in->parent->color)
	{
		//If the parent of node_in is the left child of the grandparent of node_in
		if (node_in->parent == node_in->parent->parent->left)
		{
			//If if the right child of the grandparent of node_in is red
			if (node_in->parent->parent->right->color == true)
			{
				node_in->parent->color = false;
				node_in->parent->parent->right->color = false;
				node_in->parent->parent->color = true;
				node_in = node_in->parent->parent;
			}
			//If the right child of the grandparent of node_in is black
			else
			{
				//If the right child of the parent of node_in is node_in
				if (node_in == node_in->parent->right)
				{
					node_in = node_in->parent;
					leftRot(node_in);
				}
				node_in->parent->color = false;
				node_in->parent->parent->color = true;
				rightRot(node_in->parent->parent);
			}
		}
		//If the right child of the grandparent of node_in is the parent of node_in
		else
		{
			//If the the left child of the grandparent of node_in is red
			if (node_in->parent->parent->left->color) {
				node_in->parent->color = false;
				node_in->parent->parent->left->color = false;
				node_in->parent->parent->color = true;
				node_in = node_in->parent->parent;
			}
			//If the left child of he grandparent of node_in is black
			else
			{
				//If the left child of the parents of node_in is node_in
				if (node_in == node_in->parent->left)
				{
					node_in = node_in->parent;
					rightRot(node_in);
				}
				node_in->parent->color = false;
				node_in->parent->parent->color = true;
				leftRot(node_in->parent->parent);
			}
		}
	}
	ROOT->color = false;
}

//Inserts a new data into the tree
void insert(short data_in)
{
	//The new node to insert
	struct node* newNode;
	//The root
	struct node* rootNode;
	//Last non-Nill to be eligible parent
	struct node* tempNode;
	newNode = (struct node*)malloc(sizeof(struct node));
	newNode->data = data_in;
	newNode->color = true;
	newNode->left = NILL;
	newNode->right = NILL;
	rootNode = ROOT;
	tempNode = NILL;
	while (rootNode != NILL)
	{
		tempNode = rootNode;
		if (newNode->data <= rootNode->data)
		{
			rootNode = rootNode->left;
		}
		else {
			rootNode = rootNode->right;
		}
	}
	if (tempNode == NILL)
	{
		ROOT = newNode;
	}
	else if (newNode->data <= tempNode->data)
	{
		tempNode->left = newNode;
	}
	else
	{
		tempNode->right = newNode;
	}
	newNode->parent = tempNode;
	fixup(newNode);
	++treeCount;
}

//Does a recurring tree search to return a link to either Nill or the matching node
struct node* recurSearch(struct node* root_in, short key_in)
{
	//Base Case: root_in is null or key_in is present at root_in
	if (root_in == NILL || root_in->data == key_in)
		return root_in;

	//Key_in is greater than the data of root_in
	if (root_in->data < key_in)
		return recurSearch(root_in->right, key_in);

	//Key_in is smaller than the data of root_in
	return recurSearch(root_in->left, key_in);
}

//Does a recurSearch() and returns true for a match
bool contains(struct node* root_in, short key_in)
{
	struct node* found;
	found = recurSearch(root_in, key_in);
	if (found == NILL)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//Recursively goes through the tree destroying nodes
void recurDest(struct node* node_in)
{
	if (node_in == NILL)
	{
		return;
	}
	//Kill the kids
	recurDest(node_in->left);
	recurDest(node_in->right);
	//Kill yourself
	free(node_in);
}

//Frees the whole tree
void destroyTree(struct node* node_in)
{
	recurDest(node_in);
	ROOT = NILL;
	free(ROOT);
	return;
	treeCount = 0;
}
