/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Zhijian Yang  <snfnwgi@126.com>                              |
  +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_pwaf.h"

/* If you declare any globals in php_pwaf.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(pwaf)
 */
char *logfile = "/tmp/pwaf.log";
/* True global resources - no need for thread safety here */
static int le_pwaf;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("pwaf.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pwaf_globals, pwaf_globals)
	STD_PHP_INI_ENTRY("pwaf.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pwaf_globals, pwaf_globals)
PHP_INI_END()
 */
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */

/* {{{ proto string confirm_pwaf_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_pwaf_compiled) {
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "pwaf", arg);

	RETURN_STR(strg);
}

PHP_FUNCTION(say) {
	zend_string *strg;
	strg = strpprintf(0, "test");
	RETURN_STR(strg);
}

static void php_pwaf_fcall_check(zend_execute_data *ex, const zend_op *opline, zend_function *fbc) /* {{{ */ {
	int arg_count = ZEND_CALL_NUM_ARGS(ex);

	if (!arg_count) {
		return;
	}

	if (fbc->common.scope == NULL) {
		zend_string *fname = fbc->common.function_name;
		if ((fname, "exec")) {
			zval *p = ZEND_CALL_ARG(ex, 1);
			if (p && IS_STRING == Z_TYPE_P(p) && PWAF_POSSIBLE(Z_STR_P(p))) {
				php_printf("hello");
				char *file_name = zend_get_executed_filename(TSRMLS_C);
				char *function_name = "exec";
				int lineno = zend_get_executed_lineno(TSRMLS_C);
				pwaf_log(file_name, function_name, lineno);
			}
		}
	}
} /* }}} */

static int php_pwaf_call_handler(zend_execute_data *execute_data) /* {{{ */ {
	const zend_op *opline = execute_data->opline;
	zend_execute_data *call = execute_data->call;
	zend_function *fbc = call->func;
	if (fbc->type == ZEND_INTERNAL_FUNCTION) {
		php_pwaf_fcall_check(call, opline, fbc);
	}

	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

static void php_pwaf_register_handlers() /* {{{ */ {
	zend_set_user_opcode_handler(ZEND_DO_ICALL, php_pwaf_call_handler);
} /* }}} */

void pwaf_log(char *file_name, char *function_name, int lineno) {
	FILE *fh;
	fh = fopen(logfile, "ab+");
	time_t timeNow = time(NULL);
	struct tm* p = localtime(&timeNow);
	fprintf(fh, "[filename]: %s\n[function]: %s\n[line]: %d\n[date]:%d-%d-%d %d:%d:%d\n\n", file_name, function_name, lineno, p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	fclose(fh);
}

/* {{{ pwaf_module_entry
 */

/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
 */


/* {{{ php_pwaf_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_pwaf_init_globals(zend_pwaf_globals *pwaf_globals)
{
	pwaf_globals->global_value = 0;
	pwaf_globals->global_string = NULL;
}
 */
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pwaf) {
	php_pwaf_register_handlers();
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	 */
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pwaf) {
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	 */
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pwaf) {
#if defined(COMPILE_DL_PWAF) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pwaf) {
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pwaf) {
	php_info_print_table_start();
	php_info_print_table_header(2, "pwaf support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	 */
}
/* }}} */
/* {{{ pwaf_functions[]
 *
 * Every user visible function must have an entry in pwaf_functions[].
 */
const zend_function_entry pwaf_functions[] = {
	PHP_FE(say, NULL)
	PHP_FE(confirm_pwaf_compiled, NULL) /* For testing, remove later. */
	PHP_FE_END /* Must be the last line in pwaf_functions[] */
};

/* }}} */

zend_module_entry pwaf_module_entry = {
	STANDARD_MODULE_HEADER,
	"pwaf",
	pwaf_functions,
	PHP_MINIT(pwaf),
	PHP_MSHUTDOWN(pwaf),
	PHP_RINIT(pwaf), /* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(pwaf), /* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pwaf),
	PHP_PWAF_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PWAF
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(pwaf)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
