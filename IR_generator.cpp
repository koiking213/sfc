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
    llvm::Function *func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "_simple_print", module);
    procedure_table["_simple_print"] = func;
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
#if DEBUG
    module->print(llvm::errs(), nullptr);
#endif
  }
}

namespace ast {
  llvm::Value *Int32_constant::codegen() const {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), this->value);
  }
  llvm::Value *Variable_reference::codegen() const {
    return builder.CreateLoad(variable_table[this->name], "var_tmp");
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
    llvm::Function *callee = module->getFunction("_simple_print");
    std::vector<llvm::Value*> args;
    for (auto &elm : this->elements) {
      args.push_back(elm->codegen());
      //args.push_back(builder.CreateLoad(variable_table["hoge"], "var_tmp"));
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
