#pragma once
/**
 * @file Utility.h
 * @brief Common Utility Functions and Constants
 * 
 * [Contents]
 * - UI output helpers
 * - Common algorithm functions (partition, etc.)
 * - Type definitions
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "Heap.hpp"
#include "Score.h"

using namespace std;

// ============================================================================
// UI Utilities (Legacy - prefer UI module)
// ============================================================================

/// Separator line string
inline const string CUTLINE = "------------------------------------";

/// Print separator line
inline void print_cutline() {
    cout << CUTLINE << endl;
}

/// Print section with title
inline void print_section(const string& title) {
    cout << "\n===== " << title << " =====" << endl;
}

// ============================================================================
// Algorithm Utilities
// ============================================================================

/**
 * @brief Hoare Partition Function
 * 
 * Partition function used in Quick Sort and Quick Select.
 * Uses median as pivot to prevent worst case.
 * 
 * @param p Vector to partition
 * @param left Start index
 * @param right End index
 * @return Partition boundary index
 * 
 * [Time Complexity] O(n)
 * [Sort Direction] Descending (larger values first)
 */
int partition_d(vector<Score>& p, int left, int right);
