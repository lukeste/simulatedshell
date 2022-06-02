#ifndef __UTIL_H__
#define __UTIL_H__

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

/**
 * @brief split a string into words, separated by some delimiter
 *
 * @param line a string to split
 * @param delimiter some delimiter(s); every individual character is considered
 * a separate delimiter
 * @return vector<string>
 */
vector<string> split(const string& line, const string& delimiter);

/**
 * @brief concatenates a vector of strings
 *
 * @param words strings to join
 * @param delimiter string between each word
 * @return const string
 */
const string join(const vector<string>& words, const string& delimiter);

/**
 * @brief concatenates a range of strings
 *
 * @param first iterator pointing to first element
 * @param last iterator pointing to past-the-end element, i.e. vector.end()
 * @param delimiter string between each word
 * @return const string
 */
const string join(vector<string>::const_iterator first,
                  vector<string>::const_iterator last, const string& delimiter);

#endif
