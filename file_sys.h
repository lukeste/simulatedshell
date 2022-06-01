#ifndef __FILE_SYS_H__
#define __FILE_SYS_H__

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "util.h"

using namespace std;

enum class file_type { PLAIN_TYPE, DIRECTORY_TYPE };
class inode;
class base_file;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;
using base_file_ptr = shared_ptr<base_file>;
using ptr_map = map<string, inode_ptr>;

class inode_state {
  private:
    inode_ptr root{nullptr};
    inode_ptr cwd{nullptr};
    string prompt{"$ "};
    vector<string> path;

  public:
    inode_state();
    const string& get_prompt() const;
    void set_prompt(const string& new_prompt);
    inode_ptr get_root() const;
    inode_ptr get_cwd() const;
    void set_cwd(inode_ptr new_cwd);
    void cwd_push(const string& filepath);
    void cwd_pop(bool empty);
    const string cwd_str() const;
};

class inode {
  private:
    static size_t next_inode_num;
    size_t inode_num;
    base_file_ptr contents;

  public:
    inode(file_type);
    size_t get_inode_num() const;
    base_file_ptr get_contents() const;
};

class file_error : public runtime_error {
  public:
    file_error(const string& what);
};

class base_file {
  protected:
    base_file() = default;
    virtual const string& error_file_type() const = 0;

  public:
    virtual ~base_file() = default;
    base_file(const base_file&) = delete; // don't want copy operators
    base_file& operator=(const base_file&) = delete; // for base class
    virtual size_t size() const = 0;
    virtual const vector<string>& readfile() const;
    virtual void writefile(const vector<string>&);
    virtual void remove(const ptr_map::iterator, bool);
    virtual inode_ptr mkdir(const string&);
    virtual inode_ptr mkfile(const string&);
    virtual ptr_map& get_dirents();
};

class plain_file : public base_file {
  private:
    vector<string> data;
    virtual const string& error_file_type() const override {
        static const string file_type = "plain_file";
        return file_type;
    }

  public:
    virtual size_t size() const override;
    virtual const vector<string>& readfile() const override;
    virtual void writefile(const vector<string>& new_data) override;
};

class directory : public base_file {
  private:
    ptr_map dirents;
    virtual const string& error_file_type() const override {
        static const string file_type = "directory";
        return file_type;
    }

  public:
    virtual size_t size() const override;
    virtual void remove(const ptr_map::iterator itor, bool recursive) override;
    virtual inode_ptr mkdir(const string& dirname) override;
    virtual inode_ptr mkfile(const string& filename) override;
    virtual ptr_map& get_dirents() override;
};

#endif
