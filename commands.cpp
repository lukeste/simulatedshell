#include "commands.h"

const cmd_hash cmd_map{
    {"cat", fn_cat},       {"cd", fn_cd},     {"echo", fn_echo},
    {"ls", fn_ls},         {"make", fn_make}, {"mkdir", fn_mkdir},
    {"prompt", fn_prompt}, {"pwd", fn_pwd},   {"rm", fn_rm},
    {"exit", fn_exit},     {"help", fn_help}, {"touch", fn_touch}};

cmd_fn find_cmd_fn(const string& cmd) {
    const auto result = cmd_map.find(cmd);
    if (result == cmd_map.end())
        throw command_error(cmd + ": no such command");
    return result->second;
}

command_error::command_error(const string& what) : runtime_error(what) {}

// -----------------------------
// Some useful helper functions
// -----------------------------
namespace {
/**
 * @brief walks down a filepath, opening each directory along the way
 *
 * @param cmd command from which the function was called
 * @param cwd pointer to the cwd
 * @param path the path to the resquested file
 * @return inode_ptr to the parent directory of the requested resource
 */
inode_ptr resolve_path(const string& cmd, inode_ptr cwd,
                       const vector<string>& path) {
    for (size_t i = 0; i < path.size() - 1; ++i) {
        try {
            cwd = cwd->get_contents()->get_dirents().at(path[i]);
        } catch (file_error&) {
            throw command_error(cmd + ": " + path[i] + ": Not a directory");
        } catch (out_of_range&) {
            throw command_error(cmd + ": " + path[i] +
                                ": No such file or directory");
        }
    }
    return cwd;
}

/**
 * @brief a print function for ls
 *
 * @param path path of directory being printed
 * @param dir reference to the directory entries
 */
void print_ls(const string& path, const ptr_map& dir) {
    if (path.size() == 0)
        cout << "/:" << endl;
    else
        cout << path << ":" << endl;
    for (const auto& file : dir) {
        cout << setw(6) << file.second->get_inode_num();
        cout << setw(6) << file.second->get_contents()->size();
        try {
            file.second->get_contents()->get_dirents();
            cout << "  " << file.first << "/" << endl;
        } catch (file_error&) {
            cout << "  " << file.first << endl;
        }
    }
}

/**
 * @brief recurses through a directory and its subdirectories, printing in
 * pre-order
 *
 * @param path path to directory
 * @param dir reference to the directory entries
 */
void ls_recurse(const string& path, const ptr_map& dir) {
    print_ls(path, dir);
    if (dir.size() == 2)
        return;
    for (const auto& node : dir) {
        try {
            node.second->get_contents()->get_dirents();
            if (node.first != "." && node.first != "..") {
                const string next_path = path + "/" + node.first;
                ls_recurse(next_path,
                           node.second->get_contents()->get_dirents());
            }
        } catch (file_error&) {
        }
    }
}

/**
 * @brief recurses through a directory in post-order, removing each subdirectory
 * and its contents
 *
 * @param dir pointer to the directory to be removed
 */
void rm_recurse(base_file_ptr dir) {
    // base case, "empty" directory
    if (dir->get_dirents().size() == 2) {
        dir->remove(dir->get_dirents().find("."), true);
        dir->remove(dir->get_dirents().find(".."), true);
        return;
    }
    // enter all directories, excluding '.' and '..'
    for (auto it = dir->get_dirents().cbegin(); it != dir->get_dirents().cend();
         ++it) {
        if (it->first != "." && it->first != "..") {
            try {
                rm_recurse(it->second->get_contents());
            } catch (file_error&) {
            }
        }
    }
    // then delete all directories and files
    for (auto it = dir->get_dirents().begin();
         it != dir->get_dirents().end();) {
        dir->remove(it++, true);
    }
}
} // namespace

// ---------------------
// Function definitions
// ---------------------
void fn_cat(inode_state& state, const vector<string>& words) {
    for (size_t i = 1; i < words.size(); ++i) {
        try {
            inode_ptr file =
                state.get_cwd()->get_contents()->get_dirents().at(words[i]);
            cout << join(file->get_contents()->readfile(), " ") << endl;
        } catch (out_of_range&) {
            throw command_error(words[0] + ": " + words[i] +
                                ": No such file or directory");
        } catch (file_error&) {
            throw command_error(words[0] + ": " + words[i] +
                                ": Is a directory");
        }
    }
}

void fn_cd(inode_state& state, const vector<string>& words) {
    if (words.size() > 2)
        throw command_error(words[0] + ": No more than one argument allowed");
    if (words.size() == 1 || words[1] == "/") {
        state.set_cwd(state.get_root());
        state.cwd_pop(true);
        return;
    }
    try {
        inode_ptr dir =
            state.get_cwd()->get_contents()->get_dirents().at(words[1]);
        state.set_cwd(dir);
        if (words[1] == "..")
            state.cwd_pop(false);
        else if (words[1] != ".") // don't push '.' to cwd
            state.cwd_push(words[1]);
    } catch (out_of_range&) {
        throw command_error(words[0] + ": " + words[1] +
                            ": No such file or directory");
    }
}

void fn_echo(inode_state&, const vector<string>& words) {
    cout << join(words.cbegin() + 1, words.cend(), " ") << endl;
}

void fn_ls(inode_state& state, const vector<string>& words) {
    vector<string> path;
    bool recur = false;
    if (words.size() == 1) {
        print_ls(state.cwd_str(),
                 state.get_cwd()->get_contents()->get_dirents());
        return;
    } else if (words.size() == 2) {
        if (words[1] == "-r")
            recur = true;
        else
            path = split(words[1], "/");
    } else if (words.size() == 3) {
        if (words[1] != "-r")
            throw command_error(words[0] + ": Usage: ls [-r] /path/to/file");
        recur = true;
        path = split(words[2], "/");
    } else {
        throw command_error(words[0] + ": Too many arguments");
    }
    if (path.size() == 0) {
        if (recur)
            ls_recurse("", state.get_root()->get_contents()->get_dirents());
        else
            print_ls(state.cwd_str(),
                     state.get_cwd()->get_contents()->get_dirents());
        return;
    }
    inode_ptr parent_dir = resolve_path("ls", state.get_cwd(), path);
    try {
        const ptr_map& dir = parent_dir->get_contents()
                                 ->get_dirents()
                                 .at(path.back())
                                 ->get_contents()
                                 ->get_dirents();
        if (recur) {
            ls_recurse(state.cwd_str() + words[2] + "/", dir);
        } else {
            print_ls(state.cwd_str() + words[1], dir);
        }
    } catch (file_error&) {
        // file is a plain_file, print out path
        if (recur)
            cout << words[2] << endl;
        else
            cout << words[1] << endl;
    } catch (out_of_range&) {
        throw command_error(words[0] + ": " + path.back() +
                            ": No such file or directory");
    }
}

void fn_make(inode_state& state, const vector<string>& words) {
    if (words.size() == 1)
        throw command_error(words[0] + ": must specify filename");
    if (words[1][0] < '.')
        throw command_error(words[0] + ": files cannot begin with \'" +
                            words[1][0] + "\'");
    inode_ptr new_file = state.get_cwd()->get_contents()->mkfile(words[1]);
    new_file->get_contents()->writefile(words);
}

void fn_mkdir(inode_state& state, const vector<string>& words) {
    if (words.size() == 1)
        throw command_error(words[0] + ": must specify directory name");
    if (words[1][0] < '.')
        throw command_error(words[0] +
                            ": directory names cannot begin with \'" +
                            words[1][0] + "\'");
    state.get_cwd()->get_contents()->mkdir(words[1]);
}

void fn_prompt(inode_state& state, const vector<string>& words) {
    string new_prompt = join(words.cbegin() + 1, words.cend(), " ") + " ";
    state.set_prompt(new_prompt);
}

void fn_pwd(inode_state& state, const vector<string>&) {
    cout << state.cwd_str() << endl;
}

void fn_rm(inode_state& state, const vector<string>& words) {
    if (words.size() == 1)
        throw command_error(words[0] + ": must specify a pathname");
    bool recur = false;
    vector<string> path;
    if (words.size() == 2) {
        path = split(words[1], "/");
    } else if (words.size() == 3) {
        if (words[1] != "-r")
            throw command_error(words[0] + ": Usage: rm [-r] /path/to/file");
        recur = true;
        path = split(words[2], "/");
    } else {
        throw command_error(words[0] + ": Too many arguments");
    }
    if (path.back() == "." || path.back() == "..")
        throw command_error(words[0] + ": \".\" and \"..\" may not be removed");
    inode_ptr curr = resolve_path("rm", state.get_cwd(), path);
    auto it = curr->get_contents()->get_dirents().find(path.back());
    if (it == curr->get_contents()->get_dirents().end()) {
        throw command_error(words[0] + ": " + path.back() +
                            ": No such file or directory");
    }
    if (recur) {
        try {
            rm_recurse(it->second->get_contents());
        } catch (file_error&) {
        }
    }
    curr->get_contents()->remove(it, recur);
}

void fn_exit(inode_state&, const vector<string>&) { throw shell_exit(); }

void fn_touch(inode_state& state, const vector<string>& words) {
    if (words.size() == 1)
        throw command_error(words[0] + ": must specify filename");
    if (words[1][0] < '.')
        throw command_error(words[0] + ": files cannot begin with \'" +
                            words[1][0] + "\'");
    inode_ptr new_file = state.get_cwd()->get_contents()->mkfile(words[1]);
}

void fn_help(inode_state&, const vector<string>&) {
    const char help_msg[] = R"(
    cat pathname            - Print the contents of one or several files
    cd [pathname]           - Change directory
    echo [text]             - Echo text
    exit                    - Exit the shell
    help                    - Print this message
    ls [-r] [pathname]      - Print the contents of a directory
    make pathname [text]    - Create a file with optional contents
    mkdir pathname          - Create a directory
    prompt text             - Change the shell prompt
    pwd                     - Print the current working directory
    rm [-r] pathname        - Remove a file or directory
    touch pathname          - Create an empty file
    )";
    cout << help_msg << endl;
}
