/*EECS 300 Final Project Code Team 11: rBT.h
Version: 1.2  RBT Update
Updated: MON14FEB22

This is a header ofr a Red-Black Tree that will be used for the block list in the frames program,
makes checking already used blobs when picking matches or finding orphans faster

TODO: is a location I need to come back for some reason
OPTIM: is a location that is marked for potential improvement
!!!!: is a location with specific and dangerous importance
*/

#pragma once

#ifndef RBT_H
#define RBT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//Tree node structure
struct node
{
	short data;
	bool color; //RED == true, BLACK == false
	struct node* parent;
	struct node* left;
	struct node* right;
};

//!!!!: Global ROOT and unified NILL node, declared here but must be initialized in main
extern struct node* ROOT;
extern struct node* NILL;
extern short treeCount;

//Initializes the RBT that will be used for the block list
void initTree();
//Performs an internal left rotaton at the given node
void leftRot(struct node* node_in);
//Performs an internal right rotaton at the given node
void rightRot(struct node* node_in);
//After a node has been naively inserted this does a bottom up fix to maintain Red-Black properties
void fixup(struct node* node_in);
//Inserts a new data into the tree
void insert(short data_in);
//Does a recurring tree search to return a link to either Nill or the matching node
struct node* recurSearch(struct node* root_in, short key_in);
//Does a recurSearch() and returns true for a match
bool contains(struct node* root_in, short key_in);
//Recursively does through the tree destroying nodes
void recurDest(struct node* node_in);
//Frees the whole tree
void destroyTree(struct node* node_in);

#endif
