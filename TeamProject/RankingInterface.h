#pragma once
#include "Utility.h"

enum class QueryType { TopK, WindowPQ, KthOnly };
enum class SortType { selectsort, insertionsort, bubblesort, quicksort, mergesort, shellSort, heapSort, countingSort, radixSort };
enum class BuildStrategy { SortAll, PartialSort, NthThenSortK, NthWindow };
enum class MaintainStrategy { None, NeighborInsertion, AvlTopRest };
