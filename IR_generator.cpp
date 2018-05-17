#include "IR_generator.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

static LLVMContext context;
static IRBuilder<> builder(context);
static llvm::Module *module;
static std::map<std::string, llvm::Value *> variable_table;
static std::map<std::string, llvm::Value *> procedure_table;

namespace IR_generator {
  void add_library_prototype_to_module() {
    std::vector<llvm::Type*> int_types(1, llvm::Type::getInt32Ty(context));
    llvm::FunctionType *func_type =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(context), int_types, true);
    llvm::Function *func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "_simple_print", module);
    procedure_table["_simple_print"] = func;
    for (auto &arg : func->args()) {
      arg.setName("value");
    }
  }
  void generate_IR(const std::shared_ptr<ast::Program_unit> program) {
    module = new llvm::Module("top", context);
    add_library_prototype_to_module();
    program->codegen();
    module->print(llvm::errs(), nullptr);
  }
  // llvm::Value *generate_load(std::string name)
  // {
  //   return builder.CreateLoad(variable_table[name], "var_tmp");
  // }
}

namespace ast {
  llvm::Value *Int32_constant::codegen() const {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), this->value);
  }
  llvm::Value *Variable_reference::codegen() const {
    //return builder.CreateLoad(variable_table[this->name], "var_tmp");
    return variable_table[this->name];
  }
  
  llvm::Value *Expression::codegen() const {
    llvm::Value *lhs = this->lhs->codegen();
    llvm::Value *rhs = this->rhs->codegen();
    switch (this->exp_operator) {
    case operators::add: return builder.CreateAdd(lhs, rhs, "add_tmp");
    case operators::sub: return builder.CreateSub(lhs, rhs, "sub_tmp");
    case operators::mul: return builder.CreateMul(lhs, rhs, "mul_tmp");
    case operators::div: return builder.CreateSDiv(lhs, rhs, "div_tmp");
    default:return nullptr;
    }
  }

  void Assignment_statement::codegen() const
  {
    llvm::Value *lhs = this->lhs->codegen();
    llvm::Value *rhs = this->rhs->codegen();
    builder.CreateStore(rhs, lhs);
  }

  void Output_statement::codegen() const
  {
    llvm::Function *callee = module->getFunction("_simple_print");
    std::vector<llvm::Value*> args;
    for (auto &elm : this->elements) {
      //args.push_back(elm->codegen());
      
      args.push_back(builder.CreateLoad(variable_table["hoge"], "var_tmp"));
    }
    builder.CreateCall(callee, args, "call_tmp");
  }

  // only main program now
  void Program_unit::codegen() const
  {
    variable_table.clear();
    procedure_table.clear();
    
    llvm::FunctionType *func_type =
      llvm::FunctionType::get(builder.getInt32Ty(), false);
    llvm::Function *main_func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", module);
    
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", main_func);
    builder.SetInsertPoint(entry);

    // variable declarations
    for (auto &var_decl : this->variable_declarations) {
      var_decl->codegen();
    }
    
    // executable statements
    for (auto &stmt : this->statements) {
      stmt->codegen();
    }

    builder.CreateRet(builder.getInt32(0));
  }

  void Variable::codegen() const
  {
    // is just "context" ok for llvm::Type::getInt32Ty(context)?
    auto value = builder.CreateAlloca(llvm::Type::getInt32Ty(context), 0, this->name);
    variable_table[this->name] = value;
  }

}
