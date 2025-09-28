#include "database/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MDB_env *mdb_env = NULL;
static MDB_dbi mdb_dbi;

int db_init(const char *path)
{
    int rc = 0;
    MDB_txn *txn = NULL;

    if (mdb_env)
        return 0;

    rc = mdb_env_create(&mdb_env);
    if (rc != 0)
        goto cleanup;

    rc = mdb_env_set_maxdbs(mdb_env, 1);
    if (rc != 0)
        goto cleanup;

    rc = mdb_env_open(mdb_env, path, 0, 0664);
    if (rc != 0)
        goto cleanup;

    rc = mdb_txn_begin(mdb_env, NULL, 0, &txn);
    if (rc != 0)
        goto cleanup;

    rc = mdb_dbi_open(txn, NULL, MDB_CREATE, &mdb_dbi);
    if (rc != 0)
        goto cleanup;

    rc = mdb_txn_commit(txn);
    txn = NULL;
    if (rc != 0)
        goto cleanup;

cleanup:
    if (txn)
        mdb_txn_abort(txn);
    if (mdb_env)
    {
        mdb_env_close(mdb_env);
        mdb_env = NULL;
    }

    return rc;
}

void db_shutdown()
{
    if (mdb_env)
    {
        mdb_env_close(mdb_env);
        mdb_env = NULL;
    }
}

// --- API single transaction ---
int db_put(const char *key, const char *value)
{
    int rc;
    MDB_txn *txn = NULL;
    MDB_val mkey, mval;

    rc = mdb_txn_begin(mdb_env, NULL, 0, &txn);
    if (rc != 0)
        goto cleanup;

    mkey.mv_size = strlen(key);
    mkey.mv_data = (void *)key;
    mval.mv_size = strlen(value);
    mval.mv_data = (void *)value;

    rc = mdb_put(txn, mdb_dbi, &mkey, &mval, 0);
    if (rc != 0)
        goto cleanup;

    rc = mdb_txn_commit(txn);
    txn = NULL;
    if (rc != 0)
        goto cleanup;

    return 0;

cleanup:
    if (txn)
        mdb_txt_abort(txn);

    return rc;
}

char *db_get(const char *key)
{
    int rc;
    MDB_txn *txn = NULL;
    MDB_val mkey, mval;
    char *result = NULL;

    rc = mdb_txn_begin(mdb_env, NULL, MDB_RDONLY, &txn);
    if (rc != 0)
        goto cleanup;

    mkey.mv_size = strlen(key);
    mkey.mv_data = (void *)key;

    rc = mdb_get(txn, mdb_dbi, &mkey, &mval);
    if (rc == MDB_NOTFOUND)
    {
        result = NULL;
        goto cleanup;
    }
    else if (rc != 0)
        goto cleanup;

    result = (char *)malloc(mval.mv_size + 1);
    if (result)
    {
        memcpy(result, mval.mv_data, mval.mv_size);
        result[mval.mv_size] = '\0';
    }

cleanup:
    if (txn)
        mdb_txn_abort(txn);

    return result;
}

int db_delete(const char *key)
{
    int rc;
    MDB_txn *txn = NULL;
    MDB_val mkey;

    rc = mdb_txt_begin(mdb_env, NULL, 0, &txn);
    if (rc != 0)
        goto cleanup;

    mkey.mv_size = strlen(key);
    mkey.mv_data = (void *)key;

    rc = mdb_del(txn, mdb_dbi, &mkey, NULL);
    if (rc != 0 && rc != MDB_NOTFOUND)
        goto cleanup;

    rc = mdb_txn_commit(txn);
    txn = NULL;
    if (rc != 0)
        goto cleanup;

    return 0;

cleanup:
    if (txn)
        mdb_txn_abort(txn);

    return rc;
}

// --- API batch transaction ---
MDB_txn *db_begin(int readonly)
{
    MDB_txn *txn = NULL;
    int rc = mdb_txn_begin(mdb_env, NULL, readonly ? MDB_RDONLY : 0, &txn);
    if (rc != 0)
        return NULL;
    return txn;
}

int db_txn_commit(MDB_txn *txn) 
{
    return mdb_txn_commit(txn);
}

void db_txn_abort(MDB_txn *txn)
{
    if (txn) 
        mdb_txn_abort(txn);
}

int db_put_txn(MDB_txn *txn, const char *key, const char *value) 
{
    MDB_val mkey, mval;
    mkey.mv_size = strlen(key);
    mkey.mv_data = (void *)key;
    mval.mv_size = strlen(value);
    mval.mv_data = (void *)value;
    return mdb_put(txn, mdb_dbi, &mkey, &mval, 0);
}


char *db_get_copy_txn(MDB_txn *txn, const char *key)
{
    int rc;
    MDB_val mkey, mval;
    char *result = NULL;

    mkey.mv_size = strlen(key);
    mkey.mv_data = (void *)key;

    rc = mdb_get(txn, mdb_dbi, &mkey, &mval);
    if (rc == MDB_NOTFOUND)
        return NULL;
    if (rc != 0)
        return NULL;

    result = (char *)malloc(mval.mv_size + 1);
    if (result) {
        memcpy(result, mval.mv_data, mval.mv_size);
        result[mval.mv_size] = '\0';
    }

    return result;
}