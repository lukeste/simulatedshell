#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <unordered_map>
using namespace std;

#include "file_sys.h"
#include "util.h"

using cmd_fn = void (*)(inode_state& state, const vector<string>& words);
using cmd_hash = unordered_map<string, cmd_fn>;

class command_error : public runtime_error {
 public:
   command_error(const string& what);
};

void fn_cat(inode_state& state, const vector<string>& words);
void fn_cd(inode_state& state, const vector<string>& words);
void fn_echo(inode_state& state, const vector<string>& words);
void fn_exit(inode_state& state, const vector<string>& words);
void fn_ls(inode_state& state, const vector<string>& words);
void fn_lsr(inode_state& state, const vector<string>& words);
void fn_make(inode_state& state, const vector<string>& words);
void fn_mkdir(inode_state& state, const vector<string>& words);
void fn_prompt(inode_state& state, const vector<string>& words);
void fn_pwd(inode_state& state, const vector<string>& words);
void fn_rm(inode_state& state, const vector<string>& words);
void fn_rmr(inode_state& state, const vector<string>& words);

cmd_fn find_cmd_fn(const string& command);

class shell_exit : public exception {};

#endif