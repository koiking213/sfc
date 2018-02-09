#include "IR_generator.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

static LLVMContext context;
static IRBuilder<> builder(context);
static llvm::Module *module;
static std::map<std::string, Value *> variable_table;

void generate_IR(const ast::ProgramUnit &program) {
  module = new llvm::Module("top", context);
  program.codegen();
  module->dump( );
}

namespace ast {
  // only main program now
  void ProgramUnit::codegen() const
  {
    variable_table.clear();
    llvm::FunctionType *funcType = 
      llvm::FunctionType::get(builder.getInt32Ty(), false);
    llvm::Function *mainFunc = 
      llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
    
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
    builder.SetInsertPoint(entry);

    // variable declarations
    for (auto dec : this->variable_declarations) {
      dec->codegen();
    }

    // executable statements
    for (auto stmt : this->statements) {
      stmt->codegen();
    }
  }

  void Variable::codegen() const
  {
    // is just "context" ok for llvm::Type::getInt32Ty(context)?
    auto value = builder.CreateAlloca(llvm::Type::getInt32Ty(context), 0, this->name);
    variable_table[this->name] = value;
  }

  struct codegenerator : public boost::static_visitor<llvm::Value *> {
    llvm::Value *operator()(int const value) const
    {
      return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
    }
    llvm::Value *operator()(std::string const var) const
    {
      return builder.CreateLoad(variable_table[var], "var_tmp");
    }
    template<operators Binary_op>
    llvm::Value *operator()(binary_op<Binary_op> const& op) const
    {
      llvm::Value *lhs = boost::apply_visitor(codegenerator(), op.lhs);
      llvm::Value *rhs = boost::apply_visitor(codegenerator(), op.rhs);
      switch (Binary_op) {
      case operators::add: return builder.CreateAdd(lhs, rhs, "add_tmp");
      case operators::sub: return builder.CreateSub(lhs, rhs, "sub_tmp");
      case operators::mul: return builder.CreateMul(lhs, rhs, "mul_tmp");
      case operators::div: return builder.CreateSDiv(lhs, rhs, "div_tmp");
      }
    }
  };

  void Assignment_statement::codegen() const
  {
    llvm::Value *lhs = boost::apply_visitor(codegenerator(), this->lhs);
    llvm::Value *rhs = boost::apply_visitor(codegenerator(), this->rhs);
    builder.CreateStore(rhs, lhs);
  }


}
