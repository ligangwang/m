#include "gtest/gtest.h"
#include "parser.h"
#include "analyzer.h"
#include "tutil.h"
#include <stdio.h>

TEST(testAnalyzer, testIntVariable){
  char test_code[] = "x = 11";
  auto parser = create_parser_for_string(test_code);
  block_node * block = parse_block(parser, nullptr);
  type_env* env = create_type_env();
  auto type = analyze(env, block)[0];
  auto node = (var_node*)block->nodes[0];
  ASSERT_EQ(1, block->nodes.size());
  ASSERT_STREQ("x", node->var_name.c_str());
  ASSERT_EQ(VAR_NODE, node->base.node_type);
  ASSERT_EQ(TYPE_INT, node->init_value->type);
  ASSERT_EQ(KIND_VAR, type->kind);
  auto var = (type_var*)type;
  ASSERT_STREQ("int", var->instance->name.c_str());
  destroy_type_env(env);
  destroy_parser(parser);
}

TEST(testAnalyzer, testDoubleVariable){
  char test_code[] = "x = 11.0";
  auto parser = create_parser_for_string(test_code);
  block_node * block = parse_block(parser, nullptr);
  type_env* env = create_type_env();
  auto type = analyze(env, block)[0];
  auto node = (var_node*)block->nodes[0];
  ASSERT_EQ(1, block->nodes.size());
  ASSERT_STREQ("x", node->var_name.c_str());
  ASSERT_EQ(VAR_NODE, node->base.node_type);
  ASSERT_EQ(TYPE_DOUBLE, node->init_value->type);
  ASSERT_EQ(KIND_VAR, type->kind);
  auto var = (type_var*)type;
  ASSERT_STREQ("double", var->instance->name.c_str());
  destroy_type_env(env);
  destroy_parser(parser);
}


// TEST(testAnalyzer, testBinExp){
//   char test_code[] = "x = 11. + 12";
//   auto parser = create_parser_for_string(test_code);
//   block_node * block = parse_block(parser, nullptr);
//   analyze((exp_node*)block);
//   auto node = (var_node*)block->nodes[0];
//   ASSERT_EQ(1, block->nodes.size());
//   ASSERT_STREQ("x", node->var_name.c_str());
//   ASSERT_EQ(VAR_NODE, node->base.node_type);
//   ASSERT_EQ(NUMBER_NODE, node->init_value->node_type);
//   ASSERT_EQ(TYPE_DOUBLE, node->init_value->type);
//   ASSERT_EQ(TYPE_DOUBLE, node->base.type);
//   destroy_parser(parser);
// }
