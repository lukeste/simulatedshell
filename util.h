#ifndef __UTIL_H__
#define __UTIL_H__

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

// split string based on given delimiter
vector<string> split(const string& line, const string& delimiter);

// join a vector<string>
const string join(const vector<string>& words, const string& delimiter);

// join a vector range
const string join(vector<string>::const_iterator first,
                  vector<string>::const_iterator last,
                  const string& delimiter);

#endif
