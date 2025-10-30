#pragma once

#include "Utility.h"

Score sequentialSelect(vector <Score>& p, Heap& topk, int top);
Score quickselect(vector<Score>& p, int top);
Score binaryselect(vector<Score>& p, int top);