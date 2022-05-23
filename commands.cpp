#include "commands.h"

const cmd_hash cmd_map{{"cat", fn_cat},     {"cd", fn_cd},
                       {"echo", fn_echo},   {"ls", fn_ls},
                       {"lsr", fn_lsr},     {"make", fn_make},
                       {"mkdir", fn_mkdir}, {"prompt", fn_prompt},
                       {"pwd", fn_pwd},     {"rm", fn_rm}};

cmd_fn find_cmd_fn(const string& cmd) {
    const auto result = cmd_map.find(cmd);
    if (result == cmd_map.end())
        throw command_error(cmd + ": no such command");
    return result->second;
}

command_error::command_error(const string& what) : runtime_error(what) {}

void fn_cat(inode_state& state, const vector<string>& words) {
    for (size_t i = 1; i < words.size(); ++i) {
        try {
            inode_ptr file =
                state.get_cwd()->get_contents()->get_dirents().at(words[i]);
            const vector<string> data = file->get_contents()->readfile();
            for (auto it = data.cbegin(); it != prev(data.cend()); ++it)
                cout << *it << " ";
            cout << *data.crbegin() << endl;
        } catch (out_of_range&) {
            throw command_error(words[0] + ": " + words[i] +
                                ": No such file or directory");
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
    string space = "";
    for (auto it = words.cbegin() + 1; it != words.cend(); ++it) {
        cout << space << *it;
        space = " ";
    }
    cout << endl;
}

static void print_ls(const string& path, const ptr_map& dir) {
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

void fn_ls(inode_state& state, const vector<string>& words) {
    inode_ptr dir;
    if (words.size() > 1 && words[1] != "/") {
        try {
            dir = state.get_cwd()->get_contents()->get_dirents().at(words[1]);
        } catch (out_of_range&) {
            throw command_error(words[0] + ": cannot access " + words[1] +
                                ": No such file or directory");
        }
        if (words[1] == "." || words[1] == "..")
            print_ls(words[1], dir->get_contents()->get_dirents());
    } else if (words.size() > 1 && words[1] == "/") {
        print_ls("", state.get_root()->get_contents()->get_dirents());
    } else {
        print_ls(state.cwd_str(),
                 state.get_cwd()->get_contents()->get_dirents());
    }
}

static void ls_recurse(const string& path, const ptr_map& dir) {
    print_ls(path, dir);
    if (dir.size() == 2)
        return;
    for (const auto& node : dir) {
        try {
            node.second->get_contents()->get_dirents();
            if (node.first != "." && node.first != "..") {
                string next_path = path + "/" + node.first;
                ls_recurse(next_path,
                           node.second->get_contents()->get_dirents());
            }
        } catch (file_error&) {
        }
    }
}

void fn_lsr(inode_state& state, const vector<string>& words) {
    if (words.size() > 1) {
        if (words[1] == "/") {
            ls_recurse("", state.get_root()->get_contents()->get_dirents());
        } else {
            try {
                inode_ptr dir =
                    state.get_cwd()->get_contents()->get_dirents().at(words[1]);
                ls_recurse(state.cwd_str() + words[1],
                           dir->get_contents()->get_dirents());
            } catch (out_of_range&) {
                throw command_error(words[0] + ": cannot access " + words[1] +
                                    ": No such file or directory");
            }
        }
    } else {
        ls_recurse("", state.get_cwd()->get_contents()->get_dirents());
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
    inode_ptr new_dir = state.get_cwd()->get_contents()->mkdir(words[1]);
}

void fn_prompt(inode_state& state, const vector<string>& words) {
    string new_prompt;
    for (size_t i = 1; i < words.size(); ++i) {
        new_prompt += words[i] + " ";
    }
    state.set_prompt(new_prompt);
}

void fn_pwd(inode_state& state, const vector<string>&) {
    cout << state.cwd_str() << endl;
}

static void rm_recurse(base_file_ptr dir) {
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

void fn_rm(inode_state& state, const vector<string>& words) {
    if (words.size() == 1)
        throw command_error(words[0] + ": must specify a pathname");
    bool recur = false;
    string path = "";
    if (words.size() == 2) {
        path = words[1];
    } else if (words.size() == 3) {
        if (words[1] != "-r")
            throw command_error(words[0] + ": Usage: rm [-r] /path/to/file");
        recur = true;
        path = words[2];
    } else {
        throw command_error(words[0] + ": Too many arguments");
    }
    const vector<string> split_path = split(path, "/");
    if (split_path.back() == "." || split_path.back() == "..")
        throw command_error(words[0] + ": \".\" and \"..\" may not be removed");
    inode_ptr curr = state.get_cwd();
    for (size_t i = 0; i < split_path.size() - 1; ++i) {
        try {
            curr = curr->get_contents()->get_dirents().at(split_path[i]);
        } catch (file_error&) {
            throw command_error(words[0] + ": " + split_path[i] +
                                ": Not a directory");
        }
    }
    auto it = curr->get_contents()->get_dirents().find(split_path.back());
    if (it == curr->get_contents()->get_dirents().end()) {
        throw command_error(words[0] + ": " + split_path.back() +
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
