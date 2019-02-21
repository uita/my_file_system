#include "mfs.h"
#include "file.h"
#include "list.h"
#include "type.h"
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>

#define up_dir   "..##########################"
#define this_dir ".###########################"

static __u32 _root_dir;
static __u32 _block_size;
static struct file **_pdirs = NULL;
static __u32 _now;
static __u32 _dir_max_degree;


int mfs_init(struct super_block *sb)
{
        _dir_max_degree = sb->dir_max_degree;
        _root_dir = sb->root_dir;
        _block_size = sb->block_size;
        _now = 0;
        _pdirs = (struct file **)malloc(sizeof(struct file *) * _dir_max_degree);
        if (!_pdirs)
                return 0;
        _pdirs[0] = f_open(_root_dir, NULL);
        return 1;
}

void mfs_uninit(struct super_block *sb)
{
        free(_pdirs);
}

static int is_path_valid(char *path)
{
        regex_t reg;
        const char *pattern = "\\/?([\\w\\.]{1,28}\\/)*(\\w+)?";
        regcomp(&reg, pattern, REG_EXTENDED);
        const size_t nmatch = 1;
        regmatch_t pmatch[1];
        int status = regexec(&reg, path, nmatch, pmatch, 0);
        if (status == 0) {
                regfree(&reg);
                return 1;
        } else {
                regfree(&reg);
                return 0;
        }
}

static int split_path(char **ptr, char *name)
{
        int count = 0;
        while ((**ptr) != '/') {
                if ((**ptr) == '\0') {
                        count = -1;
                        break;
                }
                name[count] = **ptr;
                ++count;
                ++(*ptr);
        }
        ++(*ptr);
        return count;
}

static int is_name_equal(char *n1, const char *n2) {
        int i = 0;
        while (i < file_name_max_len && n1[i] == n2[i])
                ++i;
        if (i == file_name_max_len)
                return 1;
        return 0;
}

static void sync_pdirs() {
        int i;
        for (i = 0; i <= _now; ++i)
                f_sync(_pdirs[i]);
}

static struct file *open_next_file(char *name, struct file *f)
{
        char n[file_name_max_len];
        int i = 0;
        __u32 id;
        while (dir_read(i++, n, &id, f)) {
                if (is_name_equal(name, n)) {
                        return f_open(id, n);
                }
        }
        return NULL;
}

static struct file *open_file_wp(char *path, int create)
{
        char *ptr = path;
        char name[file_name_max_len];
        int count;
        struct file *f = _pdirs[_now], *next= NULL;
        int is_begin = 1;
        sync_pdirs(); // need to sync all dirs on path to avoid dirty data
        while (1) {
                memset(name, illegal_char, file_name_max_len);
                count = split_path(&ptr, name);
                if (count == 0)
                        continue;
                if (is_name_equal(name, up_dir)) {
                        next = f_open(f_parent(f), NULL);
                } else if (is_name_equal(name, this_dir)) {
                        continue;
                } else {
                        next = open_next_file(name, f);
                }
                if (!next) {
                        if (count != -1 || create == 0) {
                                f_close(f);
                                return NULL;
                        }
                        // create last file
                        __u32 id;
                        if (!f_create(&id, f->ino_id, FILE_TYPE)) {
                                f_close(f);
                                return NULL;
                        }
                        dir_add(name, id, f);
                        f_close(f);
                        return f_open(id, name);
                }
                if (!is_begin) {
                        f_close(f);
                } else {
                        is_begin = 0;
                }
                f = next;
                if (count != -1 && f_type(f) != DIR_TYPE) {
                        f_close(f);
                        return NULL;
                }
                if (count == -1)
                        break;
        }
        return f;
}

void mfs_pwd()
{
        int i, j;
        printf("/");
        for (i = 1; i <= _now; ++i) {
                for (j = 0;
                     j < file_name_max_len && _pdirs[i]->name[j] != illegal_char;
                     ++j) {
                        printf("%c", _pdirs[i]->name[j]);
                }
                printf("/");
        }
        printf("\n");
}

void mfs_ls()
{
        int i = 0, k;
        char name[file_name_max_len];
        __u32 id;
        struct file *f = NULL;
        memset(name, illegal_char, file_name_max_len);
        while (dir_read(i++, name, &id, _pdirs[_now])) {
                for (k = 0; k < file_name_max_len && name[k] != illegal_char; ++k) {
                        printf("%c", name[k]);
                }
                f = open_next_file(name, _pdirs[_now]);
                if (!f)
                        return;
                if (f_type(f) == DIR_TYPE)
                        printf("/");
                printf(" ");
                f_close(f);
                memset(name, illegal_char, file_name_max_len);
        }
        printf("\n");
}

int mfs_cd(char *path)
{
        char *ptr = path;
        char name[file_name_max_len];
        int count;
        struct file *f = NULL;
        while (1) {
                memset(name, illegal_char, file_name_max_len);
                count = split_path(&ptr, name);
                if (count == 0)
                        continue;
                if (is_name_equal(name, up_dir)) {
                        if (_now == 0)
                                return 0;
                        f_close(_pdirs[_now]);
                        --_now;
                } else if (is_name_equal(name, this_dir)) {
                        continue;
                } else {
                        if (_now == _dir_max_degree - 1)
                                return 0;
                        f = open_next_file(name, _pdirs[_now]);
                        if (!f)
                                return 0;
                        if (f_type(f) != DIR_TYPE) {
                                f_close(f);
                                return 0;
                        }
                        _pdirs[++_now] = f;
                }
                if (count == -1)
                        break;
        }
        return 1;
}

void cp_r(struct file *src, struct file *dst)
{
        ;
}

void cp(struct file *src, struct file *dst)
{
        char buf[_block_size];
        f_cs(f_size(src), dst);
        __u32 leftover = f_size(src);
        __u32 cp_size;
        while (leftover != 0) {
                cp_size = leftover < _block_size ? leftover :_block_size;
                f_read(buf, cp_size, src);
                f_write(buf, cp_size, dst);
                leftover -= cp_size;
        }
}

int mfs_cp(char *src, char *dst)
{
        struct file *f_src, *f_dst;
        f_src = open_file_wp(src, 0);
        f_dst = open_file_wp(dst, 1);
        if (!f_src || !f_dst) {
                f_close(f_src);
                f_close(f_dst);
                return 0;
        }
        if (f_type(f_src) == DIR_TYPE) {
                cp_r(f_src, f_dst);  //not yet
        } else {
                cp(f_src, f_dst);
        }
        f_close(f_src);
        f_close(f_dst);
}

static struct file *find_equal_ino_wp(__u32 ino_id) {
        int i;
        for (i = 0; i <= _now; ++i)
                if (_pdirs[i]->ino_id == ino_id)
                        return _pdirs[i];
        return NULL;
}

int mfs_mv(char *src, char *dst)
{
        struct file *f_src, *f_dst;
        sync_pdirs();
        f_src = open_file_wp(src, 0);
        f_dst = open_file_wp(dst, 1);
        if (!f_src || !f_dst) {
                f_close(f_src);
                f_close(f_dst);
                return 0;
        }
        f_ci(f_src->ino_id, f_dst);
        struct file *p = find_equal_ino_wp(f_parent(f_src));
        if (p != NULL) {
                dir_remove(f_src->name, p);
        } else {
                p = f_open(f_parent(f_src), NULL);
                dir_remove(f_src->name, p);
                f_close(p);
        }
        f_close(f_dst);
        if (f_src->block != NULL)
                free(f_src->block);
        if (f_src->ino != NULL)
                free(f_src->ino);
        free(f_src);
}

static int add_subdir(char *name, int type)
{
        __u32 id;
        if (!f_create(&id, _pdirs[_now]->ino_id, type))
                return 0;
        char n[file_name_max_len];
        memset(n, illegal_char, file_name_max_len);
        int i = 0;
        while (name[i] != '\0') {
                n[i] = name[i];
                ++i;
        }
        dir_add(n, id, _pdirs[_now]);
        return 1;
}

int mfs_mkdir(char *name)
{
        return add_subdir(name, DIR_TYPE);
}

int mfs_touch(char *name)
{
        return add_subdir(name, FILE_TYPE);
}

int mfs_rm(char *path)
{
        struct file *f = open_file_wp(path, 0);
        int i;
        __u32 id;
        if (!f)
                return 0;
        id = f->ino_id;
        for (i = 0; i <= _now; ++i) {
                if (_pdirs[i]->ino_id == f_parent(f)) {
                        dir_remove(f->name, _pdirs[i]);
                        f_close(f);
                        f_delete(id);
                        return 1;
                }
        }
        struct file *p = f_open(f_parent(f), NULL);
        if (!p) {
                f_close(f);
                return 0;
        }
        dir_remove(f->name, p);
        f_close(p);
        f_close(f);
        f_delete(id);
        return 1;
}

struct file *mfs_open(char *path)
{
        return open_file_wp(path, 1);
}

int mfs_close(struct file *f)
{
        f_close(f);
}

int mfs_read(void *data, __u32 size, struct file *f)
{
        return f_read(data, size, f);
}

int mfs_write(void *data, __u32 size, struct file *f)
{
        return f_write(data, size, f);
}
