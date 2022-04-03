#ifndef _YMIRDB_H
#define _YMIRDB_H

#include <stddef.h>

/* Dynamic array of void pointers */

/*
 * A function of this type should take in a pointer to some object and perform
 * some operation on that object. The function should not return anything.
 */
typedef void (*consumer)(void *);

/*
 * A function of this type should take in a pointer to some object and modify
 * the result given by the second pointer. The function should not return
 * anything or modify the first object.
 */
typedef void (*aggregate)(const void *, void *);

/*
 * A function of this type should take in two pointers to objects and compare
 * them. It should return:
 * - a negative number if the first object is smaller;
 * - zero if they are equal; and
 * - a positive number if the first object is bigger.
 * It should not modify either object or return anything.
 */
typedef int (*comparator)(const void *, const void *);

/*
 * A function of this type should take in a pointer to some object and return a
 * pointer to another object associated with the given one (e.g. a copy of the
 * original object).
 */
typedef void *(*unary)(void *);

/*
 * A structure type that represents a dynamically sized array.
 */
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

/*
 * A enumeration representing integer elements and entry elements.
 */
typedef enum ele_type { INTEGER, ENTRY } ele_type;

/*
 * A structure representing an element in some entry. There can be two types of
 * elements:
 * - an integer element is a number represented by C int type;
 * - an entry element is reference to another entry represented by C pointer.
 */
typedef struct element element;

/*
 * A structure representing an entry. Each entry has a unique key and can
 * contain zero or more elements.
 */
typedef struct entry entry;

/*
 * A structure representing a snapshot. A snapshot can be taken at any time. Each
 * snapshot has a ID that is unique for its life-time and beyond, and a deep
 * copy of the entries at the time the snapshot was taken.
 */
typedef struct snapshot snapshot;

/*
 * Creates a new integer element.
 */
element *new_int_ele(int num);
/*
 * Creates a new entry element.
 */
element *new_ent_ele(entry *ent);

/*
 * Prints the element. If the element is an integer, its value its value is
 * printed in decimal; if the element is an entry, its key is printed.
 */
void element_print(element *ele);

/*
 * Compare two integer elements. Return the value of the first element minus the
 * that of the second.
 */
int element_int_cmp(const element *ele1, const element *ele2);

/*
 * Element aggregation functions.
 *
 * They all takes in an element and a pointer to the result type.
 * If the element is an integer, for
 * - min: if the element's value is smaller, replace the result by that value;
 * - max: if the element's value is greater, replace the result by that value;
 * - sum: add the element's value to the result;
 * - len: add one to the result.
 * If the element is an entry, perform the aggregation on the entry's elements.
 */
void element_agg_min(const element *ele, int *min);
void element_agg_max(const element *ele, int *max);
void element_agg_sum(const element *ele, long long *sum);
void element_agg_len(const element *ele, size_t *len);

/*
 * Creates a new entry with the given key.
 */
entry *new_entry(char *key);

/*
 * Prints the entry.
 *
 * - key: print the key of that entry.
 * - nokey: print the elements of that entry separated by space, surrounded by
 *   square brackets.
 * - *: print the key and the elements of that entry.
 */
void entry_print_key(entry *ent);
void entry_print_nokey(entry *ent);
void entry_print(entry *ent);

/*
 * Returns if the entry has any backward references.
 */
int entry_is_simple(entry *ent);

/*
 * Comparator functions.
 *
 * - has_key: returns if the entry's key matches the given key;
 * - key_cmp: returns the first entry's key comparing to the second entry's key,
 */
int entry_has_key(const entry *ent, const char *key);
int entry_key_cmp(const entry *ent, const entry *ent2);

/*
 * Reference management functions.
 *
 * If an entry e1 is added as an element to another entry e2, we must "link"
 * them together:
 * - Update e1's forward reference with e2 and all its forward references;
 * - Update e2's backward reference with e1 and all its backward references.
 * If an entry is removed from another entry's element list, we must do the
 * reverse - "unlink" them.
 * - Remove e2 and all its forward references from e1's forward reference;
 * - Remove e1 and all its backward references from e2's backward reference;
 *
 * The reference all function links all entry elements in the given element
 * list.
 * The dereference all function unlinks all entries in its forward reference
 * list.
 */
void entry_add_ref(entry *ent1, entry *ent2);
void entry_del_ref(entry *ent1, entry *ent2);
void entry_ref_all(entry *ent, darray *elements);
void entry_deref_all(entry *ent);

/*
 * Statistics functions.
 *
 * Returns the minimum, maximum, sum and length of the given entry respectively.
 * If the entry contains other entries, the same function is called on the
 * sub-entry and returned value is aggregated to the result.
 */
int entry_min(entry *ent);
int entry_max(entry *ent);
long long entry_sum(entry *ent);
size_t entry_len(entry *ent);

/*
 * Creates an empty copy of the entry with only the key.
 */
entry *entry_empty_copy(entry *ent);

/*
 * The find pool function accepts an array of entries as the find pool for the
 * find copy function. The find copy function accepts an entry and returns the
 * entry with the same key in the pool.
 */
void entry_find_pool(darray *pool);
entry *entry_find_copy(entry *ent);

/*
 * Deletes the entry and frees all its subsequent memory.
 */
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

/*
 * Creates a new snapshot of given entries. The ID is unique throughout the
 * lifetime of the program, incremented by 1 each time a new snapshot is
 * created.
 */
snapshot *new_snapshot(darray *entries);

/*
 * Prints the snapshot's ID number.
 */
void snapshot_print(snapshot *snap);

/*
 * Compares the snapshot's ID with the given ID number.
 */
int snapshot_has_id(const snapshot *snap, const size_t *id);

/*
 * Deletes the snapshot and frees all its subsequent memories.
 */
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
