/*
  Copyright (C) 2016 by Syohei YOSHIDA

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdlib.h>
#include <emacs-module.h>

#include <magic.h>

struct magic_flag_entry {
	emacs_value symbol;
	int value;
};

int plugin_is_GPL_compatible;

#define MAGIC_FLAG_MAX 13
static struct magic_flag_entry magic_flag_table[MAGIC_FLAG_MAX];

static char*
retrieve_string(emacs_env *env, emacs_value str, ptrdiff_t *size)
{
	*size = 0;

	env->copy_string_contents(env, str, NULL, size);
	char *p = malloc(*size);
	if (p == NULL) {
		*size = 0;
		return NULL;
	}
	env->copy_string_contents(env, str, p, size);

	return p;
}

static void
init_flag_table(emacs_env *env)
{
	int magic_flags[] = {
		MAGIC_NONE,
		MAGIC_DEBUG,
		MAGIC_SYMLINK,
		MAGIC_COMPRESS,
		MAGIC_DEVICES,
		MAGIC_MIME_TYPE,
		MAGIC_MIME_ENCODING,
		MAGIC_MIME,
		MAGIC_CONTINUE,
		MAGIC_CHECK,
		MAGIC_PRESERVE_ATIME,
		MAGIC_RAW,
		MAGIC_ERROR,
	};

	emacs_value magic_symbols[MAGIC_FLAG_MAX];
	magic_symbols[0] = env->intern(env, "none");
	magic_symbols[1] = env->intern(env, "debug");
	magic_symbols[2] = env->intern(env, "symlink");
	magic_symbols[3] = env->intern(env, "compress");
	magic_symbols[4] = env->intern(env, "devices");
	magic_symbols[5] = env->intern(env, "mime_type");
	magic_symbols[6] = env->intern(env, "mime_encoding");
	magic_symbols[7] = env->intern(env, "mime");
	magic_symbols[8] = env->intern(env, "continue");
	magic_symbols[9] = env->intern(env, "check");
	magic_symbols[10] = env->intern(env, "preserve_atime");
	magic_symbols[11] = env->intern(env, "raw");
	magic_symbols[12] = env->intern(env, "error");

	for (int i = 0; i < MAGIC_FLAG_MAX; ++i) {
		magic_flag_table[i].symbol = magic_symbols[i];
		magic_flag_table[i].value = magic_flags[i];
	}
}

static int
symbol_to_magic_value(emacs_env *env, emacs_value symbol)
{
	for (size_t i = 0; i < MAGIC_FLAG_MAX; ++i) {
		if (env->eq(env, symbol, magic_flag_table[i].symbol)) {
			return magic_flag_table[i].value;
		}
	}

	return 0;
}

static int
to_magic_flag(emacs_env *env, emacs_value flags)
{
	int flag = 0;

	ptrdiff_t len = env->vec_size(env, flags);
	for (ptrdiff_t i = 0; i < len; ++i) {
		emacs_value v = env->vec_get(env, flags, i);
		flag |= symbol_to_magic_value(env, v);
	}

	return flag;
}

static struct magic_set*
magic_init(emacs_env *env, emacs_value flag)
{
	int flags = to_magic_flag(env, flag);
	struct magic_set *magic = magic_open(flags);

	if (magic == NULL)
		return NULL;


	int ret = magic_load(magic, NULL);
	if (ret == -1) {
		magic_close(magic);
		return NULL;
	}

	return magic;
}

static emacs_value
Fmagic_file(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	emacs_value retval;
	struct magic_set *magic = magic_init(env, args[1]);
	if (magic == NULL) {
		return env->intern(env, "nil");
	}


	ptrdiff_t namelen;
	char *filename = retrieve_string(env, args[0], &namelen);
	const char *file_magic = magic_file(magic, filename);
	if (file_magic == NULL) {
		retval = env->intern(env, "nil");
		goto error;
	}

	retval = env->make_string(env, file_magic, strlen(file_magic));

error:
	magic_close(magic);

	return retval;
}

static emacs_value
Fmagic_buffer(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	emacs_value retval;
	struct magic_set *magic = magic_init(env, args[1]);
	if (magic == NULL) {
		return env->intern(env, "nil");
	}

	ptrdiff_t namelen;
	char *str = retrieve_string(env, args[0], &namelen);
	const char *str_magic = magic_buffer(magic, str, (size_t)namelen);
	if (str_magic == NULL) {
		retval = env->intern(env, "nil");
		goto error;
	}

	retval = env->make_string(env, str_magic, strlen(str_magic));

error:
	magic_close(magic);
	return retval;
}

/* Bind NAME to FUN.  */
static void
bind_function(emacs_env *env, const char *name, emacs_value Sfun)
{
	emacs_value Qfset = env->intern (env, "fset");
	emacs_value Qsym = env->intern (env, name);
	emacs_value args[] = { Qsym, Sfun };

	env->funcall(env, Qfset, 2, args);
}

/* Provide FEATURE to Emacs.  */
static void
provide(emacs_env *env, const char *feature)
{
	emacs_value Qfeat = env->intern (env, feature);
	emacs_value Qprovide = env->intern (env, "provide");
	emacs_value args[] = { Qfeat };

	env->funcall(env, Qprovide, 1, args);
}

int
emacs_module_init (struct emacs_runtime *ert)
{
	emacs_env *env = ert->get_environment (ert);
	bind_function(env, "magic-core-file", env->make_function(env, 2, 2, Fmagic_file, NULL, NULL));
	bind_function(env, "magic-core-buffer", env->make_function(env, 2, 2, Fmagic_buffer, NULL, NULL));

	init_flag_table(env);

	provide(env, "magic-core");
	return 0;
}

/*
  Local Variables:
  c-basic-offset: 8
  indent-tabs-mode: t
  End:
*/
