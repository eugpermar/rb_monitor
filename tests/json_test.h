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

#pragma once

#include "rb_message_list.h"

#include <json-c/json.h>
#include <sys/queue.h>

/// Convenience wrapper to create a json test object
struct json_key_test_elm {
	const char *key;
	struct json_object *val;
	SLIST_ENTRY(json_key_test_elm) entry;
};

typedef SLIST_HEAD(, json_key_test_elm) json_key_test;
#define JSON_KEY_TEST(check)                                                   \
	{ .slh_first = check }

/// Convenience macro for a string check
#define CHILD_X(mkey, new_f, mval, next)                                       \
	(struct json_key_test_elm[]) {                                         \
		{ .key = mkey, .val = new_f(mval), .entry.sle_next = next }    \
	}
#define CHILD_I(mkey, mval, next)                                              \
	(CHILD_X(mkey, json_object_new_int64, mval, next))
#define CHILD_S(mkey, mval, next)                                              \
	(CHILD_X(mkey, json_object_new_string, mval, next))

struct json_check;
/** Construct a check from childs
  @param childs_len length of childs
  @param childs Childs to test
  */
struct json_check *prepare_test_basic_sensor_check(json_key_test *childs);

typedef TAILQ_HEAD(, json_check) check_list_t;
void check_list_init(check_list_t *);
void check_list_push(check_list_t *list, struct json_check *check);
/** Add checks to the check list
	@param check_list list to add tests
	@param checks Checks to add to the list
	@param checks_list_size Size of checks list
	@param checks_size size of each individual check
	*/
void check_list_push_checks(check_list_t *check_list,
			    json_key_test *checks,
			    size_t checks_list_size);
void json_list_check(check_list_t *check_list, rb_message_list *msgs);
