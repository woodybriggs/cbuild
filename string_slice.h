#pragma once

#include <assert.h>
#include <stddef.h>
#include <string.h>

#ifndef false
#define false (0)
#define true (!false)
typedef int boolx;
#endif

typedef struct String_Slice_S {
	const char *str;
	size_t 		len;
} String_Slice;

typedef int (*test_char)(char c);

String_Slice string_slice_from_cstr(const char *cstr);
int string_slice_cmp(String_Slice a, String_Slice b);
boolx string_slice_has_prefix(String_Slice sub, String_Slice prefix);
boolx string_slice_has_suffix(String_Slice sub, String_Slice suffix);
String_Slice string_slice_trim_prefix(String_Slice sub, String_Slice prefix);
String_Slice string_slice_trim_prefix_all(String_Slice sub, String_Slice prefix);
String_Slice string_slice_trim_start_while(String_Slice sub, test_char predicate);
String_Slice string_slice_trim_suffix(String_Slice sub, String_Slice suffix);
String_Slice string_slice_trim_suffix_all(String_Slice sub, String_Slice suffix);
int string_slice_find(String_Slice str, String_Slice substr);
String_Slice string_slice_slice(String_Slice str, size_t start_inclusive, size_t end_inclusive);
String_Slice string_slice_split_by_delimiter(size_t *at, String_Slice sub, String_Slice delim);

static inline size_t _min_size_t(size_t a, size_t b) {
	return (a < b) ? a : b;
}

String_Slice string_slice_from_cstr(const char *cstr) {
	return (String_Slice){
		.str = cstr,
		.len = strlen(cstr)
	};
}

int string_slice_cmp(String_Slice a, String_Slice b) {
	if (a.len != b.len) return 0;
	for (size_t i = 0; i < a.len; i++) {
		if (a.str[i] != b.str[i]) return 0;
	}
	return 1;
}

boolx string_slice_has_prefix(String_Slice sub, String_Slice prefix) {
	if (sub.len < prefix.len) return false;

	for (size_t i = 0; i < prefix.len; ++i) {
		if (sub.str[i] != prefix.str[i]) return false;
	}

	return true;
}

boolx string_slice_has_suffix(String_Slice sub, String_Slice suffix) {
	if (sub.len < suffix.len) return false;

	size_t j = suffix.len;
	for (size_t i = sub.len; i > sub.len-suffix.len; --i) {
		if (sub.str[i-1] != suffix.str[j-1]) return false;
		--j;
	}

	return true;
}

String_Slice string_slice_trim_prefix(String_Slice sub, String_Slice prefix) {
	if (!string_slice_has_prefix(sub, prefix)) return sub;

	String_Slice result = sub;
	result.str += prefix.len;
	result.len -= prefix.len;

	return result;
}

String_Slice string_slice_trim_prefix_all(String_Slice sub, String_Slice prefix) {
	String_Slice result = sub;
	while (string_slice_has_prefix(result, prefix)) {
		result.str += prefix.len;
		result.len -= prefix.len;
	}

	return result;
}

String_Slice string_slice_trim_start_while(String_Slice sub, test_char predicate) {
	size_t at = 0;
	for (at = 0; predicate(sub.str[at]) && (at < sub.len); ++at);

	return (String_Slice){
		.str = sub.str + at,
		.len = sub.len - at,
	};
}

String_Slice string_slice_trim_suffix(String_Slice sub, String_Slice suffix) {
	if (!string_slice_has_suffix(sub, suffix)) return sub;

	String_Slice result = sub;
	result.len -= suffix.len;

	return result;
}

String_Slice string_slice_trim_suffix_all(String_Slice sub, String_Slice suffix) {
	String_Slice result = sub;
	while (string_slice_has_suffix(result, suffix)) {
		result.len -= suffix.len;
	}

	return result;
}

int string_slice_find(String_Slice str, String_Slice substr) {
	if (str.len < substr.len) return -1;

	for (size_t i = 0; i <= str.len - substr.len; ++i) {
		boolx match = true;
		for (size_t j = 0; j < substr.len; ++j) {
			if (str.str[i + j] != substr.str[j]) {
				match = false;
				break;
			}
		}

		if (match == true) {
			return (int)i;
		}
	}

	return -1;
}

String_Slice string_slice_slice(String_Slice str, size_t start, size_t end) {
	if (str.len == 0 || start > end) {
		return (String_Slice){
			.str = str.str + start,
			.len = 0,
		};
	}
	assert(end < str.len && "string_slice_slice: end must be within bounds of str.len");
	return (String_Slice){
		.str = str.str + start,
		.len = (end - start) + 1,
	};
}

String_Slice string_slice_split_by_delimiter(size_t *at, String_Slice sub, String_Slice delim) {
	if (*at == sub.len) {
		return (String_Slice){ .str = "", .len = 0 };
	}

	String_Slice remaining = string_slice_slice(sub, *at, sub.len-1);
	int next_delim = string_slice_find(remaining, delim);

	if (next_delim == -1) {
		(*at) += remaining.len;
		return remaining;
	}

	String_Slice result;
	if (next_delim == 0) {
		result = (String_Slice){ .str = remaining.str, .len = 0 };
	} else {
		result = string_slice_slice(remaining, 0, (size_t)next_delim-1);
	}

	(*at) += result.len + delim.len;

	return result;
}

boolx string_slice_eq(String_Slice a, String_Slice b) {
	if (a.len != b.len) return false;
	return memcmp(a.str, b.str, a.len) == 0;
}
