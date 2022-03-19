#ifndef HELP_H
#define HELP_H

#define HELP_STRING "BYE     clear database and exit\n" \
    "HELP    display this help message\n" \
    "\n" \
    "LIST KEYS             displays all keys in current state\n" \
    "LIST ENTRIES        displays all entries in current state\n" \
    "LIST SNAPSHOTS    displays all snapshots in the database\n" \
    "\n" \
    "GET <key>        displays entry values\n" \
    "DEL <key>        deletes entry from current state\n" \
    "PURGE <key>    deletes entry from current state and snapshots\n" \
    "\n" \
    "SET <key> <value ...>         sets entry values\n" \
    "PUSH <key> <value ...>        pushes values to the front\n" \
    "APPEND <key> <value ...>    appends values to the back\n" \
    "\n" \
    "PICK <key> <index>     displays value at index\n" \
    "PLUCK <key> <index>    displays and removes value at index\n" \
    "POP <key>                        displays and removes the front value\n" \
    "\n" \
    "DROP <id>            deletes snapshot\n" \
    "ROLLBACK <id>    restores to snapshot and deletes newer snapshots\n" \
    "CHECKOUT <id>    replaces current state with a copy of snapshot\n" \
    "SNAPSHOT             saves the current state as a snapshot\n" \
    "\n" \
    "MIN <key>    displays minimum value\n" \
    "MAX <key>    displays maximum value\n" \
    "SUM <key>    displays sum of values\n" \
    "LEN <key>    displays number of values\n" \
    "\n" \
    "REV <key>     reverses order of values (simple entry only)\n" \
    "UNIQ <key>    removes repeated adjacent values (simple entry only)\n" \
    "SORT <key>    sorts values in ascending order (simple entry only)\n" \
    "\n" \
    "FORWARD <key> lists all the forward references of this key\n" \
    "BACKWARD <key> lists all the backward references of this key\n" \
    "TYPE <key> displays if the entry of this key is simple or general\n"

#endif
