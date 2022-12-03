#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

//Defines
#define ALPHABET	97		// 'a' ASCII - start of alphabets
#define ALPHABETS	27
#define SPACE	' '
#define FILE_PATH	"./names.txt"
#define NAME_MAX	7
#define EXISTS		1
#define VISITED		2

typedef struct node {
	struct node* valid[ALPHABETS+2];
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
		(output)->end_of_line = end;
	}
	// Node should already exist, set a letter but dont change anything if end_of_line was set
	if (output->end_of_line == 0 && end == EXISTS) {
		printf("setting eOl\n");
		(output)->end_of_line = EXISTS;
	}
	// Either way, we've added to the root so let's update that
	*root_letter = *root_letter + 1;
	return output;
}

void edit_node(node* place, int letter,int* root_letter, int end) {
	if (letter < 0 || letter >= ALPHABETS+1 || root_letter == NULL) {
		printf("Couldn't edit node\n");
		return;
	}
	// Check if node already exists. set same status
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
		if (i == ALPHABETS) {
			printf(" ");
		}
		else {
			printf("%c", i + ALPHABET);
		}
		if (root->valid[i]->end_of_line == EXISTS) {
			root->valid[i]->end_of_line = VISITED;
			printf("\n");
			// restart at the beginning to print any more letters of the name
			i--;
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
		if(*(input+1) == '\0') {
			end = 1;
		}
		edit_node(ptr, index, &root->root_count, end);
		ptr = ptr->valid[index];
	}
	return;
}


// Traverse

// Create

// Delete



int main(void) {

	// Get a node
	node* root = malloc(sizeof(node));
	root->root_count = 0;
	root->end_of_line = 0;

	// Get input
	FILE* names = fopen(FILE_PATH, "r");
	if(!names) {
		printf("Couldn't open file to read!\n");
		return -1;
	}
	char temp_name[NAME_MAX] = {};
	size_t ret = fread(temp_name, sizeof(*temp_name),NAME_MAX, names);
	if (ret != NAME_MAX) {
		printf("Error reading file! - %zu\n", ret);
		return -1;
	}
	printf("Placing %s\n", temp_name);
	
	place_in_tree(temp_name, root);
	char* second = "Shiv S";
	place_in_tree(second, root);
	// Place in data structure
	print(root);
	return 0;
}
