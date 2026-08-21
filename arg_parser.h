#pragma once

#ifndef false
#define false (0)
#define true (!false)
typedef int boolx;
#endif

typedef enum Arg_Type_E {
	Arg_Type_None,
	Arg_Type_Positional,
	Arg_Type_KeyVal,
	Arg_Type_Flag,
} Arg_Type;

typedef struct Arg_S {
	Arg_Type 	type;
	const char *flag;
	const char *value;
} Arg; 

boolx arg_parser_has_next(int argc, int*at);

Arg arg_parser_next(int argc, const char **argv, int *at);


static boolx _cstr_has_prefix(const char *str, const char *prefix) {
	int str_len 	= strlen(str);
	int prefix_len 	= strlen(prefix);
	
	if (str_len < prefix_len) return false;

	for (int i = 0; i < prefix_len; ++i) {
		if (str[i] != prefix[i]) return false;	
	}

	return true;
}


boolx arg_parser_has_next(int argc, int *at) {
	return (*at < argc);
}


Arg arg_parser_next(int argc, const char *argv[], int *at) {
	if (*at >= argc) return (Arg){0};

	Arg result = {0};

	const char *current = argv[*at];
	if (_cstr_has_prefix(current, "--")) {
				
	}
	else if (_cstr_has_prefix(current, "-")) {

	}
	else {
			
	}
}

