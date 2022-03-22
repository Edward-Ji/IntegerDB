/*
 * COMP2017 - assignment 2
 * Ziao Ji (Edward)
 * ziji4098
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "help.h"

#define KEYLEN (16)
#define BUFLEN (1024)
#define WHITESPACE " \t\r\n\v\f"

/* Dynamic array */

typedef void (*consumer)(void *);
typedef void (*aggregate)(void *, void *);
typedef int (*comparator)(void *, void *);

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

void darray_foreach(darray *arrp, consumer fp) {
    if (arrp == NULL || fp == NULL) {
        return;
    }

    for (size_t i = 0; i < arrp->len; i++) {
        fp(arrp->itempp[i]);
    }
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
            sizeof(void *) * (arrp->len - index));
    if (!_darray_resize(arrp, arrp->len - 1)) {
        return 0;
    }

    arrp->len--;

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
        ele->type = INTEGER;
        ele->value.entry = ent;
    }

    return ele;
}

void element_print(element *ele) {
    switch (ele->type) {
        case INTEGER:
            printf("%d", ele->value.num);
            break;
        case ENTRY:
            printf("%s", ele->value.entry->key);
            break;
        default:
            printf("invalid element type");
    }
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
        printf("invalid list command");
    }
}

void command_get(char *args, darray *snapshots, darray *entries) {
    char *key = strsep(&args, WHITESPACE);
    size_t idx;
    if (!darray_search(entries, key, (comparator) entry_has_key, &idx)) {
        printf("key not found");
    }

    entry *ent;
    ent = darray_get(entries, idx);
    entry_print_nokey(ent);
}

void command_del(char *args, darray *snapshots, darray *entries) {

}

void command_purge(char *args, darray *snapshots, darray *entries) {

}

void command_set(char *args, darray *snapshots, darray *entries) {
    char *key = strsep(&args, WHITESPACE);
    if (key == NULL) {
        printf("missing key\n");
        return;
    }

    // Check existing entry with key.
    char exist;
    size_t idx;
    entry *ent;
    exist = darray_search(entries, key, (comparator) entry_has_key, &idx);
    if (exist) {
        ent = darray_get(entries, idx);
        darray_clear(ent->elements);
    } else {
        ent = new_entry(key);
    }

    char *token;
    char *temp;
    long num;
    while ((token = strsep(&args, WHITESPACE)) != NULL) {
        num = strtol(token, &temp, 10);
        if (temp == token || *temp != '\0' || num < INT_MIN || num > INT_MAX) {
            printf("invalid integer\n");
            return;
        }
        darray_append(ent->elements, new_int_ele((int) num));
    }

    if (!exist) {
        if (!darray_insert(entries, 0, ent)) {
            printf("failed\n");
        }
    }

    printf("ok\n");
}

void command_push(char *args, darray *snapshots, darray *entries) {
    char *key = strsep(&args, WHITESPACE);
    size_t idx;
    if (!darray_search(entries, key, (comparator) entry_has_key, &idx)) {
        printf("key not found");
    }

    entry *ent;
    ent = darray_get(entries, idx);

    char *token;
    char *temp;
    long num;
    if ((token = strsep(&args, WHITESPACE)) != NULL) {
        num = strtol(token, &temp, 10);
        if (temp == token || *temp != '\0' || num < INT_MIN || num > INT_MAX) {
            printf("invalid integer\n");
            return;
        }
        darray_insert(ent->elements, 0, new_int_ele((int) num));
    }
}

void command_append(char *args, darray *snapshots, darray *entries) {
    char *key = strsep(&args, WHITESPACE);
    size_t idx;
    if (!darray_search(entries, key, (comparator) entry_has_key, &idx)) {
        printf("key not found");
    }

    entry *ent;
    ent = darray_get(entries, idx);

    char *token;
    char *temp;
    long num;
    if ((token = strsep(&args, WHITESPACE)) != NULL) {
        num = strtol(token, &temp, 10);
        if (temp == token || *temp != '\0' || num < INT_MIN || num > INT_MAX) {
            printf("invalid integer\n");
            return;
        }
        darray_append(ent->elements, new_int_ele((int) num));
    }
}

void command_pick(char *args, darray *snapshots, darray *entries) {

}

void command_pluck(char *args, darray *snapshots, darray *entries) {

}

void command_pop(char *args, darray *snapshots, darray *entries) {

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

}

void command_uniq(char *args, darray *snapshots, darray *entries) {

}

void command_sort(char *args, darray *snapshots, darray *entries) {

}

void command_forward(char *args, darray *snapshots, darray *entries) {

}

void command_backward(char *args, darray *snapshots, darray *entries) {

}

void command_type(char *args, darray *snapshots, darray *entries) {

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
