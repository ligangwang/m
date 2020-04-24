/*
 * builtins.h
 * 
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * header file for builtin functions
 */
#ifndef __MLANG_BUILTIN_H__
#define __MLANG_BUILTIN_H__

#include "ast.h"

#ifdef __cplusplus
extern "C"{
#endif

array get_builtins(void* context);

#ifdef __cplusplus
}
#endif

#endif