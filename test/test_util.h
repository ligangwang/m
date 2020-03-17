#pragma once
#include <stdio.h>
#include "parser.h"

parser* create_parser_for_string(char* str);
file_tokenizer* create_tokenizer_for_string(char* str);