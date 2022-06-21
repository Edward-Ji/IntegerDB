/*
 * COMP2017 - assignment 2
 * Ziao Ji (Edward)
 * ziji4098
 */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "help.h"
#include "integerdb.h"

#define KEYLEN (16)
#define BUFLEN (1024)
#define WHITESPACE " \t\r\n\v\f"

/* Dynamic array of void pointers */

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
        while (len < cap / 2 && cap > 1) {
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

int _compare_voidp(const void *p1, const void *p2) {
    return p1 != p2;
}

int darray_pop_exact(darray *arrp, void *itemp) {
    if (arrp == NULL) {
        return 0;
    }

    size_t idx;
    if (!darray_search(arrp, itemp, _compare_voidp, &idx)) {
        return 0;
    }
    darray_pop(arrp, idx);

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

void _swap_voidp(void **pp1, void **pp2) {
    void *temp = *pp1;
    *pp1 = *pp2;
    *pp2 = temp;
}

ssize_t _partition(void **itempp, ssize_t low, ssize_t high, comparator cmp) {
    void *pivot = itempp[high];
    ssize_t i = low - 1;

    for (ssize_t j = low; j < high; j++) {
        if (cmp(itempp[j], pivot) < 0) {
            i++;
            _swap_voidp(itempp + i, itempp + j);
        }
    }
    _swap_voidp(itempp + (i + 1), itempp + high);
    return i + 1;
}

void _darray_qsort(void **itempp, ssize_t low, ssize_t high, comparator cmp) {
    if (low < high) {
        ssize_t pivot_i = _partition(itempp, low, high, cmp);

        _darray_qsort(itempp, low, pivot_i - 1, cmp);
        _darray_qsort(itempp, pivot_i + 1, high, cmp);
    }
}

int darray_sort(darray *arrp, comparator fp) {
    if (arrp == NULL || fp == NULL) {
        return 0;
    }

    if (arrp->len > 0) {
        _darray_qsort(arrp->itempp, 0, arrp->len - 1, fp);
    }

    return 1;
}

darray *darray_clone(darray *arrp, unary fp) {
    if (arrp == NULL || fp == NULL) {
        return NULL;
    }

    darray *clonep = (darray *) malloc(sizeof(darray));

    memcpy(clonep, arrp, sizeof(darray));

    clonep->itempp = (void **) malloc(sizeof(void *) * clonep->cap);
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
    size_t id;
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

int element_int_cmp(const element *ele1, const element *ele2) {
    int num1 = ele1->value.num;
    int num2 = ele2->value.num;
    if (num1 < num2) { return -1; }
    if (num1 > num2) { return 1; }
    return 0;
}

void element_agg_min(const element *ele, int *min) {
    int val;
    if (ele->type == INTEGER) {
        val = ele->value.num;
    } else {
        val = entry_min(ele->value.entry);
    }
    if (val < *min) {
        *min = val;
    }
}

void element_agg_max(const element *ele, int *max) {
    int val;
    if (ele->type == INTEGER) {
        val = ele->value.num;
    } else {
        val = entry_max(ele->value.entry);
    }
    if (val > *max) {
        *max = val;
    }
}

void element_agg_sum(const element *ele, long long *sum) {
    if (ele->type == INTEGER) {
        *sum += ele->value.num;
    } else {
        *sum += entry_sum(ele->value.entry);
    }
}

void element_agg_len(const element *ele, size_t *len) {
    if (ele->type == INTEGER) {
        (*len)++;
    } else {
        *len += entry_len(ele->value.entry);
    }
}

element *element_find_copy(element *ele) {
    if (ele->type == INTEGER) {
        return new_int_ele(ele->value.num);
    } else if (ele->type == ENTRY) {
        return new_ent_ele(entry_find_copy(ele->value.entry));
    }

    return NULL;
}

entry *new_entry(char *key) {
    entry *ent = (entry *) malloc(sizeof(entry));

    if (ent != NULL) {
        strncpy(ent->key, key, KEYLEN - 1);
        ent->elements = new_darray(free);
        ent->forward = new_darray(NULL);
        ent->backward = new_darray(NULL);
    }

    return ent;
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

int entry_has_key(const entry *ent, const char *key) {
    return strcmp(ent->key, key);
}

int entry_key_cmp(const entry *ent1, const entry *ent2) {
    return strcmp(ent1->key, ent2->key);
}

void entry_add_ref(entry *ent1, entry *ent2) {
    darray_append(ent1->forward, ent2);
    darray_extend(ent1->forward, ent2->forward);
    for (size_t i = 0; i < ent1->backward->len; i++) {
        entry *ent = darray_get(ent1->backward, i);
        darray_append(ent->forward, ent2);
        darray_extend(ent->forward, ent2->forward);
    }
    darray_append(ent2->backward, ent1);
    darray_extend(ent2->backward, ent1->backward);
    for (size_t i = 0; i < ent2->forward->len; i++) {
        entry *ent = darray_get(ent2->forward, i);
        darray_append(ent->backward, ent1);
        darray_extend(ent->backward, ent1->backward);
    }
}

void entry_del_ref(entry *ent1, entry *ent2) {
    darray_pop_exact(ent1->forward, ent2);
    for (size_t j = 0; j < ent2->forward->len; j++) {
        darray_pop_exact(ent1->forward, darray_get(ent2->forward, j));
    }
    for (size_t i = 0; i < ent1->backward->len; i++) {
        entry *enti = darray_get(ent1->backward, i);
        darray_pop_exact(enti->forward, ent2);
        for (size_t j = 0; j < ent2->forward->len; j++) {
            darray_pop_exact(enti->forward, darray_get(ent2->forward, j));
        }
    }
    darray_pop_exact(ent2->backward, ent1);
    for (size_t j = 0; j < ent1->backward->len; j++) {
        darray_pop_exact(ent2->backward, darray_get(ent1->backward, j));
    }
    for (size_t i = 0; i < ent2->forward->len; i++) {
        entry *enti = darray_get(ent2->forward, i);
        darray_pop_exact(enti->backward, ent1);
        for (size_t j = 0; j < ent1->backward->len; j++) {
            darray_pop_exact(enti->backward, darray_get(ent1->backward, j));
        }
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

int entry_min(entry *ent) {
    int min = INT_MAX;
    darray_aggregate(ent->elements, &min, (aggregate) element_agg_min);

    return min;
}

int entry_max(entry *ent) {
    int max = INT_MIN;
    darray_aggregate(ent->elements, &max, (aggregate) element_agg_max);

    return max;
}

long long entry_sum(entry *ent) {
    long long sum = 0;
    darray_aggregate(ent->elements, &sum, (aggregate) element_agg_sum);

    return sum;
}

size_t entry_len(entry *ent) {
    size_t len = 0;
    darray_aggregate(ent->elements, &len, (aggregate) element_agg_len);

    return len;
}

entry *entry_empty_copy(entry *ent) {
    entry *cpy = (entry *) malloc(sizeof(entry));
    strcpy(cpy->key, ent->key);

    return cpy;
}

entry *_entry_find_copy(entry *ent, darray *entries) {
    static darray *pool = NULL;
    if (ent == NULL) {
        pool = entries;
    }

    size_t idx;
    if (!darray_search(pool, ent->key, (comparator) entry_has_key, &idx)) {
        return NULL;
    }

    return darray_get(pool, idx);
}

void entry_find_pool(darray *pool) {
    _entry_find_copy(NULL, pool);
}

entry *entry_find_copy(entry *ent) {
    return _entry_find_copy(ent, NULL);
}

void del_entry(entry *ent) {
    del_darray(ent->elements);
    del_darray(ent->forward);
    del_darray(ent->backward);

    free(ent);
}

void print_entry_list(darray *entries) {
    entry *ent_ref;
    if (entries->len != 0) {
        ent_ref = darray_get(entries, 0);
        fputs(ent_ref->key, stdout);
        for (size_t i = 1; i < entries->len; i++) {
            printf(", ");
            ent_ref = darray_get(entries, i);
            fputs(ent_ref->key, stdout);
        }
    } else {
        printf("nil");
    }

    putchar('\n');
}

int entries_can_purge_key(darray *entries, char *key) {
    entry *ent;
    size_t idx;
    if (!darray_search(entries, key, (comparator) entry_has_key, &idx)) {
        return 1;
    }
    ent = darray_get(entries, idx);
    if (ent->backward->len != 0) {
        return 0;
    }

    return 1;
}

void entries_purge_key(darray *entries, char *key) {
    entry *ent;
    size_t idx;
    if (!darray_search(entries, key, (comparator) entry_has_key, &idx)) {
        return;
    }
    ent = darray_get(entries, idx);
    darray_search(entries, ent, compare_ptr, &idx);
    entry_deref_all(ent);
    darray_pop(entries, idx);
}

darray *entries_clone(darray *entries) {
    darray *clone = darray_clone(entries, (unary) entry_empty_copy);

    entry_find_pool(clone);

    for (size_t i = 0; i < entries->len; i++) {
        entry *ent_ori = darray_get(entries, i);
        entry *ent_cpy = darray_get(clone, i);

        ent_cpy->elements = darray_clone(ent_ori->elements,
                (unary) element_find_copy);

        ent_cpy->forward = darray_clone(ent_ori->forward,
                (unary) entry_find_copy);
        ent_cpy->backward = darray_clone(ent_ori->backward,
                (unary) entry_find_copy);
    }
    entry_find_pool(NULL);

    return clone;
}

snapshot *new_snapshot(darray *entries) {
    static size_t new_id = 1;
    snapshot *snap = (snapshot *) malloc(sizeof(snapshot));

    if (snap != NULL) {
        snap->id = new_id++;
        snap->entries = entries_clone(entries);
    }

    return snap;
}

void snapshot_print(snapshot *snap) {
    printf("%zu\n", snap->id);
}

int snapshot_has_id(const snapshot *snap, const size_t *id) {
    return snap->id != *id;
}

void del_snapshot(snapshot *snap) {
    del_darray(snap->entries);
    free(snap);
}

/* Helper parsers */

int parse_int(char *str, int *resp) {
    char *end;
    long num = strtol(str, &end, 10);
    if (end == str || *end != '\0' || num < INT_MIN || num > INT_MAX) {
        return 0;
    }
    *resp = num;
    return 1;
}

int parse_index(char *str, size_t max, size_t *resp) {
    if (str == NULL) {
        return 0;
    }

    char *end;
    unsigned long num = strtoul(str, &end, 10);
    if (end == str || *end != '\0' || num == 0 || num > max) {
        return 0;
    }
    *resp = num;
    return 1;
}

darray *parse_elements(char **strp, darray *entries, entry *self) {
    darray *elements = new_darray(free);

    char *token;
    size_t idx;
    while ((token = strsep(strp, WHITESPACE)) != NULL) {
        int num;
        element *ele;
        if (isdigit(*token) || *token == '-') {
            if (parse_int(token, &num)) {
                ele = new_int_ele(num);
            } else {
                printf("invalid integer\n");
                del_darray(elements);
                return NULL;
            }
        }
        else {
            if (strcmp(token, self->key) == 0) {
                printf("not permitted\n");
                del_darray(elements);
                return NULL;
            }
            if (!darray_search(entries, token, (comparator) entry_has_key, &idx)) {
                printf("no such key\n");
                del_darray(elements);
                return NULL;
            }
            entry *ent = darray_get(entries, idx);
            ele = new_ent_ele(ent);
        }
        darray_append(elements, ele);
    }

    darray_set_item_free(elements, NULL);

    return elements;
}

entry *parse_entry(char **strp, darray *entries) {
    char *key = strsep(strp, WHITESPACE);
    size_t idx;
    if (!darray_search(entries, key, (comparator) entry_has_key, &idx)) {
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
        if (entries->len == 0) {
            printf("no keys\n");
        } else {
            darray_foreach(entries, (consumer) entry_print_key);
        }
    } else if (strcasecmp(what, "entries") == 0) {
        if (entries->len == 0) {
            printf("no entries\n");
        } else {
        darray_foreach(entries, (consumer) entry_print);
        }
    } else if (strcasecmp(what, "snapshots") == 0) {
        if (snapshots->len == 0) {
            printf("no snapshots\n");
        } else {
            darray_foreach(snapshots, (consumer) snapshot_print);
        }
    } else {
        printf("invalid list command\n");
    }
}

void command_get(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }
    entry_print_nokey(ent);
}

void command_del(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }
    if (ent->backward->len != 0) {
        printf("not permitted\n");
        return;
    }

    size_t idx;
    darray_search(entries, ent, compare_ptr, &idx);
    entry_deref_all(ent);
    darray_pop(entries, idx);

    printf("ok\n");
}

void command_purge(char *args, darray *snapshots, darray *entries) {
    char *key = strsep(&args, WHITESPACE);

    if (!entries_can_purge_key(entries, key)) {
        printf("not permitted\n");
        return;
    }
    for (size_t i = 0; i < snapshots->len; i++) {
        snapshot *snap = darray_get(snapshots, i);
        if (!entries_can_purge_key(snap->entries, key)) {
            printf("not permitted\n");
            return;
        }
    }

    entries_purge_key(entries, key);
    for (size_t i = 0; i < snapshots->len; i++) {
        snapshot *snap = darray_get(snapshots, i);
        entries_purge_key(snap->entries, key);
    }

    printf("ok\n");
}

void command_set(char *args, darray *snapshots, darray *entries) {
    char *key = strsep(&args, WHITESPACE);
    if (key == NULL) {
        printf("missing key\n");
        return;
    }

    char exist;
    size_t idx = 0;
    entry *ent;
    exist = darray_search(entries, key, (comparator) entry_has_key, &idx);
    if (exist) {
        ent = darray_get(entries, idx);
        entry_deref_all(ent);
        darray_clear(ent->elements);
    } else {
        ent = new_entry(key);
    }

    darray *elements;
    char error = 0;
    if ((elements = parse_elements(&args, entries, ent)) == NULL) {
        error = 1;
    }

    if (!darray_extend(ent->elements, elements)) {
        printf("out of memory\n");
        error = 1;
    }

    if (!exist && !darray_insert(entries, idx, ent)) {
        printf("out of memory\n");
        error = 1;
    }

    if (error) {
        del_darray(elements);
        if (!exist) {
            del_entry(ent);
        }
        return;
    }

    entry_ref_all(ent, elements);

    del_darray(elements);
    printf("ok\n");
}

void command_push(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    darray *elements = parse_elements(&args, entries, ent);

    darray_reverse(elements);
    if (!darray_extend_at(ent->elements, 0, elements)) {
        printf("out of memory\n");
        del_darray(elements);
        return;
    }

    entry_ref_all(ent, elements);

    del_darray(elements);
    printf("ok\n");
}

void command_append(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    darray *elements = parse_elements(&args, entries, ent);
    if (elements == NULL) {
        return;
    }
    if (!darray_extend(ent->elements, elements)) {
        printf("out of memory\n");
        del_darray(elements);
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
        printf("no such key\n");
        return;
    }

    if (!parse_index(args, ent->elements->len, &idx)) {
        printf("index out of range\n");
        return;
    }
    idx--;

    element_print(darray_get(ent->elements, idx));
    putchar('\n');
}

void command_pluck(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    size_t idx;

    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    if (!parse_index(args, ent->elements->len, &idx)) {
        printf("index out of range\n");
        return;
    }
    idx--;

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
        printf("no such key\n");
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
    size_t idx, snap_idx = 0;

    if (!parse_index(args, -1, &idx)) {
        printf("index out of range\n");
        return;
    }
    if (!darray_search(snapshots,
                &idx, (comparator) snapshot_has_id, &snap_idx)) {
        printf("no such snapshot\n");
        return;
    }

    darray_pop(snapshots, snap_idx);

    printf("ok\n");
}

void command_rollback(char *args, darray *snapshots, darray *entries) {
    size_t idx, snap_idx = 0;

    if (!parse_index(args, -1, &idx)) {
        printf("index out of range\n");
        return;
    }
    if (!darray_search(snapshots,
                &idx, (comparator) snapshot_has_id, &snap_idx)) {
        printf("no such snapshot\n");
        return;
    }

    snapshot *snap = darray_get(snapshots, snap_idx);
    darray *clone = entries_clone(snap->entries);
    darray_clear(entries);
    darray_extend(entries, clone);
    darray_set_item_free(clone, NULL);
    del_darray(clone);

    darray_pop_range(snapshots, 0, snap_idx);

    printf("ok\n");
}

void command_checkout(char *args, darray *snapshots, darray *entries) {
    size_t idx, snap_idx = 0;

    if (!parse_index(args, -1, &idx)) {
        printf("index out of range\n");
        return;
    }
    if (!darray_search(snapshots,
                &idx, (comparator) snapshot_has_id, &snap_idx)) {
        printf("no such snapshot\n");
        return;
    }

    snapshot *snap = darray_get(snapshots, snap_idx);
    darray *clone = entries_clone(snap->entries);
    darray_clear(entries);
    darray_extend(entries, clone);
    darray_set_item_free(clone, NULL);
    del_darray(clone);

    printf("ok\n");
}

void command_snapshot(char *args, darray *snapshots, darray *entries) {
    snapshot *snap = new_snapshot(entries);
    darray_insert(snapshots, 0, snap);

    printf("saved as snapshot ");
    snapshot_print(snap);
}

void command_min(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }
    printf("%d\n", entry_min(ent));
}

void command_max(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }
    printf("%d\n", entry_max(ent));
}

void command_sum(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }
    printf("%lld\n", entry_sum(ent));
}

void command_len(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }
    printf("%zu\n", entry_len(ent));
}

void command_rev(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    if (!entry_is_simple(ent)) {
        printf("entry is not simple\n");
        return;
    }

    darray_reverse(ent->elements);
    printf("ok\n");
}

void command_uniq(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    if (!entry_is_simple(ent)) {
        puts("entry is not simple");
        return;
    }

    darray_unique(ent->elements, (comparator) element_int_cmp);
    printf("ok\n");
}

void command_sort(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    if (!entry_is_simple(ent)) {
        puts("entry is not simple");
        return;
    }

    darray_sort(ent->elements, (comparator) element_int_cmp);
    printf("ok\n");
}

void command_forward(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    if (ent->forward->len == 0) {
        printf("nil\n");
    } else {
        darray *sorted = darray_clone(ent->forward, clone_ptr);
        darray_sort(sorted, (comparator) entry_key_cmp);
        darray_unique(sorted, (comparator) entry_key_cmp);

        print_entry_list(sorted);

        del_darray(sorted);
    }
}

void command_backward(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
        return;
    }

    if (ent->backward->len == 0) {
        printf("nil\n");
    } else {
        darray *sorted = darray_clone(ent->backward, clone_ptr);
        darray_sort(sorted, (comparator) entry_key_cmp);
        darray_unique(sorted, (comparator) entry_key_cmp);

        print_entry_list(sorted);

        del_darray(sorted);
    }
}

void command_type(char *args, darray *snapshots, darray *entries) {
    entry *ent;
    if ((ent = parse_entry(&args, entries)) == NULL) {
        printf("no such key\n");
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