#include <iostream>
#include <cstring>
#include <fstream>

// Definitions
#define NAME_SIZE 25
#define FILE_SIZE		140190
#define FILE_NAME		"../names.txt"

typedef struct data {
	char** list;
	int start;
	int end;
} data;

// Prototypes
void swap(char** list, char** temp, int start, int end);
int compare(char* x, char* y);
void merge(char** list, int start, int mid, int end);
void simple_merge(data* info);
void create_list(char** start, int len, int size);
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
	// read file
	ifstream file(FILE_NAME);
	//file.open(FILE_NAME, ios::binary);
	while(file >> fname >> lname) {
			strcat(return_str[word],lname);
			strcat(return_str[word]," ");
			strcat(return_str[word],fname);
			word++;
	}
	/*
	for(int p =0; p < items;p++){
		cout << return_str[p] << endl;
	}
	*/
	return return_str;

	file.close();
}

// Actual Code
int main() {
	int len = 10000;
	char** list = get_namesf(FILE_NAME , FILE_SIZE, len);
	int size = NAME_SIZE;
	data* tinfo;
	tinfo = new data [1];
	tinfo->list = list;
	tinfo->start = 0;
	tinfo->end = len-1;
	simple_merge(tinfo);
	for(int i =0; i < len; i++) {
		std::cout << list[i] << "\n";
	}
	delete [] list;
	return 0;
}

// Function Implementations
void simple_merge(data* info) {
	if(!info) {
		return;
	}
	// one or less element
	// already sorted! return
	if(!info->list || info->start >= info->end) {
		return;
	}
	int mid = 0;
	int r_start = info->start;
	int r_end = info->end;
	int  len = r_end - info->start + 1;
	// find mid
	if(len == 2) {
		mid = info->start;
	}
	else if(len % 2 !=0) {
		mid =  info->start + (len-1)/2;
	}
	else {
		mid = info->start + len/2;
	}

	// sort left half
	info->end = mid;
	simple_merge(info);
	//sort right half
	info->start = mid+1;
	info->end = r_end;
	simple_merge(info);

	// merge halves
	merge(info->list, r_start, mid, r_end);
	return;
}


void create_list(char** start, int len, int size) {
	for(int i=0; i < len; i++) {
		start[i] = new char [size];
	}
	return;
}


void swap(char** list, char** temp, int start, int end) {
	for(int k=0, p=start; p <= end; p++, k++) {
			strcpy(list[p],temp[k]);
		}
	return;
}

int compare(char* x, char* y) {
	if (strcmp(x,y) >= 0) {
		return 0;
	}
	return -1;
}

void merge(char** list, int start, int mid, int end) {
	// if there's only one element, just return
	if(start == end) {
		return;
	}
	int len = end-start+1;
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
			strcpy(temp[k],list[i]);
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


