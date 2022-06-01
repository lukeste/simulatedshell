#include <cstdlib>
#include <unistd.h>

using namespace std;

#include "util.h"

vector<string> split(const string& line, const string& delimiters) {
    vector<string> words;
    size_t end = 0;

    // Loop over the string, splitting out based on delimiter
    for (;;) {
        size_t start = line.find_first_not_of(delimiters, end);
        if (start == string::npos)
            break;
        end = line.find_first_of(delimiters, start);
        words.push_back(line.substr(start, end - start));
    }
    return words;
}

const string join(const vector<string>& words, const string& delimiter) {
    string result{};
    for (auto it = words.cbegin(); it != words.cend() - 1; ++it)
        result += *it + delimiter;
    return result + *words.crbegin();
}

const string join(vector<string>::const_iterator first,
                  vector<string>::const_iterator last,
                  const string& delimiter) {
    string result{};
    for (auto it = first; it != last - 1; ++it) {
        result += *it + delimiter;
    }
    return result + *(last-1);
}

bool is_number(const string& s) {
    return !s.empty() && s.find_first_not_of("0123456789") == string::npos;
}
