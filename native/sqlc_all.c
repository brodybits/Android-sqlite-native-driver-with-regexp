
#include "sqlite3.c"

#include "sqlc.h" /* types needed for SQLiteNative_JNI.c */

#include "SQLiteNative_JNI.c"

#include "regex.h"

#include <stdbool.h>

static pthread_mutex_t mylock;

// THANKS for guidance:
// - https://github.com/ralight/sqlite3-pcre
// - http://git.altlinux.org/people/at/packages/?p=sqlite3-pcre.git
// (public domain)

void regexp(sqlite3_context * c, int ac, sqlite3_value ** args)
{
  char * r;
  const char * t;
  const char * comp_err;
  int res = -1;

  if (ac < 2) {
    sqlite3_result_error(c, "SORRY need 2 arguments", -1);
    return;
  }

  r = sqlite3_value_text(args[0]);
  t = sqlite3_value_text(args[1]);

  if (!r || !t) {
    sqlite3_result_error(c, "SORRY invalid argument", -1);
    return;
  }

  /* lock needed since the regex library is NOT re-entrant */
  pthread_mutex_lock(&mylock);

  comp_err = re_comp(r);

  if (!!comp_err) {
    sqlite3_result_error(c, "SORRY invalid argument", -1);
    return;
  }

  res = re_exec(t);

  pthread_mutex_unlock(&mylock);

  sqlite3_result_int(c, res);
}

// XXX TODO MOVE:
void sqlite3_db_ext_init(sqlite3 * db)
{
  static bool is_mylock_valid = false;

  if (!is_mylock_valid) {
    pthread_mutex_init(&mylock, NULL);
    is_mylock_valid = true;
  }

  sqlite3_create_function(db, "REGEXP", 2, SQLITE_UTF8, NULL, regexp, NULL, NULL);
}

#include "sqlc.c"

