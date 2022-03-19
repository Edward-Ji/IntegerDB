#ifndef YMIRDB_H
#define YMIRDB_H

#define MAX_KEY 16
#define MAX_LINE 1024

#include <stddef.h>

typedef struct darray darray;

enum item_type { INTEGER, ENTRY };

typedef struct element element;
typedef struct entry entry;
typedef struct snapshot snapshot;

struct element {
    enum item_type type;
    union {
        int value;
        struct entry *entry;
    };
};

struct entry {
    char key[MAX_KEY];
    darray *elements;
    darray *forward;
    darray *backward;
};

#endif
