#include "commands.h"

const cmd_hash cmd_map{
    {"cat", fn_cat},     {"cd", fn_cd},         {"echo", fn_echo},
    {"ls", fn_ls},       {"lsr", fn_lsr},       {"make", fn_make},
    {"mkdir", fn_mkdir}, {"prompt", fn_prompt}, {"pwd", fn_pwd},
    {"rm", fn_rm},       {"rmr", fn_rmr}};

cmd_fn find_cmd_fn(const string& cmd) {
    const auto result = cmd_map.find(cmd);
    if (result == cmd_map.end()) {
        throw command_error(cmd + ": no such command");
    }
    return result->second;
}

command_error::command_error(const string& what) : runtime_error(what) {}

void fn_cat(inode_state& state, const vector<string>& words) {
    for (size_t i = 1; i < words.size(); ++i) {
        try {
            inode_ptr file =
                state.get_cwd()->get_contents()->get_dirents().at(words[i]);
            vector<string> data = file->get_contents()->readfile();
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
        throw command_error(words[0] + ": No more than one operand allowed");
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

static void print_ls(const string& path, map<string, inode_ptr> dir) {
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

static void recurse(string path, map<string, inode_ptr> dir) {
    print_ls(path, dir);
    if (dir.size() == 2) {
        return;
    }
    for (const auto& node : dir) {
        try {
            node.second->get_contents()->get_dirents();
            if (node.first != "." && node.first != "..") {
                string next_path = path + "/" + node.first;
                recurse(next_path, node.second->get_contents()->get_dirents());
            }
        } catch (file_error&) {
        }
    }
}

void fn_lsr(inode_state& state, const vector<string>& words) {
    if (words.size() > 1) {
        if (words[1] == "/") {
            recurse("", state.get_root()->get_contents()->get_dirents());
        } else {
            try {
                inode_ptr dir =
                    state.get_cwd()->get_contents()->get_dirents().at(words[1]);
                recurse(state.cwd_str() + words[1],
                        dir->get_contents()->get_dirents());
            } catch (out_of_range&) {
                throw command_error(words[0] + ": cannot access " + words[1] +
                                    ": No such file or directory");
            }
        }
    } else {
        recurse("", state.get_cwd()->get_contents()->get_dirents());
    }
}

void fn_make(inode_state& state, const vector<string>& words) {
    if (words.size() == 1)
        throw command_error(words[0] + ": must specify filename");
    inode_ptr new_file = state.get_cwd()->get_contents()->mkfile(words[1]);
    new_file->get_contents()->writefile(words);
}

void fn_mkdir(inode_state& state, const vector<string>& words) {
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

void fn_rm(inode_state& state, const vector<string>& words) {
    if (words.size() == 1)
        throw command_error(words[0] + ": must specify a pathname");
    if (words.size() == 2) {
        const vector<string> path = split(words[1], "/");
        if (path[path.size() - 1] == "." || path[path.size() - 1] == "..")
            throw command_error(words[0] +
                                ": \".\" and \"..\" may not be removed");
        inode_ptr curr = state.get_cwd();
        for (size_t i = 0; i < path.size() - 1; ++i)
            curr = curr->get_contents()->get_dirents().at(path[i]);
        curr->get_contents()->remove(path[path.size() - 1], false);
    } else {
        throw command_error(words[0] + ": too many arguments");
    }
}
