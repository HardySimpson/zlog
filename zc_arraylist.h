#ifndef __zc_arraylist_h
#define __zc_arraylist_h

#define ARRAY_LIST_DEFAULT_SIZE 32

typedef void (*zc_arraylist_del_fn) (void *data);
typedef int (*zc_arraylist_cmp_fn) (void *data1, void *data2);

typedef struct {
	void **array;
	int len;
	int size;
	zc_arraylist_del_fn del_fn;
} zc_arraylist_t;

zc_arraylist_t *zc_arraylist_new(zc_arraylist_del_fn del_fn);
void zc_arraylist_del(zc_arraylist_t * a_list);

void *zc_arraylist_get(zc_arraylist_t * a_list, int i);
int zc_arraylist_set(zc_arraylist_t * a_list, int i, void *data);
int zc_arraylist_add(zc_arraylist_t * a_list, void *data);
int zc_arraylist_sortadd(zc_arraylist_t * a_list, zc_arraylist_cmp_fn, void *data);

int zc_arraylist_len(zc_arraylist_t * a_list);

#endif
