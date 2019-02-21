#include "mfs.h"
#include "file.h"
#include "list.h"
#include "type.h"
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>

#define up_dir   "..##########################"
#define this_dir ".###########################"
#define open_now f_open(_now, get_head(path_list, struct path_node, var)->name)

struct path_node {
        char name[file_name_max_len];
        struct list_node var;
};

static __u32 _root_dir;
static __u32 _block_size;
static __u32 _now;
static __u32 _dir_max_degree;
static struct list *path_list;

static int push_name(char *name)
{
        struct path_node *node = (struct path_node *)malloc(sizeof(struct path_node));
        if (!node)
                return 0;
        if (!name) {
                memset(node->name, illegal_char, file_name_max_len);
        } else {
                memcpy(node->name, name, file_name_max_len);
        }
        add_head(node, path_list, struct path_node, var);
        return 1;
}

static void pop_name()
{
        struct path_node *node = NULL;
        remove_head(node, path_list, struct path_node, var);
        if (node != NULL)
                free(node);
        return;
}


int mfs_init(struct super_block *sb)
{
        _dir_max_degree = sb->dir_max_degree;
        _root_dir = sb->root_dir;
        _block_size = sb->block_size;
        _now = _root_dir;
        create_list(path_list, struct path_node);
        push_name(NULL);
        return 1;
}

void mfs_uninit(struct super_block *sb)
{
        while (list_size(path_list) != 0)
                pop_name();
        destroy_list(path_list);
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

// the last file could be dir_type and file_type
static struct file *open_file_wp(char *path, int create)
{
        char *ptr = path;
        char name[file_name_max_len];
        int count;
        struct file *f = open_now;
        struct file *next = NULL;
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
                f_close(f);
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
        int i = 0, j;
        struct path_node *node = NULL;
        char names[_dir_max_degree][file_name_max_len];
        for_each(node, path_list, struct path_node, var) {
                memcpy(names[i], node->name, file_name_max_len);
                ++i;
        }
        while (--i != -1) {
                for (j = 0; j < file_name_max_len && names[i][j] != illegal_char; ++j)
                        printf("%c", names[i][j]);
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
        struct file *now = open_now;
        memset(name, illegal_char, file_name_max_len);
        while (dir_read(i++, name, &id, now)) {
                for (k = 0; k < file_name_max_len && name[k] != illegal_char; ++k) {
                        printf("%c", name[k]);
                }
                f = f_open(id, name);
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
        struct file *f = NULL, *next = NULL;
        //
        char nnn[28];
        memset(nnn, illegal_char, 28);
        nnn[0] = 'f';
        struct file *fff = f_open(2, nnn);
        f_close(fff);
        //
        while (1) {
                memset(name, illegal_char, file_name_max_len);
                count = split_path(&ptr, name);
                if (count == 0)
                        continue;
                if (is_name_equal(name, up_dir)) {
                        if (list_size(path_list) == 1)  // root dir
                                return 0;
                        pop_name();
                        f = open_now;
                        _now = f_parent(f);
                        f_close(f);
                } else if (is_name_equal(name, this_dir)) {
                        continue;
                } else {
                        if (list_size(path_list) == _dir_max_degree)
                                return 0;
                        f = open_now;
                        next = open_next_file(name, f);
                        if (!f || !next || (f_type(next) != DIR_TYPE)) {
                                f_close(f);
                                f_close(next);
                                return 0;
                        }
                        _now = next->ino_id;
                        push_name(name);
                        f_close(f);
                        f_close(next);
                }
                if (count == -1)
                        break;
        }
        //
        memset(nnn, illegal_char, 28);
        nnn[0] = 'f';
        fff = f_open(2, nnn);
        f_close(fff);
        //
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

int mfs_mv(char *src, char *dst)
{
        struct file *f_src, *f_dst;
        f_src = open_file_wp(src, 0);
        f_dst = open_file_wp(dst, 1);
        if (!f_src || !f_dst) {
                f_close(f_src);
                f_close(f_dst);
                return 0;
        }
        f_ci(f_src->ino_id, f_dst);
        struct file *p = f_open(f_parent(f_src), NULL);
        dir_remove(f_src->name, p);
        f_close(p);
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
        struct file *f = open_now;
        if (!f_create(&id, f->ino_id, type))
                return 0;
        ////////
        //char nnn[28];
        //memset(nnn, illegal_char, 28);
        //nnn[0] = 'f';
        //struct file *g = open_next_file(nnn, f);
        //
        char n[file_name_max_len];
        memset(n, illegal_char, file_name_max_len);
        int i = 0;
        while (name[i] != '\0') {
                n[i] = name[i];
                ++i;
        }
        dir_add(n, id, f);
        f_close(f);
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

// do not rm a dir
int mfs_rm(char *path)
{
        struct file *f = open_file_wp(path, 0);
        int i;
        __u32 id;
        if (!f)
                return 0;
        id = f->ino_id;
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

int mfs_cat(char *path)
{
        struct file *f = open_file_wp(path, 0);
        char buf[_block_size];
        __u32 leftover = f_size(f);
        __u32 cs, i;
        while (leftover) {
                cs = leftover > _block_size ? _block_size : leftover;
                f_read(buf, cs, f);
                for (i = 0; i < cs; ++i)
                        printf("%c", buf[i]);
                leftover -= cs;
        }
        f_close(f);
}

int mfs_input(char *src, char *dst)
{
        FILE *file = fopen(src, "r");
        if (!file)
                return 0;
        struct file *f = open_file_wp(dst, 1);
        if (!f) {
                fclose(file);
                return 0;
        }
        char buf[_block_size];
        fseek(file, 0, SEEK_END);
        __u32 i, cs, leftover = ftell(file);
        fseek(file, 0, SEEK_SET);
        f_cs(leftover, f);
        while (leftover) {
                cs = leftover > _block_size ? _block_size : leftover;
                fread(buf, 1, cs, file);
                f_write(buf, cs, f);
                leftover -= cs;
        }
        fclose(file);
        f_close(f);
        return 1;
}

int mfs_output(char *src, char *dst)
{
        struct file *f = open_file_wp(src, 0);
        if (!f)
                return 0;
        FILE *file = fopen(dst, "w");
        if (!file) {
                f_close(f);
                return 0;
        }
        char buf[_block_size];
        __u32 i, cs, leftover = f_size(f);
        f_cs(leftover, f);
        while (leftover) {
                cs = leftover > _block_size ? _block_size : leftover;
                f_read(buf, cs, f);
                fwrite(buf, 1, cs, file);
                leftover -= cs;
        }
        fclose(file);
        f_close(f);
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
