/**
    @file arg_parser.h
    @brief Command-line argument parser with support for positional args, flags, and key-value pairs.

    Dependencies:
    - string_slice.h: Uses String_Slice for non-owning string operations

    Features:
    - Parses --long and -short style arguments
    - Supports key=value syntax (--flag=value)
    - Supports space-separated key-value pairs (--key value, -k value)
    - Handles positional arguments
    - Peek ahead without consuming arguments

    Documentation:
    1. Use arg_parser_has_remaining_args() to check if more arguments exist
    2. Call arg_parser_next() in a loop to consume arguments
    3. Check the returned Arg's .type field to determine argument type
    4. Use arg_parser_peek() to look ahead without advancing

    @nickname Woody
    @author Edward Briggs (woody@ebrb.ltd)
    @date 2025-02-23 T 16:43:00+08:00

    @copyright Copyright (c) 2025 Edward Briggs

    @license
    This header file is distributed under the MIT License.

    Copyright (c) 2025 Edward Briggs

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#if !defined(CBUILD)
#include "string_slice.h"
#endif

typedef enum Arg_Type_E {
	Arg_Type_None,
	Arg_Type_Positional,
	Arg_Type_Flag,
	Arg_Type_KeyVal,
} Arg_Type;

typedef struct Arg_S {
	Arg_Type 		type;
	String_Slice 	flag;
	String_Slice 	value;
} Arg;

boolx arg_parser_has_remaining_args(int argc, int current) {
	return (argc - current) > 0;
}

static const String_Slice _arg_parser_single_hyphen = {
	.str = "-",
	.len = 1,
};

static const String_Slice _arg_parser_double_hyphen = {
	.str = "--",
	.len = 2,
};

static const String_Slice _arg_parser_equals = {
	.str = "=",
	.len = 1,
};

static int _arg_parser_is_valid_flag_ident_char(char c) {
	if (c >= 'a' && c <= 'z') return true;
	else if (c >= 'A' && c <= 'Z') return true;
	else if (c == '-' || c == '_') return true;
	else if (c >= '0' && c <= '9') return true;
	return false;
}

Arg arg_parser_next(int argc, const char* argv[], int *current);
Arg arg_parser_peek(int argc, const char* argv[], int current);

static inline Arg _arg_parser_finalize(
	String_Slice flag_prefix,
	String_Slice current_str,
	Arg peeked,
	int *current
) {
	Arg result = {0};

	String_Slice flag_name = string_slice_trim_prefix(current_str, flag_prefix);
	String_Slice possible_value = string_slice_trim_start_while(flag_name, _arg_parser_is_valid_flag_ident_char);

	if (possible_value.len > 0) {
		if (possible_value.str[0] == '=') {
			String_Slice value = string_slice_trim_prefix(possible_value, _arg_parser_equals);
			String_Slice flag  = string_slice_trim_suffix(flag_name, value);
			flag.len-=1;
			result.type = Arg_Type_KeyVal;
			result.flag = flag;
			result.value = value;
			(*current)++;
			return result;
		}

		(*current)++;
		return result;
	}

	switch (peeked.type) {
		case Arg_Type_Positional:
		{
			(*current)+=2;
			result.type = Arg_Type_KeyVal;
			result.flag = current_str;
			result.value = peeked.value;
		} break;
		default:
		{
			(*current)++;
			result.type = Arg_Type_Flag;
			result.flag = current_str;
		}
	}

	return result;
}

Arg arg_parser_peek(int argc, const char* argv[], int current) {
	int to_peek = current+1;
	return arg_parser_next(argc, argv, &to_peek);
}

Arg arg_parser_next(int argc, const char* argv[], int *current) {
	Arg result = {0};
	if (*current >= argc) {
		return result;
	}

	String_Slice current_str = string_slice_from_cstr(argv[*current]);

	if (false) {}
	else if (string_slice_has_prefix(current_str, _arg_parser_double_hyphen)) {
		Arg peeked = arg_parser_peek(argc, argv, *current);
		result = _arg_parser_finalize(_arg_parser_double_hyphen, current_str, peeked, current);
	}
	else if (string_slice_has_prefix(current_str, _arg_parser_single_hyphen)) {
		Arg peeked = arg_parser_peek(argc, argv, *current);
		result = _arg_parser_finalize(_arg_parser_single_hyphen, current_str, peeked, current);
	}
	else {
		result.type  = Arg_Type_Positional;
		result.value = current_str;
		(*current)++;
	}

	return result;
}

#endif // ARG_PARSER_H
