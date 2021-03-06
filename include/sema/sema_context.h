/*
 * sema_context.h
 * 
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * header file of m environement
 */

#ifndef __MLANG_SEMA_CONTEXT_H__
#define __MLANG_SEMA_CONTEXT_H__

#include "clib/array.h"
#include "clib/symboltable.h"
#include "codegen/codegen.h"
#include "parser/parser.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sema_context {
    /* 
     *  value type env: symboltable of <symbol, struct type_exp>*>
     *  binding variable name to type expression
     */
    struct symboltable decl_2_typexps;

    /* 
     *  value ast env: symboltable of <symbol, struct exp_node>*>
     *  binding variable name to ast node expression
     */
    struct symboltable varname_2_asts;

    /* 
     *  type env: symboltable of <symbol, struct type_exp>*>
     *  binding type name to type expression
     */
    struct symboltable typename_2_typexps;

    /* 
     *  new type AST def: hashtable of <symbol, struct exp_node （type_node)*> 
     *      example: type Point2D = x:int y:int
     */
    struct hashtable ext_typename_2_asts;

    /* 
     *  builtin ASTs: symbol to exp_node*, only used for codegen
     */
    struct hashtable builtin_ast;

    /* 
     *  generic value ASTs: hashtable of <symbol, struct exp_node*>
     */
    struct hashtable generic_ast;

    /* 
     *  specialized value ASTs: hashtable of <symbol, struct exp_node*>
     */
    struct hashtable specialized_ast;

    /* 
     *  prototype declaration ASTs: hashtable of <symbol, struct exp_node*>
     */
    struct hashtable protos;

    /* 
     *  call ASTs: hashtable of <symbol, struct exp_node*>
     */
    struct hashtable calls;
    /* 
     *  struct array of struct type_exp*, specialized (non generic) 
     */
    struct array nongens;

    /*
     *  used builtins of symbol, needs to be codegened by adding to the module
     */
    struct array used_builtin_names;

    symbol scope_marker;
    //reference the parser
    struct parser *parser;
};

struct sema_context *sema_context_new(struct parser *parser);
void sema_context_free(struct sema_context *env);
void enter_scope(struct sema_context *env);
void leave_scope(struct sema_context *env);

#ifdef __cplusplus
}
#endif

#endif
