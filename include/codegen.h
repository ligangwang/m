#include "parser.h"

using namespace std;

typedef struct code_generator{
    void* context;
    void* builder;
    parser* parser;
    map<std::string, void* > named_values; //llvm::AllocaInst*
    vector<void *> modules;
    void* module;
} code_generator;

code_generator* create_code_generator(parser* parser);
void destroy_code_generator(code_generator* cg);

void* generate(code_generator* cg, exp_node* node);

void* generate_prototype_node(code_generator* cg, prototype_node* node);
void* generate_function_node(code_generator* cg, function_node* node);

