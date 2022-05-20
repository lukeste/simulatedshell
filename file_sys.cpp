#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace std;

#include "file_sys.h"

size_t inode::next_inode_num = 1;

inode_state::inode_state() {
    root = make_shared<inode>(file_type::DIRECTORY_TYPE);
    cwd = root;
    root->contents->get_dirents().emplace(".", root);
    root->contents->get_dirents().emplace("..", root);
}

const string& inode_state::get_prompt() const { return prompt; };

void inode_state::set_prompt(const string& new_prompt) { prompt = new_prompt; }

inode_ptr inode_state::get_root() const { return root; }

inode_ptr inode_state::get_cwd() const { return cwd; }

void inode_state::set_cwd(inode_ptr new_cwd) { cwd = new_cwd; }

void inode_state::cwd_push(const string& filepath) { path.push_back(filepath); }

void inode_state::cwd_pop(bool empty) {
    if (empty)
        path.clear(); // return to root
    else
        path.pop_back(); // move up one level
}

const string inode_state::cwd_str() const {
    string full_path = "";
    for (const string& piece : path)
        full_path += "/" + piece;
    return path.size() > 0 ? "/" : full_path;
}

inode::inode(file_type type) : inode_num(next_inode_num++) {
    switch (type) {
    case file_type::PLAIN_TYPE:
        contents = make_shared<plain_file>();
        break;
    case file_type::DIRECTORY_TYPE:
        contents = make_shared<directory>();
        break;
    default:
        assert(false);
    }
}

size_t inode::get_inode_num() const { return inode_num; }

base_file_ptr inode::get_contents() const { return contents; }

file_error::file_error(const string& what) : runtime_error(what) {}

// function definitions so linker doesn't complain
const vector<string>& base_file::readfile() const {
    throw file_error("is a " + error_file_type());
}

void base_file::writefile(const vector<string>&) {
    throw file_error("is a " + error_file_type());
}

void base_file::remove(const string&) {
    throw file_error("is a " + error_file_type());
}

inode_ptr base_file::mkdir(const string&) {
    throw file_error("is a " + error_file_type());
}

inode_ptr base_file::mkfile(const string&) {
    throw file_error("is a " + error_file_type());
}

map<string, inode_ptr>& base_file::get_dirents() {
    throw file_error("is a " + error_file_type());
}

size_t plain_file::size() const {
    size_t size = 0;
    // size of all words
    for (const string& word : data)
        size += word.size();
    // plus all spaces
    if (data.size() > 0)
        size += data.size() - 1;
    return size;
}

const vector<string>& plain_file::readfile() const { return data; }

void plain_file::writefile(const vector<string>& new_data) {
    // i = 2 b/c new_data[0]==cmd, new_data[1]==filename
    for (size_t i = 2; i < new_data.size(); ++i)
        data.push_back(new_data[i]);
}

size_t directory::size() const { return dirents.size(); }

void directory::remove(const string& filename) { dirents.erase(filename); }

inode_ptr directory::mkdir(const string& dirname) {
    inode_ptr new_dir = make_shared<inode>(file_type::DIRECTORY_TYPE);
    dirents.emplace(dirname, new_dir);
    new_dir->get_contents()->get_dirents().emplace(".", new_dir);
    new_dir->get_contents()->get_dirents().emplace("..", new_dir);
    return new_dir;
}

inode_ptr directory::mkfile(const string& filename) {
    inode_ptr new_file = make_shared<inode>(file_type::PLAIN_TYPE);
    dirents.emplace(filename, new_file);
    return new_file;
}

map<string, inode_ptr>& directory::get_dirents() { return dirents; }
