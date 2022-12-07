#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>


// Definitions
#define FILE_SIZE		140190
#define FILE_NAME 		"./names.txt"
//Dictionary defines
#define ALPHABETS 		28
#define SPACE_INDEX		0
#define ALP_START		96 // start at a-1
//Threading
#define THREAD_COUNT	1
#define RETURN_OK		0
#define RETURN_ERROR	-1
//Enable Nested Threading
#define NESTED			0

// create a lock!
//pthread_mutex_t main_mutex[ALPHABETS] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t main_mutex[ALPHABETS] = {};
pthread_mutex_t count_md = PTHREAD_MUTEX_INITIALIZER;

// Let's make a dictionary!
// each node will have access to 27 possibilities = a - z and space
// and a bit to note if this is end of a word
typedef struct node {
		struct node* alphabets[ALPHABETS];
		int eow; // End of word
		pthread_mutex_t node_mtx;
		int visited;
} node;

typedef struct word_info {
		const char* format;
		node* ptr;
		int start;
		int mid;
		int end;
} winfo;
typedef struct main_args {
		const char* format;
		node* root;
		int start;
		int end;
		int thread;
}m_args;

// Convert back and froth from our make believe indices of alphabets
char to_letter(int index) {
		if (index == 0) {
				return ' ';
		}
		else if (index < ALPHABETS) {
				return (char) index + ALP_START;
		}
		return RETURN_ERROR;
}

int to_index(char letter) {
	if (letter == ' ') {
			return SPACE_INDEX;
	}
	return (tolower(letter) - ALP_START);

}

// Functions to Manipulate data structure
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

		pthread_mutex_lock(&main_mutex[alphabet_index]);
		//pthread_mutex_lock(&count_md);
		if (!start->alphabets[alphabet_index]) {
				if ((start->alphabets[alphabet_index] = malloc(sizeof(node))) == NULL) {
					pthread_mutex_unlock(&main_mutex[alphabet_index]);
								return NULL;
					}
		}
		pthread_mutex_unlock(&main_mutex[alphabet_index]);
		//pthread_mutex_unlock(&count_md);
		if(is_end && start->alphabets[alphabet_index]->eow != 1) {
				start->alphabets[alphabet_index]->eow = is_end;
		}
		//set_end(start->alphabets[alphabet_index], is_end);
		start->alphabets[alphabet_index]->visited++;
		pthread_mutex_init(&start->alphabets[alphabet_index]->node_mtx, NULL);
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

// Get first name to print to a string
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
		//pthread_mutex_lock(&main_mutex[i]);
		root->alphabets[i]->visited--;
		//pthread_mutex_unlock(&main_mutex[i]);
		//pthread_mutex_unlock(&root->alphabets[i]->node_mtx);
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


//  return next whole word index 
int next_whole_word(const char* string, int test_index) {
		//we know there was a word at this index somewhere, let's find it.
		int k = test_index;
		if(k < 0) {
				return RETURN_ERROR;
		}
		if (k > FILE_SIZE) {
				return FILE_SIZE;
		}
		while(string[k] != '\n') {
				if(k > FILE_SIZE) {
						return FILE_SIZE;
				}
				k++;
		}
		// now k contains index of where next word ends
		return k;
}

// Put a word in the structure :)
// We're only locking on the first letter of last name so multiple threads can run
// The more scrambled the list, the better 
void populate_word(winfo* point) {
	 const char*file_string = point->format;
	 node* ptr = point->ptr;
	 int start = point->start;
	 int mid = point->mid;
	 int end = point->end;
	 int is_end = 0;
	 //int md_index = to_index(mid+1);
	if (!ptr) {
			printf("populate_word: Not doing anything! ptr == NULL\n");
			return;
	}
	//pthread_mutex_lock(&main_mutex[md_index]);
	// Last name first
	for (int j = mid+1; j < end; j++) {
		//pthread_mutex_lock(&ptr->node_mtx);
		ptr = edit_node(ptr, to_index(file_string[j]), 0);  
		//pthread_mutex_unlock(&main_mutex[md_index]);
		//pthread_mutex_lock(&ptr->node_mtx);
		//pthread_mutex_unlock(&ptr->node_mtx);
	}

	// Then ' ' + first name
	//pthread_mutex_lock(&ptr->node_mtx);
	ptr = edit_node(ptr, SPACE_INDEX, 0);
	//pthread_mutex_unlock(&ptr->node_mtx);
	for (int j = start; j < mid; j++) {
		if (j == mid-1) {
				is_end = 1;
		}
		//pthread_mutex_lock(&ptr->node_mtx);
		ptr = edit_node(ptr, to_index(file_string[j]), 0);  
		//pthread_mutex_unlock(&ptr->node_mtx);
	}	
	//pthread_mutex_unlock(&main_mutex[to_index(mid+1)]);
	return;

}

// Populate Data Structure. 
// Creates a Tree which automagically sorts as it's populated
//void populate_ds(const char* file_string, node* root, int start, int end, int use_threads) {
void populate_ds(m_args* arg) {
		const char* file_string = arg->format;
		node* root = arg->root;
		int start = arg->start;
		int end = arg->end;
		int use_threads = arg->thread;

		if (root == NULL) {
				printf("populate_ds: Not doing anything! ptr == NULL\n");
				return;
		}
		if(end > FILE_SIZE) {
				end = FILE_SIZE;
		}
		node* ptr = root;
		int root_index= 0;
		int first = 0;
		int space = 0;
		int start_word = 1;

		//init thread 
		int depth = THREAD_COUNT;
		pthread_t threads[THREAD_COUNT];
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		winfo* container = malloc(sizeof(winfo));

	

		// Traverse list
		// Also sorts. so technically this is how long it took to sort
		for (int i=start; i < end;i++)
		{
			// if found in middle of a word, just skip it and let someone else do this.
			if (start_word ==1 && i !=0) {
				while(i < end && file_string[i] != '\n') {
						i++;
				}
				// too far!
				if(i == end-1) {
						break;
				}
				//go past one more word :)
				i++;
			}
			start_word = 0;

			// fun stuff. mostly linear filling data structure
			if (file_string[i] == ' ') 
			{
					space = i;
			}
			else if (file_string[i] == '\n') 
			{
					//remember what letter we started from
					root_index = to_index(file_string[space+1]);
					//creates a container to nicely package args for threading
					//can probably be avoided
					container->format = file_string;
					container->ptr = ptr;
					container->start = first;
					container->mid = space;
					container->end = i;
					if (use_threads == 0 || depth <= 0) {
						populate_word(container);
						// This isn't the best in case of large number of threads but
						// we assume that by the time last thread finishes, all threads will be done
						// This is dangerous assumption because last thread may get to write a word
						// without blocking whereas other threads might be blocked
						// Maybe only increase depth by 1 and then check again?
						// TODO
						if (use_threads !=0) {
								if(pthread_join(threads[THREAD_COUNT-1], NULL)){
									depth = THREAD_COUNT;
								}
						}
					}
					else {
						pthread_create(&threads[THREAD_COUNT-depth],&attr, (void*)&populate_word,(winfo*) container);
						depth--;
					}
					first = i+1;
					ptr = root;
			}
		}
		//finally done going through the string!
		//wait for threads to finish
		if (use_threads) {
				for(int k=0; k< THREAD_COUNT;k++) {
						pthread_join(threads[k], NULL);
				}
		}
		free(container);
		
		return;

}

		
int main() {
		// Reading from disk is slow. let's use the insane amount of memory we got these days and store it all in memory
		int fp = open(FILE_NAME, O_RDONLY);
		if(!fp) {
				return RETURN_ERROR;
		}
		int data = 0;
		char* file_string = calloc((FILE_SIZE+1), sizeof(char));
		// only used for printing, can be avoided
		char* sorted_string = calloc((FILE_SIZE+1), sizeof(char));
		// Used later for segmenting task
		int end_p = (int) FILE_SIZE/THREAD_COUNT;
		// Open disk to read
		while((data = read(fp, file_string, sizeof(char)*FILE_SIZE)) != 0);
		// No longer need the file descriptor. 
		close(fp);

		// Now we got the whole string in memory -- should be about 0.25 seconds to print this. same as cat 
		// but let's not print it at all :)
		//threading!
		pthread_mutexattr_t attr2;
		// if owner dies, unlock it
		pthread_mutexattr_setrobust(&attr2, PTHREAD_MUTEX_ROBUST);
		for (int i=0; i<ALPHABETS;i++) {
				pthread_mutex_init(&main_mutex[i], &attr2);
		}

		// Empty node
		node* root = malloc(sizeof(node));

		// More threads!
		pthread_t threads[THREAD_COUNT];
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		int tused = 1;
		int new_ends[THREAD_COUNT+1] = {};

		m_args* container = malloc(sizeof(m_args));
		container->format = file_string;
		container->root = root;
		container->start = 0;
		container->end = next_whole_word(file_string,(int) end_p);
		if(container->end == RETURN_ERROR) {
				printf("Whoops!\n");
				return RETURN_ERROR;
		}
		//pre calculate offsets
		new_ends[0] = container->end;
		for (int i=1; i< THREAD_COUNT;i++) {
				new_ends[i] = next_whole_word(file_string, (int) end_p + new_ends[i-1]);
		}
		printf("thread %d got %d-%d\n",tused,container->start, container->end);
		container->thread = NESTED;
		

		float start_time = (float)clock()/CLOCKS_PER_SEC;
		if(THREAD_COUNT == 1) {
			populate_ds(container);
		}
		else {
				for (int i=0; i < THREAD_COUNT; i++) {
						pthread_create(&threads[i],&attr,(void*) &populate_ds, container);
						// This makes sure each thread gets a good chunk of data but doesnt go over buffer
						container->start = container->end + 1;
						container->end = new_ends[i+1];
						printf("thread %d will get %d-%d\n",tused+1,container->start, container->end);

						// last thread should have enough 
						if(container->start > FILE_SIZE) {
							printf("start out of bounds\n");
							break;
						}
						// past the buffer. correct
						if (container->end >= FILE_SIZE) {
							container->end = FILE_SIZE;
							printf("end out of bounds\n");
						}
						else if (container->end == RETURN_ERROR) {
								// shouldn't get here. 
								printf("ended in error\n");
								break;
						}
						tused++;
				}	
				for(int i=0; i< tused;i++) {
						pthread_join(threads[i], NULL);
				}
		}

		float end_time = (float)clock()/CLOCKS_PER_SEC;
		printf("Sort time: %f seconds\n", (end_time-start_time));
		if(root == NULL) {
				printf("Nothing!\n");
		}
		else {
				if(root->alphabets[1]) {
						printf("first visit: %d", root->alphabets[1]->visited);
				}
		}
		/*
	 	int* index = malloc(sizeof(int));
		*index = 0;*/
		int k = 0;
		for(int i=1;i<ALPHABETS;i++) {
				if (root->alphabets[i] != NULL) {
						k = k + root->alphabets[i]->visited;
				}
		}
				
		printf("Visited: %d\n", k);
	/*	
		while(k < 10000) {
			one_name(root, sorted_string, index);
			k++;
		}
		printf("%s",sorted_string);
	//	*/

		free(file_string);
		free(sorted_string);
		free(container);
//		clean_nodes(root);
		return RETURN_OK;
}
		

