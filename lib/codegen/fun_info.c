#include "codegen/fun_info.h"
#include "clib/util.h"
#include "codegen/codegen.h"
#include "codegen/compute_fun_info.h"
#include "codegen/ir_arg_info.h"
#include <assert.h>

const unsigned ALL_REQUIRED = ~0U;

void fun_info_init(struct fun_info *fi, unsigned required_args)
{
    fi->is_chain_call = false;
    fi->required_args = required_args;
    array_init(&fi->args, sizeof(struct ast_abi_arg));
    ir_arg_info_init(&fi->iai);
}

void fun_info_deinit(struct fun_info *fi)
{
    array_deinit(&fi->args);
}

bool is_variadic(struct fun_info *fi)
{
    return fi->required_args != ALL_REQUIRED;
}

void _map_to_ir_arg_info(struct fun_info *fi)
{
    unsigned ir_arg_no = 0;
    if (fi->ret.info.kind == AK_INDIRECT)
        fi->iai.sret_arg_no = ir_arg_no++;

    unsigned arg_no = 0;
    unsigned arg_num = array_size(&fi->args);
    for (unsigned i = 0; i < arg_num; i++) {
        struct ast_abi_arg *aa = (struct ast_abi_arg *)array_get(&fi->args, i);
        struct ir_arg_range iar;
        ir_arg_range_init(&iar);
        if (get_padding_type(&aa->info))
            iar.padding_arg_index = ir_arg_no++;
        switch (aa->info.kind) {
        case AK_EXTEND:
        case AK_DIRECT: {
            if (aa->info.kind == AK_DIRECT && aa->info.can_be_flattened && LLVMGetTypeKind(aa->info.type) == LLVMStructTypeKind) {
                iar.ir_arg_num = LLVMCountStructElementTypes(aa->info.type);
            } else {
                iar.ir_arg_num = 1;
            }
            break;
        }
        case AK_INDIRECT:
        case AK_INDIRECT_ALIASED:
            iar.ir_arg_num = 1;
            break;
        case AK_IGNORE:
        case AK_INALLOCA:
            iar.ir_arg_num = 0;
            break;
        case AK_COERCE_AND_EXPAND:
            //TODO: different than LLVMGetStructElementTypes returned number of types ?
            iar.ir_arg_num = LLVMCountStructElementTypes(aa->info.type);
            break;
        case AK_EXPAND:
            iar.ir_arg_num = get_expansion_size(aa->type);
            break;
        }
        if (iar.ir_arg_num > 0) {
            iar.first_arg_index = ir_arg_no;
            ir_arg_no += iar.ir_arg_num;
        }
        array_push(&fi->iai.args, &iar);
    }
    fi->iai.total_ir_args = ir_arg_no;
}

struct fun_info *get_fun_info(struct prototype_node *proto)
{
    struct type_oper *fun_type = (struct type_oper *)proto->base.type;
    struct hashtable *fun_infos = get_fun_infos();
    struct fun_info *result = hashtable_get_p(fun_infos, proto->name);
    if (result)
        return result;
    struct fun_info fi;
    unsigned param_num = array_size(&fun_type->args) - 1;
    if (proto->is_variadic)
        param_num -= 1;
    fun_info_init(&fi, proto->is_variadic ? param_num : ALL_REQUIRED);
    fi.ret.type = *(struct type_exp **)array_back(&fun_type->args);
    struct ast_abi_arg aa;
    for (unsigned i = 0; i < param_num; i++) {
        aa.type = *(struct type_exp **)array_get(&fun_type->args, i);
        array_push(&fi.args, &aa);
    }
    compute_fun_info(&fi);
    // direct or extend without a specified coerce type, specify the
    // default now.
    if (can_have_coerce_to_type(&fi.ret.info) && !fi.ret.info.type)
        fi.ret.info.type = get_llvm_type(fi.ret.type);
    unsigned arg_num = array_size(&fi.args);
    for (unsigned i = 0; i < arg_num; i++) {
        struct ast_abi_arg *aa = (struct ast_abi_arg *)array_get(&fi.args, i);
        if (can_have_coerce_to_type(&aa->info) && !aa->info.type)
            aa->info.type = get_llvm_type(aa->type);
    }
    _map_to_ir_arg_info(&fi);
    hashtable_set_p(fun_infos, proto->name, &fi);
    return (struct fun_info *)hashtable_get_p(fun_infos, proto->name);
}

LLVMTypeRef get_fun_type(struct fun_info *fi)
{
    LLVMTypeRef ret_type = 0;
    switch (fi->ret.info.kind) {
    case AK_EXPAND:
    case AK_INDIRECT_ALIASED:
        assert(false);
    case AK_EXTEND:
    case AK_DIRECT:
        ret_type = fi->ret.info.type;
        break;
    case AK_INALLOCA:
        //TODO: only for MSVC
        break;
    case AK_INDIRECT:
    case AK_IGNORE:
        ret_type = LLVMVoidTypeInContext(get_llvm_context());
        break;
    case AK_COERCE_AND_EXPAND:
        ret_type = fi->ret.info.coerce_and_expand_type;
        break;
    }

    struct array arg_types;
    array_init(&arg_types, sizeof(LLVMTypeRef *));
    if (fi->iai.sret_arg_no != InvalidIndex) {
        assert(fi->iai.sret_arg_no == 0);
        //TODO: fixme address space
        LLVMTypeRef ret_type = LLVMPointerType(get_llvm_type(fi->ret.type), 0);
        array_push(&arg_types, &ret_type);
    }
    //TODO: inalloca
    unsigned arg_num = array_size(&fi->args);
    for (unsigned i = 0; i < arg_num; i++) {
        struct ast_abi_arg *aa = (struct ast_abi_arg *)array_get(&fi->args, i);
        struct ir_arg_range *iar = get_ir_arg_range(&fi->iai, i);
        if (iar->padding_arg_index != InvalidIndex) {
            assert(iar->padding_arg_index == array_size(&arg_types));
            array_push(&arg_types, &aa->info.padding_type);
        }
        switch (aa->info.kind) {
        case AK_IGNORE:
        case AK_INALLOCA:
            assert(iar->ir_arg_num == 0);
            break;
        case AK_INDIRECT: {
            assert(iar->ir_arg_num == 1);
            assert(iar->first_arg_index == array_size(&arg_types));
            LLVMTypeRef lty = get_llvm_type_for_abi(aa->type);
            LLVMTypeRef pointer_type = LLVMPointerType(lty, 0); //TODO: should use AllocaAddressSpace in Layout
            array_push(&arg_types, &pointer_type);
            break;
        }
        case AK_INDIRECT_ALIASED: {
            assert(iar->ir_arg_num == 1);
            assert(iar->first_arg_index == array_size(&arg_types));
            LLVMTypeRef lty = get_llvm_type_for_abi(aa->type);
            LLVMTypeRef pointer_type = LLVMPointerType(lty, 0); //TODO: should use AllocaAddressSpace in Layout
            array_push(&arg_types, &pointer_type);
            break;
        }
        case AK_EXTEND:
        case AK_DIRECT: {
            assert(iar->first_arg_index == array_size(&arg_types));
            LLVMTypeRef arg_type = aa->info.type;
            if (LLVMGetTypeKind(arg_type) == LLVMStructTypeKind) {
                for (unsigned i = 0; i < LLVMCountStructElementTypes(arg_type); ++i) {
                    LLVMTypeRef field_type = LLVMStructGetTypeAtIndex(arg_type, i);
                    array_push(&arg_types, &field_type);
                }
            } else {
                assert(iar->ir_arg_num == 1);
                array_push(&arg_types, &arg_type);
            }
            break;
        }
        case AK_COERCE_AND_EXPAND: {
            assert(iar->first_arg_index == array_size(&arg_types));
            assert(iar->ir_arg_num);
            LLVMTypeRef *types;
            MALLOC(types, sizeof(*types) * iar->ir_arg_num);
            get_coerce_and_expand_types(&aa->info, types);
            for (unsigned i = 0; i < iar->ir_arg_num; ++i) {
                array_push(&arg_types, &types[i]);
            }
            free(types);
            break;
        }
        case AK_EXPAND:
            assert(iar->first_arg_index == array_size(&arg_types));
            get_expanded_types(aa->type, &arg_types);
            break;
        }
    }
    assert(fi->iai.total_ir_args == array_size(&arg_types));
    assert(ret_type);
    LLVMTypeRef fun_type = LLVMFunctionType(ret_type, fi->iai.total_ir_args ? array_get(&arg_types, 0) : 0, fi->iai.total_ir_args, is_variadic(fi));
    array_deinit(&arg_types);
    return fun_type;
}

bool check_rvo(struct fun_info *fi)
{
    return fi->ret.type->type == TYPE_EXT && (fi->ret.info.kind == AK_INDIRECT || fi->ret.info.kind == AK_INDIRECT_ALIASED);
}