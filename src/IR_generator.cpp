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

// for codeout
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

static llvm::Module *module;
static LLVMContext context;
static IRBuilder<> builder(context);
static std::map<std::string, llvm::Value *> variable_table;
static std::map<std::string, llvm::Value *> procedure_table;

namespace IR_generator {
  void add_library_prototype_to_module() {
    std::vector<llvm::Type*> int_types(1, llvm::Type::getInt32Ty(context));
    llvm::FunctionType *func_type =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(context), int_types, true);

    // TODO: write -> write
    llvm::Function *func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "_write_int", module);
    procedure_table["_write_int"] = func;
    for (auto &arg : func->args()) {
      arg.setName("value");
    }

    func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "_write_float", module);
    procedure_table["_write_float"] = func;
    for (auto &arg : func->args()) {
      arg.setName("value");
    }
  }
  void codeout(std::string outfile_name) {
    // Initialize the target registry etc.
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    auto TargetTriple = sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
      errs() << Error;
      return;
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TheTargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    module->setDataLayout(TheTargetMachine->createDataLayout());

    std::error_code EC;
    raw_fd_ostream dest(outfile_name, EC, sys::fs::F_None);

    if (EC) {
      errs() << "Could not open file: " << EC.message();
      return;
    }

    legacy::PassManager pass;
    auto FileType = TargetMachine::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
      errs() << "TheTargetMachine can't emit a file of this type";
      return;
    }

    pass.run(*module);
    dest.flush();
  }
  
  void generate_IR(const std::shared_ptr<ast::Program_unit> program) {
    module = new llvm::Module("top", context);
    add_library_prototype_to_module();
    program->codegen();
#if DEBUG_MODE
    module->print(llvm::errs(), nullptr);
#endif
  }
}

namespace ast {
  llvm::Value *Int32_constant::codegen() const {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), this->value);
  }
  llvm::Value *FP32_constant::codegen() const {
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(context), this->value);
  }
  llvm::Value *Variable_reference::codegen() const {
    return builder.CreateLoad(variable_table[this->var->get_name()], "var_tmp");
  }
  
  llvm::Value *Expression::codegen() const {
    llvm::Value *lhs = this->lhs->codegen();
    llvm::Value *rhs = this->rhs->codegen();
    switch (this->exp_operator) {
    case operators::add:
      if (this->get_type() == Type_kind::i32) {
        return builder.CreateAdd(lhs, rhs, "add_tmp");
      } else if (this->get_type() == Type_kind::fp32) {
        return builder.CreateFAdd(lhs, rhs, "fadd_tmp");
      } else {
        assert(0);
      }
    case operators::sub:
      if (this->get_type() == Type_kind::i32) {
        return builder.CreateSub(lhs, rhs, "sub_tmp");
      } else if (this->get_type() == Type_kind::fp32) {
        return builder.CreateFSub(lhs, rhs, "fsub_tmp");
      } else {
        assert(0);
      }
    case operators::mul:
      if (this->get_type() == Type_kind::i32) {
        return builder.CreateMul(lhs, rhs, "mul_tmp");
      } else if (this->get_type() == Type_kind::fp32) {
        return builder.CreateFMul(lhs, rhs, "fmul_tmp");
      } else {
        assert(0);
      }
    case operators::div:
      if (this->get_type() == Type_kind::i32) {
        return builder.CreateSDiv(lhs, rhs, "div_tmp");
      } else if (this->get_type() == Type_kind::fp32) {
        return builder.CreateFDiv(lhs, rhs, "fdiv_tmp");
      } else {
        assert(0);
      }
    default:return nullptr;
    }
  }

  llvm::Value *Variable_definition::codegen() const {
    return variable_table[this->name];
  }

  void Assignment_statement::codegen() const
  {
    llvm::Value *lhs = this->lhs->codegen();
    llvm::Value *rhs = this->rhs->codegen();
    builder.CreateStore(rhs, lhs);
  }

  void Output_statement::codegen() const
  {
    for (auto &elm : this->elements) {
      std::vector<llvm::Value*> args;
      args.push_back(elm->codegen());
      llvm::Function *callee;
      if (elm->get_type() == Type_kind::i32) {
	callee = module->getFunction("_write_int");
      } else if (elm->get_type() == Type_kind::fp32) {
	callee = module->getFunction("_write_float");
      } else {
	assert(0);
      }
      builder.CreateCall(callee, args, "call_tmp");
    }
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
    for (auto var_decl : *this->variables) {
      var_decl.second->codegen();
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
    llvm::Value *value;
    if (this->get_type_kind() == Type_kind::i32) {
      value = builder.CreateAlloca(llvm::Type::getInt32Ty(context), 0, this->name);
    } else if (this->get_type_kind() == Type_kind::fp32) {
      value = builder.CreateAlloca(llvm::Type::getFloatTy(context), 0, this->name);
    }
    variable_table[this->name] = value;
  }

}
