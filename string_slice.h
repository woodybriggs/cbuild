#pragma once

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

String_Slice string_slice_from_cstr(const char * cstr);

boolx		 string_slice_has_prefix(String_Slice str, String_Slice prefix);
String_Slice string_slice_trim_prefix(String_Slice str, String_Slice prefix);


String_Slice string_slice_from_cstr(const char * cstr) {
	return (String_Slice){
		.str = cstr,
		.len = strlen(cstr)
	};
}

boolx string_slice_has_prefix(String_Slice str, String_Slice prefix) {

	if (str.len < prefix.len) return false;

	for (size_t i = 0; i < prefix.len; ++i) {
		if (str.str[i] != prefix.str[i]) return false;
	}

	return true;
}

String_Slice string_slice_trim_prefix(String_Slice str, String_Slice prefix) {	
	if (!string_slice_has_prefix(str, prefix)) return str;

	String_Slice res = str;
	res.str += prefix.len;
	res.len -= prefix.len;

	return res;
}

size_t string_slice_index_of(String_Slice str, String_Slice find) {

	for (size_t i = 0; i < str.len; i++) {

		boolx found = true;
		
		for (size_t j = 0; j < find.len; ++j) {
			if (str.str[i] != find.str[j]) {
				found = false;
				break;
			}	
		}

		if (found) {
			return i;
		}
	}

	return str.len;
} 

String_Slice string_slice_split_by_delim(String_Slice *str, String_Slice delim) {
	size_t next_delim_at = string_slice_index_of(*str, delim);

	String_Slice result = *str;
	result.len = next_delim_at;

	str->str += (next_delim_at+delim.len);
	str->len -= (next_delim_at+delim.len);

	return result;
}

boolx string_slice_eq(String_Slice a, String_Slice b) {
	if (a.len != b.len) return false;
	return memcmp(a.str, b.str, a.len) == 0;
}










