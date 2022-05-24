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

// checks if s is a number
bool is_number(const string& s);

#endif
