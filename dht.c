/*
	Jake Bussa
	CS 512
	December 2, 2019
	Program that implements a dht-based system in order to resolve
	key to address lookups. Prints output that is used as input to
	dht.dgs which is then used to display what's happening.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#define PI 3.14159265359

int n = 32;
int step;

struct DistributedHashTable {

	int* nodesInChargeOf;
	int nodesInChargeOfLength;
	int* fingerTable;
	int fingerTableLength;
};

// using a circular linked list to make the ring, only nodes that are online have a dht
struct Node {

	int id;
	struct Node* next;
	int online;
	struct DistributedHashTable* dht;
};

struct Node* head = NULL;

// adds a node to the end of the list
void addNode(int id, int online) {

	struct Node* node = malloc(sizeof(struct Node));
	node->id = id;
	node->online = online;

	// this is our first node to be added to the list
	if(head == NULL) {

		head = node;
		head->next = head;
		return;
	}

	struct Node* current = head;

	// traverse until the end up the list is reached, then add the node
	while(current->next != head)
		current = current->next;

	current->next = node;

	// making the list circular
	node->next = head;
}

// once list is created, assigns the online node the list of nodes that it's in charge of
void setNodesInChargeOf(int onlineNodeId) {

	// traverse the list until the online node is reached
	struct Node* onlineNode = head;

	while(onlineNode->id != onlineNodeId)
		onlineNode = onlineNode->next;

	// now find the online node that comes before this online node to get a range of values
	struct Node* previousOnlineNode = onlineNode;
	struct Node* traversalNode = onlineNode->next;

	while(traversalNode != onlineNode) {

		if(traversalNode->online)
			previousOnlineNode = traversalNode;

		traversalNode = traversalNode->next;
	}

	// handling a very, very rare case in which there is only 1 online node (happens when n is small)
	if(onlineNode == previousOnlineNode) {

		// in this case, this single online node will be in charge of all nodes
		onlineNode->dht->nodesInChargeOf = malloc(sizeof(int) * n);

		int i;
		for(i = 0; i < n; i++)
			*(onlineNode->dht->nodesInChargeOf + i) = i;

		onlineNode->dht->nodesInChargeOfLength = n;

		// don't execute the common case below
		return;
	}

	// the common case: setting the nodes that this online node will be in charge of
	int i = 0;
	while(previousOnlineNode != onlineNode) {

		previousOnlineNode = previousOnlineNode->next;

		// the number in charge of won't be known in advance
		onlineNode->dht->nodesInChargeOf = realloc(onlineNode->dht->nodesInChargeOf, sizeof(int) * (i + 1));
		*(onlineNode->dht->nodesInChargeOf + i) = previousOnlineNode->id;

		i++;
	}

	// will need the length for later use
	onlineNode->dht->nodesInChargeOfLength = i;
}

// power function adapted to work for this program only
// was originally using pow(x, y) from <math.h>, but required gcc -Wall dht.c -o dht -lm
int power(int x, int y) {

	if(y == 0)
		x = 1;

	int i;
	for(i = 1; i < y; i++)
		x *= 2;

	return x;
}

// sets the finger tables for an online node
void setFingerTables(int onlineNodeId) {

	struct Node* onlineNode = head;

	while(onlineNode->id != onlineNodeId)
		onlineNode = onlineNode->next;

	onlineNode->dht->fingerTableLength = log(n) / log(2);
	onlineNode->dht->fingerTable = malloc(sizeof(int) * onlineNode->dht->fingerTableLength);

	int i;
	for(i = 0; i < onlineNode->dht->fingerTableLength; i++) {

		// getting the entry for this node (ie. (p + 2^i-1))
		int entry = onlineNode->id + power(2, (i + 1 - 1));

		// allow entry to make it around the ring
		if(entry >= n)
			entry %= n;

		// finding the successor to the entry (ie. succ(p + 2^i-1))
		struct Node* successor = onlineNode;
		int foundSuccessor = 0;

		// get the successor node for this finger table entry
		while(! foundSuccessor) {

			if(successor->online) {

				int j;
				for(j = 0; j < successor->dht->nodesInChargeOfLength; j++) {

					if(*(successor->dht->nodesInChargeOf + j) == entry) {

						*(onlineNode->dht->fingerTable + i) = successor->id;
						foundSuccessor = 1;
						break;
					}
				}
			}

			successor = successor->next;
		}
	}
}

struct Node* getNode(int id) {

	struct Node* current = head;

	while(current->id != id)
		current = current->next;

	return current;
}

// returns a string representation of the nodes that a node is in charge of
char* getNodesInChargeOfString(struct Node* node) {

	// this is done like this because I have no idea what I'm doing
	char* nodesInChargeOf = malloc(sizeof(char) * 200);

	// don't want ", " only happens if a node is in charge of a single element
	if(node->dht->nodesInChargeOfLength == 1)
		sprintf(nodesInChargeOf, "%d", *(node->dht->nodesInChargeOf + 0));
	else
		sprintf(nodesInChargeOf, "%d, ", *(node->dht->nodesInChargeOf + 0));

	int i;
	for(i = 1; i < node->dht->nodesInChargeOfLength; i++) {

		char temp[20];

		// don't want the extra ", " at the end
		if(i == node->dht->nodesInChargeOfLength - 1)
			sprintf(temp, "%d", *(node->dht->nodesInChargeOf + i));
		else
			sprintf(temp, "%d, ", *(node->dht->nodesInChargeOf + i));

		strncat(nodesInChargeOf, temp, 20);
	}

	return nodesInChargeOf;
}

// prints all nodes, their x and y positions, and labels for the dgs file
void printNodes() {

	// this is needed for some reason
	printf("DGS004\n");
	printf("null 0 0\n");

	// declare that this is the first step
	printf("\nst 0\n");
	step++;

	// this is an invisible node that is not apart of the program, used to display the lookups
	double halfPI = PI / 2;
	double xPos = cos(halfPI);
	printf("an %s x=%lf y=%lf label=%s\n", "lookup", xPos, 1.5, "\"Lookup Key: from Node:\"");
	printf("cn %s ui.class=invisible\n", "lookup");
	printf("an %s x=%1f y=%1f label=%s\n", "path", xPos, 1.35, "\"Path:\"");
	printf("cn %s ui.class=invisible\n", "path");

	int i;
	for(i = 0; i < n; i++) {

		// setting the x and y coords such that the nodes appear in a circle
		// based off of: https://stackoverflow.com/questions/2508704/draw-a-circle-with-a-radius-and-points-around-the-edge
		double t = 2 * PI * i / n + (PI + PI / 2);

		double x = cos(t);
		double y = -sin(t);

		// adding the node and a label
		printf("an %d x=%lf y=%lf label=%d\n", i, x, y, i);
	}
}

// these nodes are invisible and appear outside the ring, show the nodes that an online node is in charge of
void printOnlineNodes() {

	int i;
	for(i = 0; i < n; i++) {

		double t = 2 * PI * i / n + (PI + PI / 2);

		double x = cos(t);
		double y = -sin(t);

		x *= 1.25;
		y *= 1.25;

		if(getNode(i)->online) {

			printf("an on%d x=%1f y=%1f label=\"{%s}\"\n", i, x, y, getNodesInChargeOfString(getNode(i)));
			printf("cn on%d ui.class=online\n", i);

			// create a dashed edge connecting the online node and the list of nodes its in charge of
			printf("ae onedge%d on%d %d\n", i, i, i);
			printf("ce onedge%d ui.class=dash\n", i);
		}
	}
}

// resets the color of all nodes, called after a lookup finds the final node
void resetNodes() {

	// gotta print off the next step
	printf("\nst %d\n", step);
	step++;

	// note: ui.class=reset does not appear in the java program but resets anyways somehow
	int i;
	for(i = 0; i < n; i++)
		printf("cn %d ui.class=reset\n", i);
}

// clears the edges, called after a lookup
void resetEdges(int removeEdges[]) {

	// deleting edges
	int i = 0;
	while(removeEdges[i+1] != -1) {

		printf("de %d%d\n", removeEdges[i], removeEdges[i+1]);
		i++;
	}

	// reset lookup and path
	printf("cn %s label=\"Look up Key: from Node:\"\n", "lookup");
	printf("cn %s label=\"Path:\"\n", "path");
}

// helps the lookup() function by telling it if the key is within two values
// eg. key = 6 between 24 and 9 returns 1 (true) while key = 1 between 5 and 12 returns 0 (false)
int inBetween(int start, int key, int end) {

	struct Node* current = head;

	while(current->id != start)
		current = current->next;

	// set of nodes that lie between start and end, includes start but not end
	int* betweenNodes = NULL;
	int i = 0;

	while(current->id != end) {

		betweenNodes = realloc(betweenNodes, sizeof(int) * (i + 1));
		*(betweenNodes + i) = current->id;

		current = current->next;
		i++;
	}

	// now check to see if the key is in the set of between nodes
	int j;
	for(j = 0; j < i; j++)
		if(*(betweenNodes + j) == key)
			return 1;

	free(betweenNodes);

	// didn't find the key within the two values
	return 0;
}

// performs the lookup function
void lookup(int key, int start, int lastLookup) {

	// let the file know what step this is
        printf("\nst %d\n", step);
        step++;

	// save the lookup path to show to the user
	char lookupPath[300];
	sprintf(lookupPath, "%d", start);

	// tell the user what the lookup is
	printf("cn %s label=\"Look up Key: %d from Node: %d\"\n", "lookup", key, start);

	// display the lookup path
	printf("cn %s label=\"Path: %s\"\n", "path", lookupPath);

	// changing the starting node's color
	printf("cn %d ui.class=visited\n", start);

	// need an outer loop to get to next online node location
	int nextNode = start;
	int atFinalDestination = 0;

	// after the lookup, will need to remove the edges created
	int removeEdges[25];
	int removeEdgesIterator = 0;

	// processes next node until we reach the final destination
	while(! atFinalDestination) {

		int prevNode = nextNode;
		removeEdges[removeEdgesIterator] = nextNode;
		removeEdgesIterator++;

		struct Node* current = head;

		while(current->id != nextNode)
			current = current->next;

		// check to see if we're already at the destination
                int i;
                for(i = 0; i < current->dht->nodesInChargeOfLength; i++) {

			if(*(current->dht->nodesInChargeOf + i) == key) {

				printf("\nst %d\n", step);
				step++;

				// change the color to show what the final destination is
				printf("cn %d ui.class=key\n", *(current->dht->nodesInChargeOf + i));

				if(! lastLookup) {

					// resets all the nodes and edges after a lookup
					resetNodes();

					removeEdges[removeEdgesIterator] = -1;
					resetEdges(removeEdges);
				}

				atFinalDestination = 1;
                                break;
			}
                }

                if(atFinalDestination)
			break;

		int foundNext = 0;

		// checking p < k < FTp[1] go to FTp[1]
                if(inBetween(current->id, key, *current->dht->fingerTable)) {

                	nextNode = *current->dht->fingerTable;
			foundNext = 1;
                }

		// checking a rare case in which all entries in finger table are the same, next location doesn't matter
		int allSame = 1;
		for(i = 0; i < current->dht->fingerTableLength - 1; i++)
			if(*(current->dht->fingerTable + i) != *(current->dht->fingerTable + (i + 1)))
				allSame = 0;

		if(allSame) {

			nextNode = *current->dht->fingerTable;
			foundNext = 1;
		}

		int j;
                int loopy = 1;

		// checking FTp[j] <= k < FTp[j+1] go to FTp[j]
		if(! foundNext) {

			for(j = 0; j < current->dht->fingerTableLength; j++) {

				// allow [j+1] to loop to the first entry of the table
				if(loopy == current->dht->fingerTableLength)
					loopy = 0;

				if(inBetween(*(current->dht->fingerTable + j), key, *(current->dht->fingerTable + loopy))) {

					nextNode = *(current->dht->fingerTable + j);
					break;
				}

				loopy++;
			}
		}

		// change the color of the next node
		printf("\nst %d\n", step);
		step++;

		// color the visited node
                printf("cn %d ui.class=visited\n", nextNode);

		// continue to show the process of the lookup, done in dumb way
		char temp[50];
		sprintf(temp, "->%d", nextNode);
		strncat(lookupPath, temp, 50);
		printf("cn %s label=\"Path: %s\"\n", "path", lookupPath);

		// create the edge between these two nodes
		printf("ae %d%d %d>%d ui.class=path\n", prevNode, nextNode, prevNode, nextNode);
	}
}

// https://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2
int isPowerOfTwo(int value) {
	return value != 0 && (value & (value - 1)) == 0;
}

int main(int argc, char **argv) {

	// default is 32 nodes, user can change this to any power of two in the command argument
	if (argc == 2) {
		
		int value = atoi(argv[1]);
		int isValidValue = value != 0;
		int isPowerOfTwo2 = isPowerOfTwo(value);

		if (isValidValue && isPowerOfTwo2) {
			n = value;
		}
	}

	// creating each node and setting whether that node is online (has 30% chance of being online)
	srand(time(NULL));

	int numberOnline = 0;

	int i;
	for(i = 0; i < n; i++) {

		// generate a random number between 0 and 99; online = 0 (false) online = 1 (true)
		int randomNumber = rand() % 100;
		int online = 0;

		if(randomNumber < 30) {

			online = 1;
			numberOnline++;
		}

		addNode(i, online);
	}

	// rare case in which no nodes are online (only happens when n is small)
	if(numberOnline == 0) {

		printf("No nodes are online, try again\n");
		exit(0);
	}

	// after all the nodes have been created, give each online node a dht
	struct Node* current = head;

        if(current->online)
                current->dht = malloc(sizeof(struct DistributedHashTable));

        current = current->next;

        while(current != head) {

                if(current->online)
			current->dht = malloc(sizeof(struct DistributedHashTable));

                current = current->next;
        }

	// now give each online node the list of nodes that they will be in charge of
	current = head;

        if(current->online)
		setNodesInChargeOf(current->id);

        current = current->next;

        while(current != head) {

                if(current->online)
			setNodesInChargeOf(current->id);

                current = current->next;
        }

	// after that, give each online node their finger table entries
	current = head;

	if(current->online)
                setFingerTables(current->id);

        current = current->next;

        while(current != head) {

                if(current->online)
                        setFingerTables(current->id);

                current = current->next;
        }

	// printing the online nodes and their labels for the .dgs file
	printNodes();

	// makes it more clear what the online nodes are
	printOnlineNodes();

	int lastLookup = 0;

	// perform 5 random lookups
	for(i = 0; i < 5; i++) {

		if(i == 4)
			lastLookup = 1;

		int randKey = rand() % n;
		int randNode;

		// the start node needs to be online in order to perform the lookup function
		int validNode = 0;

		while(! validNode) {

			randNode = rand() % n;
			current = head;

			if(current->online && current->id == randNode)
				break;

			current = current->next;

			// check randNode to make sure it's online in order to proceed with lookup
			while(current != head) {

				if(current->online && current->id == randNode) {

					validNode = 1;
					break;
				}

				current = current->next;
			}

			if(validNode)
				break;
		}

		// finally perform the lookup function starting from an online node
		lookup(randKey, randNode, lastLookup);
	}

	// freeing the heap
	current = head;

	for(i = 0; i < n; i++) {

		//printf("Freeing node: %d\n", current->id);

		// Unfortunately, I think you need to recursively free items
		if(current->dht != NULL) {

			//printf("This node has stuff\n");
			free(current->dht->nodesInChargeOf);
			free(current->dht->fingerTable);
			free(current->dht);
		}

		head = head->next;
		free(current);
		current = head;
	}

	return 0;
}
