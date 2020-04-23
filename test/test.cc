#include "gtest/gtest.h"
#include "parser.h"
#include "repl.h"
#include "tutil.h"
#include "analyzer.h"
#include <stdio.h>
#include "astdump.h"

TEST(testGeneral, testBuildJit)
{
    menv* env = env_new();
    parser* parser = parser_new(NULL, true, NULL);
    JIT* jit = build_jit(env, parser);
    jit_free(jit);
    env_free(env);
}