/**
 * COMP2017 - assignment 2
 * Ziao Ji (Edward)
 * ziji4098
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "help.h"
#include "ymirdb.h"

/*
 * Dyanmic Array
 */
struct darray {
    void **_data;
    size_t _capacity;
    size_t _t_size;
    size_t length;
};

darray *new_darray(size_t t_size) {
    darray *arr = (darray *) malloc(sizeof(darray));

    if (arr == NULL) {
        return arr;
    }
    arr->_capacity = 1;
    arr->_t_size = t_size;
    arr->length = 0;

    arr->_data = NULL;

    return arr;
}

bool _darray_resize(darray *arr) {
    void *data = arr->_data;
    size_t length = arr->length;
    size_t capacity = arr->_capacity;

    if (length > capacity) {
        while (length > capacity) {
            capacity *= 2;
        }
    } else {
        while (length < capacity) {
            capacity /= 2;
        }
    }
    if (capacity != arr->_capacity) {
        data = realloc(data, arr->_t_size * arr->_capacity);
        if (data == NULL) {
            return false;
        }
        arr->_data = data;
        arr->length = length;
        arr->_capacity = capacity;
    }

    return true;
}

bool darray_append(darray *arr, void *item) {
    arr->length++;
    if (!_darray_resize(arr)) {
        return false;
    }

    void *copy = malloc(arr->_t_size);
    if (copy == NULL) {
        return false;
    }
    memcpy(copy, item, arr->_t_size);
    arr->_data[arr->length - 1] = copy;

    return true;
}

void darray_clear(darray *arr) {
    for (size_t i = 0; i < arr->length; i++) {
        free(arr->_data[i]);
    }
    arr->length = 0;
}

void del_darray(darray **arr) {
    darray_clear(*arr);
    free(*arr);
}

void command_bye() {
    printf("bye\n");
}

void command_help() {
    printf("%s\n", HELP_STRING);
}

int main() {

    char line[MAX_LINE];

    while (1) {
        printf("> ");

        if (NULL == fgets(line, MAX_LINE, stdin)) {
            printf("\n");
            command_bye();
            return 0;
        }

    }

    return 0;
}
