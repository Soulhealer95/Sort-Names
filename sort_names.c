#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>

// create a lock!
pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

// Definitions
#define SIZE_NAME	15
#define FILE_SIZE	140190
//#define FILE_SIZE	40
#define ALPHABETS 	28
#define SPACE_INDEX	0
#define ALP_START	96 // start at a-1
#define FILE_NAME "./names.txt"
#define RETURN_OK	0
#define RETURN_ERROR	-1

// Let's make a dictionary!
// each node will have access to 27 possibilities = a - z and space
// and a bit to note if this is end of a word
typedef struct node {
		struct node* alphabets[ALPHABETS];
		int eow; // End of word
		int word_count;
		int visited;
} node;


char to_letter(int index) {
		if (index == 0) {
				return ' ';
		}
		else if (index < ALPHABETS) {
				return (char) index + ALP_START;
		}
		return -1;
}

int to_index(char letter) {
	if (letter == ' ') {
			return SPACE_INDEX;
	}
	return (tolower(letter) - ALP_START);

}

void set_end(node* node, int end_val) {
		// don't unset a value if it's already set
		if (node->eow == 1) {
				return;
		}
		if (end_val == 0 || end_val == 1) {
				node->eow = end_val;
		}
		return;
}

node* edit_node(node* start, int alphabet_index,  int is_end) {
		if (!start) {
				return NULL;
		}
		if (!start->alphabets[alphabet_index]) {
				if ((start->alphabets[alphabet_index] = malloc(sizeof(node))) == NULL) {
								return NULL;
					}
		}
		set_end(start->alphabets[alphabet_index], is_end);
		start->alphabets[alphabet_index]->visited++;
		return start->alphabets[alphabet_index];
}

// We can be dirty and leak memory in order to make things faster but let's set up something
// to do the cleaning anyway in case we want to use it :D
void clean_nodes(node* root) {
		if (!root) {
				return;
		}

		for(int i=0; i < ALPHABETS; i++) {
				// if node has leaves, follow them and clean them up
				if (root->alphabets[i] != NULL) {
					clean_nodes(root->alphabets[i]);
				}
		}
		// We're at the end, clean the node itself
		free(root);
		return;
}

// Get first name
void one_name(node* root, char* output, int* index) {
		if(!root) {
				return;
		}
		int i=0;
		while(i < ALPHABETS) {
				if(root->alphabets[i] != NULL) {
						if (root->alphabets[i]->visited > 0) {
								break;
						}
				}
				i++;
		}
		root->alphabets[i]->visited--;
		sprintf(&output[*index], "%c", to_letter(i));
		*index = *index+1;

		// end of the word?
		if(root->alphabets[i]->eow == 1) {
				sprintf(&output[*index], "\n");
				*index = *index+1;
				return;
		}
		//go further! 
		one_name(root->alphabets[i], output, index);
		return;
}
void populate_ds(const char* file_string, node* root, int start, int end) {

		if (root == NULL) {
				return;
		}
		node* ptr = root;
		int root_index= 0;
		int first = 0;
		int space = 0;

		// Traverse list
		// Also sorts. so technically this is how long it took to sort
		for (int i=start; i < end;i++)
		{
			if (file_string[i] == ' ') 
			{
					space = i;
			}
			else if (file_string[i] == '\n') 
			{
					//remember what letter we started from
					root_index = to_index(file_string[space+1]);

					// Last name first
					for (int j = space+1; j < i; j++) {
						ptr = edit_node(ptr, to_index(file_string[j]), 0);  
					//	printf("%c", file_string[j]);
					}

					// Then first name
					ptr = edit_node(ptr, SPACE_INDEX, 0);
					//printf(" ");
					for (int j = first; j < space; j++) {
						ptr = edit_node(ptr, to_index(file_string[j]), 0);  
					}	
					// end of word
					set_end(ptr, 1);	
					first = i+1;
					ptr = root;
					ptr->alphabets[root_index]->word_count++;
					//printf("\n");
			}
		}
		return;

}

		
int main() {
		// Reading from disk is slow. let's use the insane amount of memory we got these days and store it all in memory
		int fp = open(FILE_NAME, O_RDONLY);
		int data = 0;
		char* file_string = calloc((FILE_SIZE+1), sizeof(char));
		char* sorted_string = calloc((FILE_SIZE+1), sizeof(char));
		while((data = read(fp, file_string, sizeof(char)*FILE_SIZE)) != 0);
		// No longer need the file descriptor. 
		close(fp);
		// Now we got the whole string in memory -- should be about 0.25 seconds to print this. same as cat 

		// Empty node
		node* root = malloc(sizeof(node));
		populate_ds(file_string, root, 0, FILE_SIZE);
		int* index = malloc(sizeof(int));
		int k = 0;
		node* last;
		
		while(k < 10000) {
			one_name(root, sorted_string, index);
			k++;
		}
		printf("%s",sorted_string);

		free(file_string);
		free(sorted_string);
		clean_nodes(root);
		return RETURN_OK;
}
		
	
