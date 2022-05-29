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

/**
 * @brief prints the contents of one or more files
 *
 * @param words words[1..words.size()-1] should be filenames
 */
void fn_cat(inode_state& state, const vector<string>& words);

/**
 * @brief changes the current working directory
 *
 * @param words words[1] should be a valid pathname
 */
void fn_cd(inode_state& state, const vector<string>& words);

/**
 * @brief echos user input
 *
 * @param words words to be echoed
 */
void fn_echo(inode_state& state, const vector<string>& words);

/**
 * @brief prints the contents of a directory (and its subdirectories if -r is
 * present)
 *
 * @param words words[1] should be either a pathname or '-r'; if '-r' is
 * present, words[2] should be a pathname
 */
void fn_ls(inode_state& state, const vector<string>& words);

/**
 * @brief creates a text file
 *
 * @param words words[1] is the name of the file, words[2..words.size()-1] is
 * the contents of the file
 */
void fn_make(inode_state& state, const vector<string>& words);

/**
 * @brief creates a new directory
 *
 * @param words words[1] is the name of the directory
 */
void fn_mkdir(inode_state& state, const vector<string>& words);

/**
 * @brief changes the prompt of the shell
 *
 * @param words words[1..words.size()-1] is the new prompt
 */
void fn_prompt(inode_state& state, const vector<string>& words);

/**
 * @brief prints the current working directory
 *
 */
void fn_pwd(inode_state& state, const vector<string>& words);

/**
 * @brief removes a file or directory if '-r' is present
 *
 * @param words words[1] should be a pathname or '-r'; if '-r', words[2] should
 * be a pathname
 */
void fn_rm(inode_state& state, const vector<string>& words);

/**
 * @brief exits the program
 * 
 */
void fn_exit(inode_state& state, const vector<string>& words);

/**
 * @brief find the appropriate command function
 * 
 * @param command requested command
 * @return cmd_fn pointer to the command function
 */
cmd_fn find_cmd_fn(const string& command);

class shell_exit : public exception {};

#endif