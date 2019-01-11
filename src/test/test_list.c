#include "list.h"
#include <stdio.h>
#include <stdlib.h>

struct ufs{
        int data;
        struct list_node var;
};

int f(struct ufs *con) {
        if (con && con->data > 3)
                return 1;
        return 0;
}

int main() {
        struct list *ls;
        create_list(ls, struct ufs);
        int i;
        struct ufs *con;
        for (i = 0; i < 10; i++) {
                con = (struct ufs*)malloc(sizeof(struct ufs));
                con->data = i;
                con->var.next = NULL;
                add_tail(con, ls, struct ufs, var);
        }
        for (i = 0; i < 15; i++) {
                remove_head(con, ls, struct ufs, var);
                if (con)
                        printf("%d ", con->data);
                if (con)
                        free(con);
        }
        for (i = 0; i < 10; i++) {
                con = (struct ufs*)malloc(sizeof(struct ufs));
                con->data = i;
                con->var.next = NULL;
                add_tail(con, ls, struct ufs, var);
        }
        printf("\n");
        for_each(con, ls, struct ufs, var) {
                printf("%d ", con->data);
        }
        printf("\n");
        printf("-------------------------------");
        printf("\n");
        for (i = 0; i < 3; i++) {
                remove_container(f, con, ls, struct ufs, var);
                add_tail(con, ls, struct ufs, var);
                if (con)
                        printf("%d ", con->data);
        }
        printf("\n");
        for_each(con, ls, struct ufs, var) {
                printf("%d ", con->data);
        }
        printf("\n");
        printf("-------------------------------");
        printf("\n");
        for (i = 0; i < 15; i++) {
                remove_head(con, ls, struct ufs, var);
                if (con)
                        printf("%d ", con->data);
                if (con)
                        free(con);
        }
}
