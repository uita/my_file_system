#ifndef LIST_H
#define LIST_H

#include <string.h>

struct list {
        unsigned size;
        struct list_node *head;
        struct list_node *tail;
};

struct list_node {
        struct list_node *next;
};

#define create_list(ls, type) \
        ls = (struct list *)malloc(sizeof(struct list)); \
        if (ls) \
                memset(ls, 0, sizeof(struct list));

#define destroy_list(ls) \
        if (ls) \
                free(ls); \
        ls = NULL;

#define list_size(ls) (ls->size)

#define container(ln, type, var) (ln == NULL ? NULL : (type *)(((void*)ln)-((unsigned long)(&((type *)0)->var))))

#define next_container(con, type, var) container(con->var.next, type, var)

#define for_each(con, ls, type, var) \
        for (con = container(ls->head, type, var); con != NULL; \
                        con = next_container(con, type, var))

#define get_head(ls, type, var) container(ls->head, type, var);

#define get_tail(ls, type, var) container(ls->tail, type, var);

#define add_tail(con, ls, type, var) \
        if (ls->size != 0) { \
                ls->tail->next = &(con->var); \
                ls->tail = &(con->var); \
        } else { \
                ls->head = ls->tail = &(con->var); \
        } \
        ls->tail->next = NULL; \
        ls->size += 1; \

#define add_head(con, ls, type, var) \
        if (ls->size != 0) { \
                con->var.next = ls->head; \
                ls->head = &(con->var); \
        } else { \
                ls->head = ls->tail = &(con->var); \
        } \
        ls->size += 1;

#define remove_head(con, ls, type, var) \
        if (ls->size > 0) { \
                con = container(ls->head, type, var); \
                ls->head = ls->head->next; \
                con->var.next = NULL; \
                ls->size -= 1; \
                if (ls->size == 0) { \
                        ls->tail = NULL; \
                } \
        } else { \
                con = NULL; \
        }

/* remove and get the first container which makes 'func' return true 
 * the 'func' should return 0 or false if pass a NULL pointer to it*/
#define remove_container(func, con, ls, type, var) \
        if (func(container(ls->head, type, var))) { \
                remove_head(con, ls, type, var); \
        } else { \
                void *p = NULL; \
                int is_find = 0; \
                for_each(con, ls, type, var) { \
                        p = next_container(con, type, var); \
                        if (func((type *)p)) { \
                                if (&((((type *)p)->var)) == ls->tail) { \
                                        ls->tail = &(con->var); \
                                } \
                                con->var.next = con->var.next->next; \
                                con = (type *)p; \
                                con->var.next = NULL; \
                                ls->size -= 1; \
                                is_find = 1; \
                                break; \
                        } \
                } \
                if (!is_find) {\
                        con = NULL; \
                } \
        }

#endif

