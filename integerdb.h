#ifndef _YMIRDB_H
#define _YMIRDB_H

#include <stddef.h>

/* Pointer helper functions */

/*
 * Returns a negative integer if the first pointer is smaller than the second
 * one; returns zero if they are equal; returns a positive integer otherwise.
 */
int compare_ptr(const void *p1, const void *p2);

/*
 * Return the given pointer.
 */
void *clone_ptr(const void *p);

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
