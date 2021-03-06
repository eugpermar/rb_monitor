/*
  Copyright (C) 2016 Eneo Tecnologia S.L.
  Copyright (C) 2017 Eugenio Perez
  Author: Eugenio Perez <eupm90@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "json_test.h"

#include <librd/rdfloat.h>

#include <setjmp.h> // Needs to be before of cmocka.h

#include <cmocka.h>

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

struct json_check {
	struct json_object *json;
	TAILQ_ENTRY(json_check) entry;
};

static void json_check(/*const*/ json_object *check,
		       /*const*/ json_object *other) {
	json_object_object_foreach(check, key, val) {
		/* All members checked must exist */
		struct json_object *o_val = NULL;
		const bool get_rc =
				json_object_object_get_ex(other, key, &o_val);
		assert_true(get_rc); // o_Val exists
		assert_int_equal(json_object_get_type(val),
				 json_object_get_type(o_val));

		/* And to be equal than other['key'] */
		// assert_true(json_object_equal(val, o_val));
		switch (json_object_get_type(val)) {
		case json_type_null:
			assert_null(o_val);
			break;
		case json_type_boolean:
			assert_int_equal(json_object_get_boolean(val),
					 json_object_get_boolean(o_val));
			break;
		case json_type_double:
			assert_true(rd_deq(json_object_get_double(val),
					   json_object_get_double(o_val)));
			break;
		case json_type_int:
			assert_true(json_object_get_int64(val) ==
				    json_object_get_int64(o_val));
			break;
		case json_type_object: {
			/*const*/ struct json_object *chld = NULL;
			const bool chld_get_rc = json_object_object_get_ex(
					other, key, &o_val);
			assert_true(chld_get_rc);
			json_check(val, chld);
		} break;
		case json_type_array:
			/// Still not needed
			assert_true(false);
			break;
		case json_type_string:
			assert_string_equal(json_object_get_string(val),
					    json_object_get_string(o_val));
			break;
		default:
			/// Unhandled case!
			assert_true(false);
			break;
		};
	}
}

void check_list_init(check_list_t *list) {
	TAILQ_INIT(list);
}

void check_list_push(check_list_t *list, struct json_check *object) {
	TAILQ_INSERT_TAIL(list, object, entry);
}

void check_list_push_checks(check_list_t *check_list,
			    json_key_test *checks,
			    size_t checks_list_size) {
	size_t i;

	for (i = 0; i < checks_list_size; ++i) {
		struct json_check *check =
				prepare_test_basic_sensor_check(&checks[i]);
		check_list_push(check_list, check);
	}
}

/** Checks and consume produced messages against check_list */
void json_list_check(check_list_t *check_list, rb_message_list *msgs) {

	while (!(rb_message_list_empty(msgs))) {
		rb_message_array_t *array = rb_message_list_first(msgs);
		rb_message_list_remove(msgs, array);
		for (size_t i = 0; i < array->count; ++i) {
			char *msg = array->msgs[i].payload;
			json_object *jmsg = json_tokener_parse(msg);
			assert_non_null(jmsg);

			struct json_check *check = TAILQ_FIRST(check_list);
			assert_non_null(check);
			TAILQ_REMOVE(check_list, check, entry);

			json_check(check->json, jmsg);

			json_object_put(jmsg);
			json_object_put(check->json);
			free(msg);
			free(check);
		}
		message_array_done(array);
	}

	assert_true(TAILQ_EMPTY(check_list));
}

struct json_check *prepare_test_basic_sensor_check(json_key_test *childs) {
	struct json_key_test_elm *i = NULL;
	struct json_check *ret = calloc(1, sizeof(ret[0]));
	assert_non_null(ret);
	ret->json = json_object_new_object();
	assert_non_null(ret->json);

	SLIST_FOREACH(i, childs, entry) {
		/* const int add_rc = */ json_object_object_add(
				ret->json, i->key, i->val);
		// assert_int_equal(add_rc, 0);
	}

	return ret;
}
