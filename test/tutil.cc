/*
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * Utility functions for testing
 */
#include <stdio.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>
#include "tutil.h"
#include "libfmemopen.h"
#include "sema/env.h"
#include "codegen/codegen.h"
#include "compiler/compiler.h"
#include "gtest/gtest.h"

const char *module_name = "test_ir";

FILE* open_file_from_string(const char* code)
{
    char* file_content = (char*)code;
    return fmemopen(file_content, strlen(file_content), "r");
}

file_tokenizer* create_tokenizer_for_string(const char* str)
{
    FILE* file = fmemopen((void*)str, strlen(str), "r");
    return create_tokenizer(file, "");
}

void make_module_ir(LLVMModuleRef module, const char *module_name, const char *ir_string, char *module_ir)
{
    sprintf(module_ir, R"(; ModuleID = '%s'
source_filename = "%s"
target datalayout = "%s"
target triple = "%s"
%s)", module_name, module_name, LLVMGetDataLayoutStr(module), LLVMGetDefaultTargetTriple(), ir_string);
}

void validate_m_code_with_ir_code(const char *m_code, const char *ir_code)
{
    char module_ir[1024];
    env* env = env_new(false);
    create_ir_module(env->cg, module_name);
    make_module_ir(env->cg->module, module_name, ir_code, module_ir);
    block_node* block = parse_string(env->parser, "test", m_code);
    char *ir_string = emit_ir_string(env, &block->base);
    ASSERT_STREQ(module_ir, ir_string);
    free_ir_string(ir_string);
    env_free(env);
}
