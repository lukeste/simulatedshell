#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <utility>

using namespace std;

#include "commands.h"
#include "file_sys.h"
#include "util.h"

int main(int, char** argv) {
    cout << argv[0] << " build " << __DATE__ << " " << __TIME__ << endl;
    inode_state state;
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
            const vector<string> words = split(line, " \t");
            if (words.size() == 0 || words[0] == "#")
                continue;
            cmd_fn fn = find_cmd_fn(words[0]);
            fn(state, words);
        } catch (file_error& error) {
            cerr << error.what() << endl;
        } catch (command_error& error) {
            cerr << error.what() << endl;
        }
    }
    return EXIT_SUCCESS;
}
