/*
 * ir_api.h
 * 
 * Copyright (C) 2021 Ligang Wang <ligangwangs@gmail.com>
 *
 * header file defining interface of IR interfaces
 */
#ifndef __MLANG_IR_API_H__
#define __MLANG_IR_API_H__

#include <llvm-c/Core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct address {
    LLVMValueRef pointer;
    unsigned alignment;
};

LLVMValueRef create_alloca(LLVMTypeRef type, unsigned align, LLVMValueRef fun, const char *var_name);
void create_coerced_store(LLVMBuilderRef builder, LLVMValueRef src, LLVMValueRef dst, unsigned align);

#ifdef __cplusplus
}
#endif

#endif