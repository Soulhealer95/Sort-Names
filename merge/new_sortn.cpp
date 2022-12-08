#include <iostream>
#include <cstring>

#define NAME_SIZE 22

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

// Actual Code
int main() {
	char** list;
	int len = 3;
	int size = NAME_SIZE;
	list = new char* [len];
	char* temp[len];
	create_list(temp,len,size);
	strcpy(temp[0], "Yadira Pierce");
	strcpy(temp[1], "Dennis Brennan");
	strcpy(temp[2], "Abbott Hassan");
	create_list(list,len,size);
	for(int i =0; i < len; i++) {
		strcpy(list[i],temp[i]);
		std::cout << list[i] << "\n";
	}
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
	if(len % 2 !=0) {
		mid =  info->start + (len-1)/2 - 1;
	}
	else {
		mid = info->start + len/2 - 1;
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


