#pragma once
#include "Utility.h"
#include <vector>
using namespace std;

void selectsort(vector<Score>& p);
void insertionsort(vector<Score>& p);
void bubblesort(vector<Score>& p);
void quicksort(vector<Score>& p, int left, int right);
void mergesort(vector<Score>& p, int left, int right);
void quicksort(vector<Score>& p);
void mergesort(vector<Score>& p);
void shellSort(vector<Score>& p);
void heapSort(vector<Score>& p);
void countingSort(vector<Score>& p, int max);
void radixSort(vector<Score>& p);
