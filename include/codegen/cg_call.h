/*
 * cg_call.h
 * 
 * Copyright (C) 2021 Ligang Wang <ligangwangs@gmail.com>
 *
 * header file defining interface emitting call site
 */

#ifndef __MLANG_CG_CALL_H__
#define __MLANG_CG_CALL_H__

#include "clib/array.h"
#include "parser/ast.h"
#include <llvm-c/Core.h>

#ifdef __cplusplus
extern "C" {
#endif

LLVMValueRef emit_call_node(struct code_generator *cg, struct exp_node *node);

#ifdef __cplusplus
}
#endif

#endif
