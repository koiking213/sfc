#include "IR_generator.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

static LLVMContext context;
static IRBuilder<> builder(context);
static llvm::Module *module;

void create_IR(const parser::Program &program) {
  module = new llvm::Module("top", context);

  llvm::FunctionType *funcType = 
    llvm::FunctionType::get(builder.getInt32Ty(), false);
  llvm::Function *mainFunc = 
    llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
 
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
  builder.SetInsertPoint(entry);
 
  module->dump( );
}

//struct codegen : public boost:: static_visitor
