#include "database/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MDB_env *mdb_env = NULL;
static MDB_dbi mdb_dbi;

