/*
 * Copyright (C) 2021 Ligang Wang <ligangwangs@gmail.com>
 *
 * LLVM IR Code Generation for functions
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "clib/array.h"
#include "clib/object.h"
#include "clib/util.h"
#include "codegen/abi_arg_info.h"
#include "codegen/cg_fun.h"
#include "codegen/codegen.h"
#include "codegen/fun_info.h"
#include "codegen/ir_api.h"
#include "codegen/type_size_info.h"
#include "sema/type.h"
#include <llvm-c/Support.h>

struct address emit_address_at_offset(struct code_generator *cg, struct address adr, struct abi_arg_info *info)
{
    unsigned offset = info->direct_offset;
    if (offset) {
    }
    return adr;
}

void _emit_argument_allocas(struct code_generator *cg, struct prototype_node *node,
    struct fun_info *fi, LLVMValueRef fun)
{
    struct type_oper *proto_type = (struct type_oper *)node->base.type;
    //assert (LLVMCountParams(fun) == array_size(&proto_type->args) - 1);
    unsigned param_count = array_size(&fi->args);
    struct array params;
    array_init(&params, sizeof(struct address));
    for (unsigned i = 0; i < param_count; i++) {
        struct var_node *param = (struct var_node *)array_get(&node->fun_params, i);
        struct type_exp *type_exp = *(struct type_exp **)array_get(&proto_type->args, i);
        struct ast_abi_arg *aaa = (struct ast_abi_arg *)array_get(&fi->args, i);
        struct ir_arg_range *iar = (struct ir_arg_range *)array_get(&fi->iai.args, i);
        unsigned first_ir_arg = iar->first_arg_index;
        unsigned ir_arg_num = iar->ir_arg_num;
        struct address param_value;
        LLVMValueRef arg_value = LLVMGetParam(fun, first_ir_arg);
        param_value.pointer = 0;
        param_value.alignment = 0;
        switch (aaa->info.kind) {
        case AK_INDIRECT:
        case AK_INDIRECT_ALIASED: {
            assert(ir_arg_num == 1);
            param_value.pointer = arg_value;
            param_value.alignment = aaa->info.indirect_align;
            if (proto_type->base.type < TYPE_EXT) { //aggregate
                //
                if (aaa->info.indirect_realign || aaa->info.kind == AK_INDIRECT_ALIASED) {
                    //realign the value, if the address is aliased, copy the param to ensure
                    //a unique address
                } else {
                }
            }
            array_push(&params, &param_value);
            hashtable_set_p(&cg->varname_2_irvalues, param->var_name, param_value.pointer);
            break;
        }
        case AK_DIRECT: {
            LLVMValueRef alloca = 0;
            struct type_size_info tsi = get_type_size_info(aaa->type);
            unsigned align = tsi.align_bits / 8;
            LLVMTypeRef sig_type = get_llvm_type(aaa->type);
            if (LLVMGetTypeKind(aaa->info.type) != LLVMStructTypeKind && aaa->info.direct_offset == 0
                && aaa->info.type == sig_type) {
                alloca = create_alloca(
                    aaa->info.type, align, fun, string_get(param->var_name));
                LLVMBuildStore(cg->builder, arg_value, alloca);
            } else {
                //TODO: if struct type
                //aaa->info.type
                size_t length;
                string arg_name;
                string_init_chars(&arg_name, LLVMGetValueName2(arg_value, &length));
                string_add_chars(&arg_name, ".coerce");
                LLVMSetValueName2(arg_value, string_get(&arg_name), string_size(&arg_name));
                alloca = create_alloca(
                    sig_type, align, fun, string_get(param->var_name));
                create_coerced_store(cg->builder, arg_value, alloca, align);
            }
            hashtable_set_p(&cg->varname_2_irvalues, param->var_name, alloca);
            break;
        }
        default: {
            assert(false);
        }
        }
    }
    array_deinit(&params);
}

LLVMValueRef emit_prototype_node(struct code_generator *cg, struct exp_node *node, struct fun_info **out_fi)
{
    struct prototype_node *proto = (struct prototype_node *)node;
    assert(proto->base.type);
    hashtable_set_p(&cg->protos, proto->name, proto);
    struct type_oper *proto_type = (struct type_oper *)proto->base.type;
    assert(proto_type->base.kind == KIND_OPER);
    struct fun_info *fi = get_fun_info(proto);
    if (out_fi)
        *out_fi = fi;
    assert(fi);
    LLVMTypeRef fun_type = get_fun_type(fi);
    LLVMValueRef fun = LLVMAddFunction(cg->module, string_get(proto->name), fun_type);
    if (fi->iai.sret_arg_no != InvalidIndex) {
        LLVMValueRef ai = LLVMGetParam(fun, fi->iai.sret_arg_no);
        const char *sret_var = "agg.result";
        LLVMSetValueName2(ai, sret_var, strlen(sret_var));
        add_fun_param_attribute(cg->context, fun, fi->iai.sret_arg_no, "noalias");
        add_fun_param_type_attribute(cg->context, fun, fi->iai.sret_arg_no, "sret", get_llvm_type(fi->ret.type));
    }
    unsigned param_count = array_size(&fi->args);
    for (unsigned i = 0; i < param_count; i++) {
        LLVMValueRef param = LLVMGetParam(fun, i);
        struct var_node *fun_param = (struct var_node *)array_get(&proto->fun_params, i);
        LLVMSetValueName2(param, string_get(fun_param->var_name), string_size(fun_param->var_name));
        struct ast_abi_arg *aa = (struct ast_abi_arg *)array_get(&fi->args, i);
        if (aa->type->type == TYPE_EXT)
            hashtable_set_p(&cg->varname_2_typename, fun_param->var_name, aa->type->name);
    }
    return fun;
}

LLVMValueRef emit_function_node(struct code_generator *cg, struct exp_node *node)
{
    struct function_node *fun_node = (struct function_node *)node;
    if (is_generic(node->type)) {
        return 0;
    }
    assert(fun_node->base.type->kind == KIND_OPER);
    hashtable_clear(&cg->varname_2_irvalues);
    struct fun_info *fi = 0;
    LLVMValueRef fun = emit_prototype_node(cg, (struct exp_node *)fun_node->prototype, &fi);
    assert(fun && fi);

    LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext(cg->context, fun, "entry");
    LLVMPositionBuilderAtEnd(cg->builder, bb);
    _emit_argument_allocas(cg, fun_node->prototype, fi, fun);
    LLVMValueRef ret_val = 0;
    //handle ret value
    // if (fi->ret.type->type == TYPE_UNIT) {

    // } else if (fi->ret.info.kind == AK_INDIRECT) {
    //     // sret struct type return type
    // } else {
    //     LLVMTypeRef ret_type = get_llvm_type(fi->ret.type);
    //     if (ret_type != fi->ret.info.type) {
    //         struct type_size_info tsi = get_type_size_info(fi->ret.type);
    //         unsigned align = tsi.align_bits / 8;
    //         LLVMValueRef ret_alloca = create_alloca(ret_type, align, fun, "retval");
    //     }
    // }
    for (size_t i = 0; i < array_size(&fun_node->body->nodes); i++) {
        struct exp_node *stmt = *(struct exp_node **)array_get(&fun_node->body->nodes, i);
        ret_val = emit_ir_code(cg, stmt);
    }
    if (!ret_val || !fi->ret.info.type) {
        //struct type_exp *ret_type = get_ret_type(fun_node);
        //enum type type = get_type(ret_type);
        //ret_val = cg->ops[type].get_zero(cg->context, cg->builder);
        LLVMBuildRetVoid(cg->builder);
    } else {

        LLVMTypeRef ret_type = LLVMTypeOf(ret_val);
        if (LLVMGetTypeKind(ret_type) != LLVMGetTypeKind(fi->ret.info.type)) {
            // assuming cast struct to
            struct type_size_info tsi = get_type_size_info(fi->ret.type);
            LLVMTypeRef ret_ptr = LLVMPointerType(fi->ret.info.type, 0);
            assert(LLVMGetTypeKind(ret_type) == LLVMGetTypeKind(ret_ptr));
            //cast struct pointer to int pointer
            ret_val = LLVMBuildBitCast(cg->builder, ret_val, ret_ptr, "");
            //load int from int pointer
            ret_val = LLVMBuildLoad2(cg->builder, fi->ret.info.type, ret_val, "");
            LLVMSetAlignment(ret_val, tsi.align_bits / 8);
        }
        LLVMBuildRet(cg->builder, ret_val);
    }
    return fun;
}

LLVMValueRef get_llvm_function(struct code_generator *cg, symbol fun_name)
{
    const char *name = string_get(fun_name);
    LLVMValueRef f = LLVMGetNamedFunction(cg->module, name);
    if (f)
        return f;
    struct exp_node *fp = (struct exp_node *)hashtable_get_p(&cg->protos, fun_name);
    if (fp)
        return emit_prototype_node(cg, fp, 0);
    return 0;
}
