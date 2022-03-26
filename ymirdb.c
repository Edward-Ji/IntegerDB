/*
 * COMP2017 - assignment 2
 * Ziao Ji (Edward)
 * ziji4098
 */

#define _GNU_SOURCE

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "help.h"

#define KEYLEN (16)
#define BUFLEN (1024)
#define WHITESPACE " \t\r\n\v\f"

/* Dynamic array of void pointers */

typedef void (*consumer)(void *);
typedef void (*aggregate)(const void *, void *);
typedef int (*comparator)(const void *, const void *);
typedef void *(*unary)(void *);

typedef struct darray darray;

struct darray {
    void **itempp;
    consumer item_free;
    size_t len;
    size_t cap;
};

darray *new_darray(consumer item_free) {
    darray *arrp = malloc(sizeof(darray));
    if (arrp != NULL) {
        arrp->item_free = item_free;
        arrp->len = 0;
        arrp->cap = 1;
        arrp->itempp = malloc(sizeof(void *) * arrp->cap);
        if (arrp->itempp == NULL) {
            return NULL;
        }
    }

    return arrp;
}

int darray_set_item_free(darray *arrp, consumer item_free) {
    if (arrp == NULL) {
        return 0;
    }

    arrp->item_free = item_free;

    return 1;
}

int _darray_resize(darray *arrp, size_t len) {
    if (arrp == NULL) {
        return 0;
    }

    void **itempp = arrp->itempp;
    size_t cap = arrp->cap;

    if (len > cap) {
        while (len > cap) {
            cap *= 2;
        }
    } else {
        while (len <= cap / 2 && cap > 1) {
            cap /= 2;
        }
    }
    if (cap != arrp->cap) {
        itempp = realloc(itempp, sizeof(void *) * cap);
        if (itempp == NULL) {
            return 0;
        }
        arrp->itempp = itempp;
        arrp->cap = cap;
    }

    return arrp->cap;
}

size_t darray_len(darray *arrp) {
    return arrp->len;
}

int darray_foreach(darray *arrp, consumer fp) {
    if (arrp == NULL || fp == NULL) {
        return 0;
    }

    for (size_t i = 0; i < arrp->len; i++) {
        fp(arrp->itempp[i]);
    }

    return 1;
}

void darray_aggregate(darray *arrp, void *resp, aggregate fp) {
    if (arrp == NULL || fp == NULL) {
        return;
    }

    for (size_t i = 0; i < arrp->len; i++) {
        fp(arrp->itempp[i], resp);
    }
}

int darray_append(darray *arrp, void *itemp) {
    if (arrp == NULL || itemp == NULL) {
        return 0;
    }

    if (!_darray_resize(arrp, arrp->len + 1)) {
        return 0;
    }

    arrp->itempp[arrp->len] = itemp;

    arrp->len++;

    return 1;
}

void *darray_get(darray *arrp, size_t index) {
    if (arrp == NULL || index >= arrp->len) {
        return NULL;
    }

    return arrp->itempp[index];
}

int darray_pop(darray *arrp, size_t index) {
    if (arrp == NULL || index >= arrp->len) {
        return 0;
    }

    if (arrp->item_free != NULL) {
        arrp->item_free(arrp->itempp[index]);
    }
    memmove(arrp->itempp + index,
            arrp->itempp + index + 1,
            sizeof(void *) * (arrp->len - index - 1));
    if (!_darray_resize(arrp, arrp->len - 1)) {
        return 0;
    }

    arrp->len--;

    return 1;
}

int darray_pop_range(darray *arrp, size_t start, size_t end) {
    if (arrp == NULL || start > end || end > arrp->len) {
        return 0;
    }

    if (arrp->item_free != NULL) {
        for (size_t i = start; i < end; i++) {
            arrp->item_free(arrp->itempp[i]);
        }
    }
    memmove(arrp->itempp + start,
            arrp->itempp + end,
            sizeof(void *) * (arrp->len - end));
    if (!_darray_resize(arrp, arrp->len - (end - start))) {
        return 0;
    }

    arrp->len -= end - start;

    return 1;
}

int darray_insert(darray *arrp, size_t index, void *itemp) {
    if (arrp == NULL || index > arrp->len || itemp == NULL) {
        return 0;
    }

    if (!_darray_resize(arrp, arrp->len + 1)) {
        return 0;
    }
    memmove(arrp->itempp + index + 1,
            arrp->itempp + index,
            sizeof(void *) * (arrp->len - index));

    arrp->itempp[index] = itemp;

    arrp->len++;

    return 1;
}

int darray_search(darray *arrp, void *itemp, comparator fp, size_t *indexp) {
    if (arrp == NULL || itemp == NULL || fp == NULL || indexp == NULL) {
        return 0;
    }

    for (size_t i = 0; i < arrp->len; i++) {
        if (fp(arrp->itempp[i], itemp) == 0) {
            *indexp = i;
            return 1;
        }
    }

    return 0;
}

int darray_extend_at(darray *arrp1, size_t index, darray *arrp2) {
    if (arrp1 == NULL || index > arrp1->len || arrp2 == NULL) {
        return 0;
    }

    if (!_darray_resize(arrp1, arrp1->len + arrp2->len)) {
        return 0;
    }
    memmove(arrp1->itempp + index + arrp2->len,
            arrp1->itempp + index,
            sizeof(void *) * (arrp1->len - index));

    for (size_t i = 0; i < arrp2->len; i++) {
        arrp1->itempp[index + i] = darray_get(arrp2, i);
    }

    arrp1->len += arrp2->len;

    return 1;
}

int darray_extend(darray *arrp1, darray *arrp2) {
    if (arrp1 == NULL || arrp2 == NULL) {
        return 0;
    }

    if (!_darray_resize(arrp1, arrp1->len + arrp2->len)) {
        return 0;
    }

    for (size_t i = 0; i < arrp2->len; i++) {
        arrp1->itempp[arrp1->len + i] = darray_get(arrp2, i);
    }

    arrp1->len += arrp2->len;

    return 1;
}

int darray_reverse(darray *arrp) {
    if (arrp == NULL) {
        return 0;
    }

    for (size_t i = 0; i < arrp->len / 2; i++) {
        void *temp = arrp->itempp[i];
        arrp->itempp[i] = arrp->itempp[arrp->len - i - 1];
        arrp->itempp[arrp->len - i - 1] = temp;
    }

    return 1;
}

int darray_unique(darray *arrp, comparator fp) {
    if (arrp == NULL || fp == NULL) {
        return 0;
    }

    void **itempp = arrp->itempp;
    size_t m = 1;
    size_t i = 1;
    for (; i < arrp->len; i++) {
        if (fp(itempp[i-1], itempp[i]) != 0) {
            if (!darray_pop_range(arrp, m, i)) {
                return 0;
            }
            i -= i - m;
            m = i + 1;
        }
    }
    if (!darray_pop_range(arrp, m, i)) {
        return 0;
    }

    return 1;
}

int _compare_wrapper(const void *p1, const void *p2, void *fp) {
    return ((comparator) fp)(*(void **) p1, *(void **) p2);
}

int darray_sort(darray *arrp, comparator fp) {
    if (arrp == NULL || fp == NULL) {
        return 0;
    }

    qsort_r(arrp->itempp, arrp->len, sizeof(void *), _compare_wrapper, fp);

    return 1;
}

darray *darray_clone(darray *arrp, unary fp) {
    if (arrp == NULL || fp == NULL) {
        return NULL;
    }

    darray *clonep = (darray *) malloc(sizeof(darray));

    memcpy(clonep, arrp, sizeof(darray));

    clonep->itempp = (void **) malloc(sizeof(void *) * clonep->len);
    for (size_t i = 0; i < clonep->len; i++) {
        clonep->itempp[i] = fp(arrp->itempp[i]);
    }

    return clonep;
}

int darray_clear(darray *arrp) {
    if (arrp == NULL) {
        return 0;
    }

    for (size_t i = 0; i < arrp->len; i++) {
        if (arrp->item_free != NULL) {
            arrp->item_free(arrp->itempp[i]);
        }
    }
    arrp->len = 0;

    return 1;
}

void del_darray(darray *arrp) {
    if (arrp == NULL) {
        return;
    }

    darray_clear(arrp);
    free(arrp->itempp);
    free(arrp);
}

/* Pointer helper functions */

int compare_ptr(const void *p1, const void *p2) {
    if (p1 < p2) { return -1; }
    if (p1 > p2) { return 1; }
    return 0;
}

void *clone_ptr(void *p) {
    return p;
}

/* Database */

typedef enum ele_type { INTEGER, ENTRY } ele_type;

typedef struct element element;
typedef struct entry entry;
typedef struct snapshot snapshot;

struct element {
    enum ele_type type;
    union {
        int num;
        struct entry *entry;
    } value;
};

struct entry {
    char key[KEYLEN];
    darray *elements;
    darray *forward;
    darray *backward;
};

struct snapshot {
    int id;
    darray *entries;
};

element *new_int_ele(int num) {
    element *ele = (element *) malloc(sizeof(element));
    if (ele != NULL) {
        ele->type = INTEGER;
        ele->value.num = num;
    }

    return ele;
}

element *new_ent_ele(entry *ent) {
    element *ele = (element *) malloc(sizeof(element));
    if (ele != NULL) {
        ele->type = ENTRY;
        ele->value.entry = ent;
    }

    return ele;
}

void element_print(element *ele) {
    if (ele == NULL) {
        printf("nil");
        return;
    }
    switch (ele->type) {
        case INTEGER:
            printf("%d", ele->value.num);
            break;
        case ENTRY:
            printf("%s", ele->value.entry->key);
            break;
        default:
            printf("?");
    }
}

int element_has_type(const element *ele, ele_type *type) {
    return ele->type != *type;
}

int element_int_cmp(const element *ele1, const element *ele2) {
    int num1 = ele1->value.num;
    int num2 = ele2->value.num;
    if (num1 < num2) { return -1; }
    if (num1 > num2) { return 1; }
    return 0;
}

entry *new_entry(char *key) {
    entry *ent = (entry *) malloc(sizeof(entry));

    if (ent != NULL) {
        strncpy(ent->key, key, KEYLEN);
        ent->elements = new_darray(free);
        ent->forward = new_darray(NULL);
        ent->backward = new_darray(NULL);
    }

    return ent;
}

int entry_has_key(entry *ent, char *key) {
    return strcmp(ent->key, key);
}

void entry_print_key(entry *ent) {
    printf("%s\n", ent->key);
}

void entry_print_nokey(entry *ent) {
    printf("[");
    if (ent->elements->len != 0) {
        element_print(darray_get(ent->elements, 0));
    }
    for (size_t i = 1; i < ent->elements->len; i++) {
        putchar(' ');
        element_print(darray_get(ent->elements, i));
    }
    printf("]\n");
}

void entry_print(entry *ent) {
    printf("%s ", ent->key);
    entry_print_nokey(ent);
}

int entry_is_simple(entry *ent) {
    return darray_len(ent->forward) == 0;
}

int entry_key_cmp(const entry *ent1, const entry *ent2) {
    return strcmp(ent1->key, ent2->key);
}

void entry_add_ref(entry *ent1, entry *ent2) {
    darray_append(ent1->forward, ent2);
    darray_append(ent2->backward, ent1);
    for (size_t i = 0; i < ent2->forward->len; i++) {
        entry *ent = darray_get(ent2->forward, i);
        darray_append(ent1->forward, ent);
        darray_append(ent->backward, ent1);
    }
}

void entry_del_ref(entry *ent1, entry *ent2) {
    size_t idx;
    darray_search(ent1->forward, ent2, compare_ptr, &idx);
    darray_pop(ent1->forward, idx);
    darray_search(ent2->backward, ent1, compare_ptr, &idx);
    darray_pop(ent2->backward, idx);
    for (size_t i = 0; i < ent2->forward->len; i++) {
        entry *ent = darray_get(ent2->forward, i);
        darray_search(ent1->forward, ent, compare_ptr, &idx);
        darray_pop(ent1->forward, idx);
        darray_search(ent->backward, ent1, compare_ptr, &idx);
        darray_pop(ent->backward, idx);
    }
}

void entry_ref_all(entry *ent, darray *elements) {
    for (size_t i = 0; i < elements->len; i++) {
        element *ele = darray_get(elements, i);
        if (ele->type == ENTRY) {
            entry_add_ref(ent, ele->value.entry);
        }
    }
}

void entry_deref_all(entry *ent) {
    for (size_t i = 0; i < ent->forward->len; i++) {
        entry_del_ref(ent, darray_get(ent->forward, i));
    }
}

void del_entry(entry *ent) {
    del_darray(ent->elements);
    del_darray(ent->forward);
    del_darray(ent->backward);

    free(ent);
}

snapshot *new_snapshot(int id, darray *entries) {
    snapshot *snap = (snapshot *) malloc(sizeof(snapshot));

    if (snap != NULL) {
        snap->id = id;
        snap->entries = new_darray((consumer) del_entry);
    }

    return snap;
}

void snapshot_print(snapshot *snap) {
    printf("snapshot %d\n", snap->id);
}

void del_snapshot(snapshot *snap) {
    del_darray(snap->entries);

    free(snap);
}

/* Helper parsers */

/*
 * Given an integer string of base 10, converts it to an integer and stores it
 * in the result pointer. Returns 1 if the conversion is successful, 0
 * otherwise.
 */
int parse_int(char *str, int *resp) {
    char *end;
    long num = strtol(str, &end, 10);
    if (end == str || *end != '\0' || num < INT_MIN || num > INT_MAX) {
        return 0;
    }
    *resp = num;
    return 1;
}

/*
 * Given a non-negative integer string of base 10, converts it to a unsigned
 * size type and stores it in the result pointer. The number must be smaller
 * than the maximum value provided. Returns 1 if the conversion is successful, 0
 * otherwise.
 */
int parse_index(char *str, size_t max, size_t *resp) {
    char *end;
    unsigned long num = strtoul(str, &end, 10);
    if (end == str || *end != '\0' || num >= max) {
        return 0;
    }
    *resp = num;
    return 1;
}

/*
 * Parse the elements in an argument list and return a dynamic array containing
 * all the elements. If an error occurred while parsing, the function returns
 * `NULL`.
 */
darray *parse_elements(char **strp, darray *entries) {
    darray *elements = new_darray(free);

    char *token;
    size_t idx;
    while ((token = strsep(strp, WHITESPACE)) != NULL) {
        int num;
        element *ele;
        if (isdigit(*token)) {
            if (parse_int(token, &num)) {
                ele = new_int_ele(num);
            } else {
                printf("invalid integer\n");
                del_darray(elements);
                return NULL;
            }
        }
        else {
            if (darray_search(entries, token, (comparator) entry_has_key, &idx)) {
                ele = new_ent_ele(darray_get(entries, idx));
            } else {
                printf("no such key\n");
                del_darray(elements);
                return NULL;
            }
        }
        darray_append(elements, ele);
    }

    darray_set_item_free(elements, NULL);

    return elements;
}

/*
 * Parse a string into an entry with such key. If an error occurred while
 * parsing, the function returns `NULL`.
 */
entry *parse_entry(char **strp, darray *entries) {
    char *key = strsep(strp, WHITESPACE);
    size_t idx;
    if (!darray_search(entries, key, (comparator) entry_has_key, &idx)) {
        printf("no such key\n");
        return NULL;
    }

    return darray_get(entries, idx);
}

/* Commands */

void command_help() {
    fputs(HELP_STRING, stdout);
}

void command_list(char *args, darray *snapshots, darray *entries) {
    char *what = strsep(&args, WHITESPACE);
    if (strcasecmp(what, "keys") == 0) {
        darray_foreach(entries, (consumer) entry_print_key);
    } else if (strcasecmp(what, "entries") == 0) {
        darray_foreach(entries, (consumer) entry_print);
    } else if (strcasecmp(what, "snapshots") == 0) {
        darray_foreach(snapshots, (consumer) snapshot_print);
    } else {
        printf("invalid list command\n");
    }
}

void command_get(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }
    entry_print_nokey(ent);
}

void command_del(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }
    if (ent->backward->len != 0) {
        printf("not permitted\n");
        return;
    }

    size_t idx;
    darray_search(entries, ent, compare_ptr, &idx);
    darray_pop(entries, idx);

    entry_deref_all(ent);
}

void command_purge(char *args, darray *snapshots, darray *entries) {

}

void command_set(char *args, darray *snapshots, darray *entries) {
    char *key = strsep(&args, WHITESPACE);
    if (key == NULL) {
        printf("missing key\n");
        return;
    }

    darray *elements;
    if ((elements = parse_elements(&args, entries)) == NULL) {
        return;
    }

    char exist;
    size_t idx;
    entry *ent;
    exist = darray_search(entries, key, (comparator) entry_has_key, &idx);
    if (exist) {
        darray_pop(entries, idx);
    } else {
        idx = 0;
    }

    ent = new_entry(key);

    if (!darray_extend(ent->elements, elements)) {
        printf("out of memory\n");
        return;
    }

    if (!darray_insert(entries, idx, ent)) {
        printf("out of memory\n");
        return;
    }

    entry_ref_all(ent, elements);

    del_darray(elements);
    printf("ok\n");
}

void command_push(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    darray *elements = parse_elements(&args, entries);

    darray_reverse(elements);
    if (!darray_extend_at(ent->elements, 0, elements)) {
        printf("out of memory\n");
        return;
    }

    entry_ref_all(ent, elements);

    del_darray(elements);
    printf("ok\n");
}

void command_append(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    darray *elements = parse_elements(&args, entries);
    if (!darray_extend(ent->elements, elements)) {
        printf("out of memory\n");
        return;
    }

    entry_ref_all(ent, elements);

    del_darray(elements);
    printf("ok\n");
}

void command_pick(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    size_t idx;

    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (!parse_index(args, ent->elements->len, &idx)) {
        printf("invalid index");
        return;
    }

    element_print(darray_get(ent->elements, idx));
    putchar('\n');
}

void command_pluck(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    size_t idx;

    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (!parse_index(args, ent->elements->len, &idx)) {
        printf("invalid index");
        return;
    }

    element *ele = darray_get(ent->elements, idx);
    element_print(ele);
    putchar('\n');

    if (ele != NULL && ele->type == ENTRY) {
        entry_del_ref(ent, ele->value.entry);
    }
    darray_pop(ent->elements, idx);
}

void command_pop(char *args, darray *snapshots, darray *entries) {
    entry *ent;

    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    element *ele = darray_get(ent->elements, 0);
    element_print(ele);
    putchar('\n');

    if (ele != NULL && ele->type == ENTRY) {
        entry_del_ref(ent, ele->value.entry);
    }
    darray_pop(ent->elements, 0);
}

void command_drop(char *args, darray *snapshots, darray *entries) {

}

void command_rollback(char *args, darray *snapshots, darray *entries) {

}

void command_checkout(char *args, darray *snapshots, darray *entries) {

}

void command_snapshot(char *args, darray *snapshots, darray *entries) {

}

void command_min(char *args, darray *snapshots, darray *entries) {

}

void command_max(char *args, darray *snapshots, darray *entries) {

}

void command_sum(char *args, darray *snapshots, darray *entries) {

}

void command_len(char *args, darray *snapshots, darray *entries) {

}

void command_rev(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (!entry_is_simple(ent)) {
        printf("entry is not simple\n");
    }

    darray_reverse(ent->elements);
    printf("ok\n");
}

void command_uniq(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (!entry_is_simple(ent)) {
        puts("entry is not simple");
    }

    darray_unique(ent->elements, (comparator) element_int_cmp);
    printf("ok\n");
}

void command_sort(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (!entry_is_simple(ent)) {
        puts("entry is not simple");
    }

    darray_sort(ent->elements, (comparator) element_int_cmp);
    printf("ok\n");
}

void command_forward(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (ent->forward->len == 0) {
        printf("nil\n");
    } else {
        darray *sorted = darray_clone(ent->forward, clone_ptr);
        darray_sort(sorted, (comparator) entry_key_cmp);
        darray_unique(sorted, (comparator) entry_key_cmp);
        darray_foreach(sorted, (consumer) entry_print_key);
        del_darray(sorted);
    }
}

void command_backward(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (ent->backward->len == 0) {
        printf("nil\n");
    } else {
        darray *sorted = darray_clone(ent->backward, clone_ptr);
        darray_sort(sorted, (comparator) entry_key_cmp);
        darray_unique(sorted, (comparator) entry_key_cmp);
        darray_foreach(sorted, (consumer) entry_print_key);
        del_darray(sorted);
    }
}

void command_type(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        return;
    }

    if (entry_is_simple(ent)) {
        puts("simple");
    } else {
        puts("general");
    }
}

/* Main program */

int main() {

    darray *snapshots = new_darray((consumer) del_snapshot);
    darray *entries = new_darray((consumer) del_entry);

    char buf[BUFLEN];
    char *args;
    char *comm;

    while (1) {
        printf("> ");

        if (fgets(buf, BUFLEN, stdin) == NULL) {
            break;
        }
        buf[strcspn(buf, "\n")] = 0;

        args = buf;
        comm = strsep(&args, WHITESPACE);

        if (strcasecmp(comm, "help") == 0) {
            command_help();
        } else if (strcasecmp(comm, "list") == 0) {
            command_list(args, snapshots, entries);
        } else if (strcasecmp(comm, "get") == 0) {
            command_get(args, snapshots, entries);
        } else if (strcasecmp(comm, "del") == 0) {
            command_del(args, snapshots, entries);
        } else if (strcasecmp(comm, "purge") == 0) {
            command_purge(args, snapshots, entries);
        } else if (strcasecmp(comm, "set") == 0) {
            command_set(args, snapshots, entries);
        } else if (strcasecmp(comm, "push") == 0) {
            command_push(args, snapshots, entries);
        } else if (strcasecmp(comm, "append") == 0) {
            command_append(args, snapshots, entries);
        } else if (strcasecmp(comm, "pick") == 0) {
            command_pick(args, snapshots, entries);
        } else if (strcasecmp(comm, "pluck") == 0) {
            command_pluck(args, snapshots, entries);
        } else if (strcasecmp(comm, "pop") == 0) {
            command_pop(args, snapshots, entries);
        } else if (strcasecmp(comm, "drop") == 0) {
            command_drop(args, snapshots, entries);
        } else if (strcasecmp(comm, "rollback") == 0) {
            command_rollback(args, snapshots, entries);
        } else if (strcasecmp(comm, "checkout") == 0) {
            command_checkout(args, snapshots, entries);
        } else if (strcasecmp(comm, "snapshot") == 0) {
            command_snapshot(args, snapshots, entries);
        } else if (strcasecmp(comm, "min") == 0) {
            command_min(args, snapshots, entries);
        } else if (strcasecmp(comm, "max") == 0) {
            command_max(args, snapshots, entries);
        } else if (strcasecmp(comm, "sum") == 0) {
            command_sum(args, snapshots, entries);
        } else if (strcasecmp(comm, "len") == 0) {
            command_len(args, snapshots, entries);
        } else if (strcasecmp(comm, "rev") == 0) {
            command_rev(args, snapshots, entries);
        } else if (strcasecmp(comm, "uniq") == 0) {
            command_uniq(args, snapshots, entries);
        } else if (strcasecmp(comm, "sort") == 0) {
            command_sort(args, snapshots, entries);
        } else if (strcasecmp(comm, "forward") == 0) {
            command_forward(args, snapshots, entries);
        } else if (strcasecmp(comm, "backward") == 0) {
            command_backward(args, snapshots, entries);
        } else if (strcasecmp(comm, "type") == 0) {
            command_type(args, snapshots, entries);
        } else if (strcasecmp(comm, "bye") == 0) {
            printf("bye\n");
            break;
        } else {
            printf("no such command\n");
        }

        putchar('\n');
    }

    del_darray(snapshots);
    del_darray(entries);

    return 0;
}
