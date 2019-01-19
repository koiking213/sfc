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
static std::map<std::string, llvm::Value *> global_string_table;

namespace IR_generator {
  void add_library_prototype_to_module() {
    std::vector<llvm::Type*> int_types(1, llvm::Type::getInt32Ty(context));
    llvm::FunctionType *func_type =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(context), int_types, true);

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

    func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "_write_logical", module);
    procedure_table["_write_logical"] = func;
    for (auto &arg : func->args()) {
      arg.setName("value");
    }

    func =
      llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "_write_string", module);
    procedure_table["_write_string"] = func;
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
  
  void generate_IR(const std::shared_ptr<ast::Program_unit> program, bool debug_mode) {
    module = new llvm::Module("top", context);
    add_library_prototype_to_module();
    program->codegen();
    if (debug_mode) {
      std::cout << "=== LLVM IR ===" << std::endl;
      module->print(llvm::errs(), nullptr);
    }
  }
}

namespace ast {
  llvm::Value *Int32_constant::codegen() const {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), this->value);
  }
  llvm::Value *FP32_constant::codegen() const {
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(context), this->value);
  }
  llvm::Value *Logical_constant::codegen() const {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), this->get_int_value());
  }
  llvm::Value *Character_constant::codegen() const {
    return global_string_table[this->value];
  }
  llvm::Value *Variable_reference::codegen() const {
    if (this->get_type_kind() == Type_kind::character) {
      llvm::Value* zero = builder.getInt32(0);
      llvm::Value* args[] = {zero, zero};
      return  builder.CreateInBoundsGEP(builder.getInt8Ty(),
                                        variable_table[this->get_var_name()],
                                        zero,
                                        "character_ref");
    } else {
      return builder.CreateLoad(variable_table[this->var->get_name()], "var_tmp");
    }
  }
  llvm::Value *Array_element_reference::codegen() const {
    llvm::Value *val = builder.CreateGEP(variable_table[this->get_var_name()],
                                         this->offset_expr->codegen(),
                                         "array_element_ref");
    return builder.CreateLoad(val, "elm_load_tmp");
  }
  llvm::Value *Unary_op::codegen() const {
    switch (this->exp_operator) {
    case unary_op_kind::i32tofp32:
      return builder.CreateSIToFP(this->operand->codegen(), llvm::Type::getFloatTy(context), "i32tofp32cast");
    }
    return nullptr;
  }
  llvm::Value *Binary_op::codegen() const {
    if (this->is_constant_int()) {
      return builder.getInt32(this->eval_constant_value());
    }
    llvm::Value *lhs = this->lhs->codegen();
    llvm::Value *rhs = this->rhs->codegen();
    switch (this->exp_operator) {
    case binary_op_kind::add:
      if (this->get_type_kind() == Type_kind::i32) {
        return builder.CreateAdd(lhs, rhs, "add_tmp");
      } else if (this->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFAdd(lhs, rhs, "fadd_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::sub:
      if (this->get_type_kind() == Type_kind::i32) {
        return builder.CreateSub(lhs, rhs, "sub_tmp");
      } else if (this->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFSub(lhs, rhs, "fsub_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::mul:
      if (this->get_type_kind() == Type_kind::i32) {
        return builder.CreateMul(lhs, rhs, "mul_tmp");
      } else if (this->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFMul(lhs, rhs, "fmul_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::div:
      if (this->get_type_kind() == Type_kind::i32) {
        return builder.CreateSDiv(lhs, rhs, "div_tmp");
      } else if (this->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFDiv(lhs, rhs, "fdiv_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::eq:
      if (this->lhs->get_type_kind() == Type_kind::i32) {
        return builder.CreateICmpEQ(lhs, rhs, "ieq_tmp");
      } else if (this->lhs->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFCmpUEQ(lhs, rhs, "feq_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::ne:
      if (this->lhs->get_type_kind() == Type_kind::i32) {
        return builder.CreateICmpNE(lhs, rhs, "ine_tmp");
      } else if (this->lhs->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFCmpUNE(lhs, rhs, "fne_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::lt:
      if (this->lhs->get_type_kind() == Type_kind::i32) {
        return builder.CreateICmpSLT(lhs, rhs, "ilt_tmp");
      } else if (this->lhs->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFCmpULT(lhs, rhs, "flt_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::le:
      if (this->lhs->get_type_kind() == Type_kind::i32) {
        return builder.CreateICmpSLE(lhs, rhs, "ile_tmp");
      } else if (this->lhs->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFCmpULE(lhs, rhs, "fle_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::gt:
      if (this->lhs->get_type_kind() == Type_kind::i32) {
        return builder.CreateICmpSGT(lhs, rhs, "igt_tmp");
      } else if (this->lhs->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFCmpUGT(lhs, rhs, "fgt_tmp");
      } else {
        assert(0);
      }
    case binary_op_kind::ge:
      if (this->lhs->get_type_kind() == Type_kind::i32) {
        return builder.CreateICmpSGE(lhs, rhs, "ige_tmp");
      } else if (this->lhs->get_type_kind() == Type_kind::fp32) {
        return builder.CreateFCmpUGE(lhs, rhs, "fge_tmp");
      } else {
        assert(0);
      }
    }
  }

  llvm::Value *Variable_definition::codegen() const {
    return variable_table[this->get_var_name()];
  }

  llvm::Value *Array_element_definition::codegen() const {
    return builder.CreateGEP(variable_table[this->get_var_name()],
                             this->offset_expr->codegen(),
                             "array_element_def");
  }

  void Assignment_statement::codegen() const
  {
    llvm::Value *lhs = this->lhs->codegen();
    llvm::Value *rhs = this->rhs->codegen();

    if (this->lhs->get_type_kind() == Type_kind::character) {
      /* TODO: characterだけでなく配列への代入をこのルートへ通す */

      std::vector<llvm::Value*> args;
      llvm::Value *size = builder.getInt32(this->lhs->get_len().eval_constant_value()+1);
      builder.CreateMemCpy(lhs, rhs, size, /* alignment= */ 1);
    } else {
      builder.CreateStore(rhs, lhs);
    }
  }

  void Output_statement::codegen() const
  {
    for (auto &elm : this->elements) {
      std::vector<llvm::Value*> args;
      args.push_back(elm->codegen());
      llvm::Function *callee;
      if (elm->get_type_kind() == Type_kind::i32) {
        callee = module->getFunction("_write_int");
      } else if (elm->get_type_kind() == Type_kind::fp32) {
        callee = module->getFunction("_write_float");
      } else if (elm->get_type_kind() == Type_kind::logical) {
        callee = module->getFunction("_write_logical");
      } else if (elm->get_type_kind() == Type_kind::character) {
        callee = module->getFunction("_write_string");
      } else {
        assert(0);
      }
      builder.CreateCall(callee, args, "call_tmp");
    }
  }

  void Block::codegen() const
  {
    for (auto &stmt : this->statements) {
      stmt->codegen();
    }
  }

  void If_construct::codegen() const
  {
    llvm::Value *cond_val = this->condition_expression->codegen();

    llvm::Function *func = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *then_BB = llvm::BasicBlock::Create(context, "then", func);
    llvm::BasicBlock *else_BB = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock *merge_BB = llvm::BasicBlock::Create(context, "ifcont");

    // cond_valに応じてthen_BBとelse_BBに分岐する命令をifの前のブロックに挿入
    builder.CreateCondBr(cond_val, then_BB, else_BB);

    // then_BBをカレントブロックにする
    builder.SetInsertPoint(then_BB);
    // then_BBにthen_BBのIRを生成
    this->then_block->codegen();
    // then_BBからmerge_BBへのjumpを挿入
    builder.CreateBr(merge_BB);

    // else側
    func->getBasicBlockList().push_back(else_BB);
    builder.SetInsertPoint(else_BB);
    this->else_block->codegen();
    builder.CreateBr(merge_BB);

    // 合流後
    func->getBasicBlockList().push_back(merge_BB);
    builder.SetInsertPoint(merge_BB);
  }

  void Do_construct::codegen() const
  {
    this->initial_expr->codegen();

    llvm::Function *func = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *preheaderBB = builder.GetInsertBlock();
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(context, "loop", func);
    builder.CreateBr(loopBB);

    builder.SetInsertPoint(loopBB);
    this->block->codegen();
    this->increment_expr->codegen();
    llvm::Value *cond = this->condition_expr->codegen();

    llvm::BasicBlock *loop_end_BB = builder.GetInsertBlock();
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(context, "after_loop", func);
    builder.CreateCondBr(cond, loopBB, afterBB);
    builder.SetInsertPoint(afterBB);
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

    for (std::string str : this->global_strings) {
      global_string_table[str] = builder.CreateGlobalStringPtr(str);
    }

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
    llvm::Value *size = nullptr;
    if (this->shape) {
      size = builder.getInt32(this->shape->get_size());
    }
    
    llvm::Value *value;
    if (this->get_type_kind() == Type_kind::i32) {
      value = builder.CreateAlloca(llvm::Type::getInt32Ty(context), size, this->name);
    } else if (this->get_type_kind() == Type_kind::fp32) {
      value = builder.CreateAlloca(llvm::Type::getFloatTy(context), size, this->name);
    } else if (this->get_type_kind() == Type_kind::logical) {
      value = builder.CreateAlloca(llvm::Type::getInt32Ty(context), size, this->name);
    } else if (this->get_type_kind() == Type_kind::character) {
      value = builder.CreateAlloca(llvm::Type::getInt8Ty(context), builder.getInt32(this->get_len().eval_constant_value()+1), this->name);
    }
    variable_table[this->name] = value;
  }
}
