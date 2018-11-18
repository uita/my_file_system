#ifndef LIST_H
#define LIST_H

struct list_list {
	struct list_node* head;
	struct list_node* tail;
	unsigned int size;
};
struct list_node {
	struct list_node* next;
};

/* get the pointer of next struct */
#define container_of(ptr, type, member) 				\
	((type*)((long)(ptr)-(long)(&(((type *)0)->member))))

/* create and init a list */
#define create_list(list_name) 				\
	struct list_list list_name = {NULL, NULL, 0}

/* add a container into head of list */
#define add_to_list_head(container, list, member)	 		\
	if(container != NULL) { 					\
		(container)->member.next = list.head;			\
		if (list.size == 0) {					\
			list.head = list.tail = &((container)->member);	\
		} else {						\
			list.head = &((container)->member);		\
		}							\
		list.size = list.size + 1; 				\
	}

/* add a container into tail of list */
#define add_to_list_tail(container, list, member)			\
	if(container != NULL) {						\
		(container)->member.next = NULL;				\
		if (list.size == 0) {					\
			list.head = list.tail = &((container)->member); 	\
		} else {						\
			list.tail->next = &((container)->member);		\
			list.tail = list.tail->next;			\
		}							\
		list.size = list.size + 1;				\
	}

/* remove head of list but do not delete the container */
#define remove_list_head(container, list, type, member)			\
	if (list.size != 0) {						\
		container = container_of(list.head, type, member);	\
		if (list.size == 1) {					\
			list.head = list.tail = NULL;			\
		} else {						\
			list.head = list.head->next;			\
			(container)->member.next = NULL;		\
		}							\
		list.size = list.size - 1;				\
	}

/* travel each container of list */
#define for_each(container, list, type, member) 			\
	container = container_of(list.head, type, member); 		\
	for(struct list_node* now = list.head; now != NULL; 		\
	now = now->next, container = container_of(now, type, member))

#endif
