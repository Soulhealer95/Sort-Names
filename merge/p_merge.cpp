/*					Parallel Merge Sort
 * Sort a list of names by last name then first name
 * Algorithm: Merge Sort
 * Fastest Speed: ~3ms @ depth = 10
 *
 * Description:
 * Pretty straight forward textbook implementation that works.
 * Doesn't print the list but can be made to.
 * Only thing it does better is when it gets data from disk, it organizes it
 * as lastname_firstname. so minor tweaks needed if they want to see it.
 * It's no different than using a struct to store it differently.
 *
 * Author:													Dated
 * Shivam S.												08-Dec-22
 * Student, McMasterU
 *
 */
#include <iostream>
#include <cstring>
#include <fstream>
#include <atomic>
#include <chrono>
#include <thread>

// Definitions
#define NAME_SIZE 25
#define FILE_SIZE		140190
#define FILE_NAME		"../names.txt"
#define MAX_DEPTH	10
#define ITEMS	10000
// This define is used mostly for debugging as it can limit items to sort
#define TEST_ITEMS	ITEMS

// Prototypes
void swap(char** list, char** temp, int start, int end);
int compare(char* x, char* y);
void merge(char** list, int start, int mid, int end);
void* simple_merge(char** list, int start, int end, std::atomic_int* depth);
void create_list(char** start, int len, int size);
char** get_namesf(const char* file_name , int file_size, int items);

// Actual Code
int main() {
	// Get some vars in the picture
	int range_end, len = ITEMS;
	std::atomic_int* depth;
	// get list of names from the file
	char** list;
	list = get_namesf(FILE_NAME , FILE_SIZE, len);
	depth = new std::atomic_int [2]();
	range_end = TEST_ITEMS-1;

	// Start Sorting!
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	simple_merge(list, 0, range_end, depth);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Sort time (sec) = " << (std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count())/1000000.0 << std::endl;

	/* print_list -- debugging only *
	for(int i =0; i < TEST_ITEMS; i++) {
		std::cout << list[i] << "\n";
	}
	//end print*/
	delete [] list;
	return 0;
}

// Function Implementations
void* simple_merge(char** list, int start, int end, std::atomic_int* depth) {
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

	if(std::atomic_load(depth) >= MAX_DEPTH) {
		// sort left half
		simple_merge(list, start, mid, depth);
		//sort right half
		simple_merge(list, mid+1, end, depth);
	}
	else {
		std::atomic_store(depth,(std::atomic_load(depth) + 1));
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
// Just some IO function. Please Free after :)
char** get_namesf(const char* file_name , int file_size, int items) {
	using namespace std;
	//allocate mem to return buffer
	char** return_str;
	return_str = new char* [items];
	create_list(return_str, items, NAME_SIZE);
	char tbuffer [NAME_SIZE];
	char fname[NAME_SIZE];
	char lname[NAME_SIZE];
	int word = 0;
	// read file, store  to list as lastname+firstname
	ifstream file(FILE_NAME);
	while(file >> fname >> lname) {
			strcat(return_str[word],lname);
			strcat(return_str[word]," ");
			strcat(return_str[word],fname);
			word++;
	}
	/* debugging purposes only
	for(int p =0; p < items;p++){
		cout << return_str[p] << endl;
	}
	*/
	return return_str;

	file.close();
}

