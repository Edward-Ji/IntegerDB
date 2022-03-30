#ifndef _YMIRDB_H
#define _YMIRDB_H

#include <stddef.h>

/* Dynamic array of void pointers */

typedef void (*consumer)(void *);
typedef void (*aggregate)(const void *, void *);
typedef int (*comparator)(const void *, const void *);
typedef void *(*unary)(void *);

typedef struct darray darray;

/*
 * Creates a new dynamic array with items of a generic size. The free function
 * pointer must be provided to free any allocated memory of the items.
 */
darray *new_darray(consumer item_free);

/*
 * Sets the function pointer that frees the item if the item pointer is removed
 * from the array.
 */
int darray_set_item_free(darray *arrp, consumer item_free);

/*
 * Returns the length of the given dynamic array.
 */
size_t darray_len(darray *arrp);

/*
 * Calls the given function on every object in the array sequentially.
 */
int darray_foreach(darray *arrp, consumer fp);

/*
 * Calls the aggregation function with every item in the array as the first
 * argument, and the result pointer as the seconds argument.
 */
void darray_aggregate(darray *arrp, void *resp, aggregate fp);

/*
 * Adds a copy of the item at the end of the array.
 */
int darray_append(darray *arrp, void *itemp);

/*
 * Gets the item at a given index in the array.
 */
void *darray_get(darray *arrp, size_t index);

/*
 * Removes the item at a given index in the array.
 */
int darray_pop(darray *arrp, size_t index);

/*
 * Removes the items within a given index range in the array. The index range
 * includes all indices between start (inclusive) and end (exclusive).
 */
int darray_pop_range(darray *arrp, size_t start, size_t end);

/*
 * Inserts the item at a given index in the array.
 */
int darray_insert(darray *arrp, size_t index, void *itemp);

/*
 * Searches for the item in the array using the given comparator and stores its
 * index in the index pointer. The comparator is called with array items as the
 * first parameter, and should return 0 on equality and a non-zero value
 * otherwise. This function returns 1 if there is a match, or 0 otherwise.
 */
int darray_search(darray *arrp, void *itemp, comparator fp, size_t *indexp);

/*
 * Inserts all items of the second array at the given index in the first array
 * in the order they appear in the second array.
 */
int darray_extend_at(darray *arrp1, size_t index, darray *arrp2);

/*
 * Appends all items of the second array to the end of the first array.
 */
int darray_extend(darray *arrp1, darray *arrp2);

/*
 * Reverses all items of the given array in place.
 */
int darray_reverse(darray *arrp);

/*
 * Removes all adjacent unique items in the given array. The comparator is
 * called to compare each pair of adjacent items, and should only return 0 on
 * equality.
 */
int darray_unique(darray *arrp, comparator fp);

/*
 * Sorts all items in the given array in place using the quick sort algorithm.
 * The comparator should return a negative number if the first item is smaller,
 * 0 if the items are equal, and a positive number if the first item is greater.
 *
 * The sorting algorithm is adopted from
 * https://gist.github.com/adwiteeya3/f1797534506be672b591f465c3366643/
 */
int darray_sort(darray *arrp, comparator fp);

/*
 * Returns a deep clone of the given array using the clone function. The clone
 * function, given an item in the array, should return a deep clone of that
 * item. This means that any allocated field is also cloned.
 */
darray *darray_clone(darray *arrp, unary fp);

/*
 * Removes all items from the array.
 */
int darray_clear(darray *arrp);

/*
 * Removes all items and free the array's memory.
 */
void del_darray(darray *arrp);

/* Pointer helper functions */

/*
 * Returns a negative integer if the first pointer is smaller than the second
 * one; returns zero if they are equal; returns a positive integer otherwise.
 */
int compare_ptr(const void *p1, const void *p2);

/*
 * Return the given pointer.
 */
void *clone_ptr(void *p);

/* Database */

typedef enum ele_type { INTEGER, ENTRY } ele_type;

typedef struct element element;
typedef struct entry entry;
typedef struct snapshot snapshot;

element *new_int_ele(int num);
element *new_ent_ele(entry *ent);

void element_print(element *ele);

int element_has_type(const element *ele, ele_type *type);

int element_int_cmp(const element *ele1, const element *ele2);

void element_agg_min(const element *ele, int *min);
void element_agg_max(const element *ele, int *max);
void element_agg_sum(const element *ele, long long *sum);
void element_agg_len(const element *ele, size_t *len);

entry *new_entry(char *key);

void entry_print_key(entry *ent);
void entry_print_nokey(entry *ent);
void entry_print(entry *ent);

int entry_is_simple(entry *ent);

int entry_has_key(const entry *ent, const char *key);
int entry_key_cmp(const entry *ent, const entry *ent2);

void entry_add_ref(entry *ent1, entry *ent2);
void entry_del_ref(entry *ent1, entry *ent2);
void entry_ref_all(entry *ent, darray *elements);
void entry_deref_all(entry *ent);

int entry_min(entry *ent);
int entry_max(entry *ent);
long long entry_sum(entry *ent);
size_t entry_len(entry *ent);

entry *entry_empty_copy(entry *ent);

/*
 * The find pool function accepts an array of entries as the find pool for the
 * find copy function. The find copy function accepts an entry and returns the
 * entry with the same key in the pool.
 */
void entry_find_pool(darray *pool);
entry *entry_find_copy(entry *ent);

void del_entry(entry *ent);

/*
 * The entries_can_purge function returns 0 if the key exists but can not purge.
 * The purge function deletes the entry with the given key from the array of
 * entries only when the entry has no backward references.
 */
int entries_can_purge_key(darray *entries, char *key);
void entries_purge_key(darray *entries, char *key);

/*
 * Creates a deep copy array of the given entries. All new entries are
 * independent of the old entries and linked to themselves in the same way as
 * the old ones.
 */
darray *entries_clone(darray *entries);

snapshot *new_snapshot(darray *entries);

void snapshot_print(snapshot *snap);

int snapshot_has_id(const snapshot *snap, const size_t *id);

void del_snapshot(snapshot *snap);

/* Helper parser functions */

/*
 * Given an integer string of base 10, converts it to an integer and stores it
 * in the result pointer. Returns 1 if the conversion is successful, 0
 * otherwise.
 */
int parse_int(char *str, int *resp);

/*
 * Given a non-negative integer string of base 10, converts it to a unsigned
 * size type and stores it in the result pointer. The number must be greater
 * than zero, and smaller than or equal to the maximum value provided. Returns 1
 * if the conversion is successful, 0 otherwise.
 */
int parse_index(char *str, size_t max, size_t *resp);

/*
 * Parse the elements in an argument list and return a dynamic array containing
 * all the elements. If an error occurred while parsing, the function returns
 * `NULL`. Entry elements can not be the same as self.
 */
darray *parse_elements(char **strp, darray *entries, entry *self);

/*
 * Parse a string into an entry with such key. If an error occurred while
 * parsing, the function returns `NULL`.
 */
entry *parse_entry(char **strp, darray *entries);

#endif
