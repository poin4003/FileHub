#ifndef DB_H
#define DB_H

#include <lmdb.h>

int db_init(const char *path);
void db_shutdown();

// --- API single transaction ---
int db_put(const char *key, const char *value);
char *db_get(const char *key); 
int db_delete(const char *key);

// --- API batch transaction ---
MDB_txn *db_begin(int readonly);
int db_txn_commit(MDB_txn *txn);
void db_txn_abort(MDB_txn *txn);

int db_put_txn(MDB_txn *txn, const char *key, const char *value);
char *db_get_copy_txn(MDB_txn );
#endif