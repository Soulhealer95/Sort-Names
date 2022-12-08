#include <iostream>

typedef struct data {
	int* list;
	int start;
	int end;
}data;

// Prototypes
void swap(int* list, int* temp, int start, int end);
int compare(int x, int y);
void merge(int* list, int start, int mid, int end);
void simple_merge(data* info);

// Actual Code
int main() {
	int* list;
	int len = 9;
	list = new int [len];
	int temp[len] = {6,4,5,12,33,22,111,0,2};
	for(int i =0; i < len; i++) {
		list[i] = temp[i];
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
	int tend = info->end;
	int  len = tend - info->start + 1;
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
	info->start = info->end+1;
	info->end = tend;
	simple_merge(info);

	// merge halves
	merge(info->list, info->start, mid, info->end);
	return;
}




void swap(int* list, int* temp, int start, int end) {
	for(int k=0, p=start; p <=end; p++, k++) {
			list[p] = temp[k];
		}
	return;
}

int compare(int x, int y) {
	if (x >= y) {
		return 0;
	}
	return -1;
}

void merge(int* list, int start, int mid, int end) {
	int len = end-start+1;
	int* temp;
	temp = new int [len];
	int k = 0;
	int i = start;
	int j = mid+1;
	for (; i <= mid && j <= end; k++){
		// if i is bigger than  = j
		if(compare(list[i], list[j]) == 0) {
			temp[k] = list[j];
			//swap(list, i, j);
			j++;
		}
		else {
			// j must be bigger than i
			temp[k] = list[i];
			//swap(list, j, i);
			i++;
		}
	}
	//put leftovers in
	while(k < len) {
		if(i <= mid) {
			temp[k] = list[i];
			i++;
		}
		else {
			temp[k] = list[j];
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


