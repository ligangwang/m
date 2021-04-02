/*
 * compiler.h
 * 
 * Copyright (C) 2020 Ligang Wang <ligangwangs@gmail.com>
 *
 * header file for m compiler
 */
#ifndef __MLANG_COMPILER_H__
#define __MLANG_COMPILER_H__

#ifdef __cplusplus
extern "C" {
#endif

enum object_file_type {
    FT_UNK = 0,
    FT_BITCODE = 1,
    FT_IR = 2,
    FT_OBJECT = 3
};

int compile(const char* fn, enum object_file_type file_type);

#ifdef __cplusplus
}
#endif

#endif