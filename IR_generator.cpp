#include "IR_generator.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

static LLVMContext context;
static IRBuilder<> builder(context);
static llvm::Module *module;
static std::map<std::string, Value *> variable_table;
static std::map<std::string, Value *> procedure_table;

namespace IR_generator {
  void add_library_prototype_to_module() {
    std::vector<Type*> int_types(1, llvm::Type::getInt32Ty(context));
    llvm::FunctionType *func_type =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(context), int_types, false);
    llvm::Function *func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "_simple_print", module);
    procedure_table["_simple_print"] = func;
    for (auto &arg : func->args()) {
      arg.setName("value");
    }
  }
  void generate_IR(const ast::ProgramUnit &program) {
    module = new llvm::Module("top", context);
    add_library_prototype_to_module();
    program.codegen();
    module->dump( );
  }
  llvm::Value *generate_load(std::string name)
  {
    return builder.CreateLoad(variable_table[name], "var_tmp");
  }
}

namespace ast {
  struct codegenerator : public boost::static_visitor<llvm::Value *> {
    llvm::Value *operator()(Constant const constant) const
    {
      if (constant.type() == typeid(Integer_constant)) {
	int value = boost::get<Integer_constant>(constant).value;
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
      } else {
	return nullptr;
      }
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
    assert(this->lhs.type() == typeid(std::string));
    llvm::Value *lhs = variable_table[boost::get<std::string>(this->lhs)];
    llvm::Value *rhs = boost::apply_visitor(codegenerator(), this->rhs);
    builder.CreateStore(rhs, lhs);
  }

  void Output_statement::codegen() const
  {
    llvm::Function *callee = module->getFunction("_simple_print");
    std::string var_name = boost::get<std::string>(this->elements[0]);
    std::vector<llvm::Value*> args;
    args.push_back(IR_generator::generate_load(var_name));
    builder.CreateCall(callee, args, "call_tmp");
  }

  // todo: visitor pattern
  void stmt_codegen(Statement const &stmt)
  {
    if (stmt.type() == typeid(Assignment_statement)) {
      boost::get<Assignment_statement>(stmt).codegen();
    } else if (stmt.type() == typeid(Output_statement)) {
      boost::get<Output_statement>(stmt).codegen();
    }
  }

  // only main program now
  void ProgramUnit::codegen() const
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
    for (auto dec : this->variable_declarations) {
      dec->codegen();
    }

    // executable statements
    for (auto stmt : this->statements) {
      stmt_codegen(stmt);
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
