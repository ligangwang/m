/*
 * analyzer.h
 * 
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * header file for type inference & semantic analsysis
 */
#ifndef __MLANG_ANALYZER_H__
#define __MLANG_ANALYZER_H__

#include "ast.h"
#include "clib/hashtable.h"

typedef struct {
    std::map<std::string, type_exp*> type_env;  //hashtable of <string, type_exp*>
    array nogens; //array of type_exp*
}type_env;

type_env* type_env_new();
void type_env_free(type_env* env);
type_exp* analyze(type_env* env, exp_node* node);

#endif