#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <utility>

using namespace std;

#include "commands.h"
#include "file_sys.h"
#include "util.h"

int main(int, char** argv) {
    cout << boolalpha; // print false or true instead of 0 or 1.
    cerr << boolalpha;
    cout << argv[0] << " build " << __DATE__ << " " << __TIME__ << endl;
    inode_state state;
    try {
        for (;;) {
            try {
                // read a line, break at EOF
                cout << state.get_prompt();
                string line;
                getline(cin, line);
                if (cin.eof()) {
                    cout << endl;
                    break;
                }

                // split the line into words and lookup the function
                vector<string> words = split(line, " \t");
                if (words.at(0) == "#")
                    continue;
                cmd_fn fn = find_cmd_fn(words.at(0));
                fn(state, words);
            } catch (file_error& error) {
                cerr << error.what() << endl;
            } catch (command_error& error) {
                cerr << error.what() << endl;
            }
        }
    } catch (shell_exit&) {
    }

    return EXIT_SUCCESS;
}
