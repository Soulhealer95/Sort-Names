#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//Defines
#define ALPHABET	97		// 'a' ASCII - start of alphabets
#define ALPHABETS	27
#define SPACE	' '

typedef struct node {
	struct node* valid[ALPHABETS+1];
	int end_of_line;
	int root_count; // This is clever way to support parallelization.
} node;

node* create_alphabets(node* output, int* root_letter, int end) {
	if (!output) {
		// Maybe should use calloc? How does valid[ALPHABETS] make an array with NULL pointers?
		output = malloc(sizeof(node));
		if(output == NULL) {
			printf("Not enough Memory!\n");
			return NULL;
		}
//		for (int i =0; i < ALPHABETS; i++) {
//			output->valid[i] = NULL;
//		}
		(output)->end_of_line = end;
	}
	// Node should already exist, set a letter but dont change anything if end_of_line was set
	if ((output)->end_of_line != end && end == 1) {
		(output)->end_of_line = 1;
	}
	// Either way, we've added to the root so let's update that
	*root_letter = *root_letter + 1;
	return output;
}

void edit_node(node* place, int letter,int* root_letter, int end) {
	if (letter < 0 || letter >= ALPHABETS || root_letter == NULL) {
		printf("Couldn't edit node\n");
		return;
	}
	// Check if node already exists. set same status
	printf("About to create a new node\n");
	place->valid[letter] = create_alphabets(place->valid[letter],root_letter, end);
	return;
}


void print(node* root) {
	if (!root) {
		return;
	}
	for (int i=0; i <= ALPHABETS; i++) {
		if(root->valid[i] == NULL) {
			continue;
		}
		if (i == 27) {
			printf(" ");
		}
		else {
			printf("%c", i + ALPHABET);
		}
		// Go further into this
		print(root->valid[i]);
	}
	return;
}
// Insert
void place_in_tree(char* input, node* root) {
	char lower;
	node* ptr = root;
	int index, end;
	if (input == NULL) {
		printf("Invalid input\n");
		return;
	}
	for(;*input; input++) {
		lower = tolower(*input);
		index = 0;
		end = 0;

		// convert alphabet to an acceptable index
		// 'a'- 0 to 'z'- 25
		if ((int) lower >= ALPHABET) {
		// validated the output is acceptable.
		// now place it into the structure
			index = (int) lower - ALPHABET;
			//printf("%c - %d\n",lower, (int) lower - ALPHABET);
		}
		else if (lower == SPACE) {
			index = ALPHABETS; // set space as last entry
			//printf("%c - %d\n", lower, ALPHABETS+1);
		}
		else {
			printf("Invalid characters detected. skipping - '%c'\n", lower);
			continue;
		}
		if(!(*input+1)) {
			end = 1;
		}
		// This could be done in parallel
		printf("Trying to create node for %c - %d\n", lower, index);
		edit_node(ptr, index ,&root->root_count, end);
		printf("Finished editing!\n");
		ptr = ptr->valid[index];
	}
	return;
}


// Traverse

// Create

// Delete

int main(void) {
	// Get input
	char* input = "Marie Eve Denis";
	node* root = malloc(sizeof(node));
	root->root_count = 0;
	root->end_of_line = 0;
	place_in_tree(input, root);
	// Place in data structure
	print(root);
	return 0;
}
