/*					Parallel Merge Sort
 * Sort a list of names by last name then first name
 * Algorithm: Merge Sort
 * Fastest Speed: ~1.29ms @ depth = 2 [note, this speed varies!]
 *
 * Compilation Instructions:
 * $ g++ -g -O2 p_merge.cpp -o psort -lpthread && ./psort
 * Sort time (ms) = 1.898
 *
 * Tested on x86_64 Ubuntu 18
 *
 * Description:
 * Pretty straight forward textbook implementation of merge sort that works.
 *
 * Here's the twist:
 * Thing it does better is when it gets data from disk, it organizes it
 * as lastname_firstname into a make shift hash table.
 * That's exactly the same as storing it in any other struct.
 * People optimize by splitting strings early into struct members, 
 * I decided to keep them together but place them into a table instead.
 *
 * Basically each of this table entries is indexed by alphabets
 * So last name with same starting letters are together. This is the simplest
 * and fastest hash function as it's pretty much constant time.
 *
 * Then 26 threads are created to solve roughly n/26th problem "at once"
 * each thread sorting names starting with same last name initial letter
 * and time is recorded for them to finish. 
 * 
 * Key thing is that DEPTH of each thread is simply a parallel gimmick
 * Real parallelization is the splitting of work early on
 *
 * Doesn't print the list but can be made to with DEBUG option
 *
 * Author:												Dated
 * Shivam S.											15-Dec-22
 * Student, McMasterU
 *
 */
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>

// Definitions
#define NAME_SIZE 25
#define FILE_SIZE		140190
#define FILE_NAME		"../names.txt"
#define MAX_DEPTH	2
#define ITEMS	10000
#define ALPHABET_START	'A'
#define ALPHABETS	26
// This define is used mostly for debugging as it can limit items to sort
#define TEST_ITEMS	ITEMS
#define DEBUG 0

// let's create a hash table 
typedef struct hash_t {
		char* NAMES[ITEMS];
		int count;
} hash_t;



// Prototypes
void swap(char** list, char** temp, int start, int end);
int compare(char* x, char* y);
void merge(char** list, int start, int mid, int end);
void* simple_merge(char** list, int start, int end, int depth);
void create_list(char** start, int len, int size);

// Hash table functions
int get_namesf(const char* file_name , int items, hash_t* table);
void place_in_table(const char* name, int size, hash_t* table);
void print_table(hash_t* table, int size);
void del_table(hash_t* table, int size);

// Actual Code
int main() {
	// Get some vars in the picture
	hash_t* table;
	table = new hash_t[ALPHABETS]();
	if(get_namesf(FILE_NAME, ITEMS, table) == -1) {
		return -1;
	}
	std::thread t[ALPHABETS];

	// Start Sorting!
	for(int i =0; i < ALPHABETS; i++) {
		t[i] = std::thread (simple_merge, table[i].NAMES, 0, table[i].count-1,0);
	}
	// start timing once all threads are working
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	// sorting is done when all threads are done
	for(int i =0; i < ALPHABETS; i++) {
		t[i].join();
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	// print results
	std::cout << "Sort time (ms) = " << (std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count())/1000.0 << std::endl;

	if(DEBUG) {
		print_table(table,ALPHABETS);
	}

	// cleanup
	del_table(table, ALPHABETS);
	return 0;
}

// Function Implementations
void* simple_merge(char** list, int start, int end, int depth) {
	// one or less element
	// already sorted! return
	if(!list || start >= end) {
		return NULL;
	}
	int mid = 0;
	int r_start = start;
	int r_end = end;
	int  len = r_end - start + 1;

	// find mid
	if(len == 2) {
		mid = start;
	}
	else if(len % 2 !=0) {
		mid = start + (len-1)/2;
	}
	else {
		mid = start + len/2;
	}

	if(depth >= MAX_DEPTH) {
		// sort left half
		simple_merge(list, start, mid, depth);
		//sort right half
		simple_merge(list, mid+1, end, depth);
	}
	else {
		depth++;
		// start threads
		// sort left
		std::thread left(simple_merge, list, start, mid, depth);

		// sort right half
		std::thread right(simple_merge, list, mid+1, end, depth);

		// Wait for halves to be sorted
		left.join();
		right.join();
	}

	// merge halves
	merge(list, r_start, mid, r_end);
	return NULL;
}

// provide memory to lists :)
void create_list(char** start, int len, int size) {
	for(int i=0; i < len; i++) {
		start[i] = new char [size];
	}
	return;
}

// swap one member of list with another
void swap(char** list, char** temp, int start, int end) {
	for(int k=0, p=start; p <= end; p++, k++) {
			strcpy(list[p],temp[k]);
		}
	return;
}
// basically wrapper for strcmp but the idea is to provide a wrapper so
// we can use same merge() implementation
int compare(char* x, char* y) {
	if (strcmp(x,y) >= 0) {
		return 0;
	}
	return -1;
}

// Actual magic happens here :)
// Doesn't need any syncronization for 2 reasons:
// 1) All threads work on different bit of the list
// 2) The join() in first simple_merge thread ensures the threads dont reach here
// before they have to
void merge(char** list, int start, int mid, int end) {
	// if there's only one element, just return
	if(start == end) {
		return;
	}
	int len = end-start+1;
	// allocating a temp buffer for moving things
	char** temp;
	temp = new char* [len];
	create_list(temp,len,NAME_SIZE);
	int k = 0;
	int i = start;
	int j = mid+1;
	for (; i <= mid && j <= end; k++){
		// if i is bigger than  = j
		if(compare(list[i], list[j]) == 0) {
			strcpy(temp[k],list[j]);
			j++;
		}
		else {
			// j must be bigger than i
			strcpy(temp[k],list[i]);
			i++;
		}
	}
	//put leftovers in
	while(k < len) {
		if(i <= mid) {
			strcpy(temp[k],list[i]);
			i++;
		}
		else if(j <= end) {
			strcpy(temp[k],list[j]);
			j++;
		}
		k++;
	}
	//copy this to original
	swap(list, temp, start, end);

	// remove temp
	delete[] temp;

	return;
}

// helper functions for hash table
int to_index(char character) {
		return (int) (character - ALPHABET_START);
}

// gets a first/last name and places into a hash table 
void place_in_table(const char* fname, const char* lname, int size, hash_t* table) {
		int ptr = 0;
		hash_t* t_ptr = &table[to_index(lname[0])];
		// place the name
		char* tbuffer;
		tbuffer = new char[size]();
		memset(tbuffer, '\0', size);
		strcpy(tbuffer, lname);
		strcat(tbuffer, " ");
		strcat(tbuffer,fname);	
		t_ptr->NAMES[t_ptr->count]= tbuffer;
		t_ptr->count++;

		return;
}

// Get names from file IO and print
int get_namesf(const char* file_name , int items, hash_t* table) {
	// read file, store  to list as lastname+firstname
	std::ifstream file (FILE_NAME);
	if(!file) {
			std::cout << "Error Opening File";
			return -1;
	}
	char fname[NAME_SIZE];
	char lname[NAME_SIZE];
	while(file >> fname >> lname) {
		place_in_table(fname, lname, NAME_SIZE, table);
	}
	file.close();
	return 0;

}

// iterate over names within each table entry and print
void print_table(hash_t* table, int size) {
		for(int i = 0; i < size; i++) {
				for(int j = 0; j < table[i].count; j++) {
						printf("%s\n", table[i].NAMES[j]);
				}
		}

}
// clean up allocated memory
void del_table(hash_t* table, int size) {
		for(int i = 0; i < size; i++) {
				// delete all allocated names
				for(int j = 0; j < table[i].count; j++) {
						delete []table[i].NAMES[j];
				}
		}
		delete [] table;
}
