#include "database/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// static MDB_env *MDB_env;
// static MDB_dbi mdb_dbi;

// int db_init(const char *path) {
//     int rc;

//     rc = mdb_env_create(&mdb_env);
//     if (rc) {
//         fprintf(stderr, "mdb_env_create: %s\n", mdb_strerror(rc));
//         return rc;
//     }

//     rc = mdb_env_open(mdb_env, path, MDB_WRITEMAP | MDB_NOSYNC, 0664);
//     if (rc) {
//         fprintf(stderr, "mdb_env_open: %s\n", mdb_strerror(rc));
//         return rc;       
//     }

//     MDB_txn *txn;
//     rc = mdb_txn_begin(mdb_env, NULL, 0, &txn);
//     if (rc) {
//         fprintf(stderr, "mdb_env_open: %s\n", mdb_strerror(rc));
//         return rc;       
//     }

//     rc = mdb_dbi_open(txn, NULL, 0, &mdb_dbi);
//     if (rc) {
//         fprintf(stderr, "mdb_env_create: %s\n", mdb_strerror(rc));
//         return rc;
//     }

//     printf("Database initialized successfully at %s\n", path);
// }

// void db_shutdown() {
//     if (mdb_env) {
//         mdb_dbi_close(mdb_env, mdb_dbi);
//         mdb_env_close(mdb_env);
//         mdb_env = NULL;
//         printf("Database shutdown complete.\n");
//     }
// }

// int db_put(const char *key, const char *value) {
//     MDB_txn *txn;
//     MDB_val mdb_key, mdb_data;
//     int rc;

//     rc = mdb_txn_begin(mdb_env, NULL, 0, &txn);
//     if (rc) return rc;

//     mdb_key.mv_size = strlen(key);
//     mdb_key.mv_data = (void *)key;
//     mdb_data.mv_size = strlen(value);
//     mdb_data.mv_data = (void *)(value);

//     rc = mdb_put(txn, mdb_dbi, &mdb_key, &mdb_data, 0);
//     if (rc) {
//         mdb_txn_abort(txn);
//         return rc;
//     }

//     rc = mdb_txn_commit(txn);
//     return rc;
// }

// char *db_get(const char *key) {
//     MDB_txn *txn;
//     MDB_val mdb_key, mdb_data;
//     int rc;
//     char *value = NULL;

//     rc = mdb_txn_begin(mdb_env, NULL, MDB_RONLY, &txn);
//     if (rc) return NULL;

//     mdb_key.mv_size = strlen(key);
//     mdb_key.mv_data = (void *)key;

//     rc = mdb_get(txn, mdb_dbi, &mdb_key, &mdb_data);
//     if (rc == 0) {
//         value = malloc(mdb_data.mv_size + 1);
//         if (value) {
//             memcpy(value, mdb_data.mv_data, mdb_data.mv_size);
//             value[mdb_data.mv_size] = '\0';
//         }
//     }

//     mdb_txn_abort(txn);
//     return value;
// }

// int db_delete(const char *key) {
//     MDB_txn *tnx;
//     MDB_val mdb_key;
//     int rc;

//     rc = mdb_txn_begin(mdb_env, NULL, 0, &txn);
//     if (rc) return rc;

//     mdb_key.mv_size = strlen(key);
//     mdb_key.mv_data = (void *)key;

//     rc = mdb_del(txn, mdb_dbi, &mdb_key, NULl);
//     if (rc) {
//         mdb_txn_abort(txn);
//         return rc;
//     }

//     return mdb_txn_commit(txn);
// }

