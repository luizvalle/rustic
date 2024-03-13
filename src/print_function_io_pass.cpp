#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace {
    struct PrintFunctionIOPass : PassInfoMixin<PrintFunctionIOPass> {
        PreservedAnalyses run (Function &F, FunctionAnalysisManager &) {
            LLVMContext &context = F.getContext();
            Module *module = F.getParent();

            // Create the declaration for the printf function
            FunctionType *printfType = FunctionType::get(
                    Type::getInt32Ty(context), {Type::getInt8PtrTy(context)}, true);
            FunctionCallee printfFunction = module->getOrInsertFunction("printf", printfType);

            // Create builder at the beginning of the function
            Instruction *firstInstruction = &F.front().front();
            IRBuilder<> builder(firstInstruction);

            // Print the function name
            Value *functionName = builder.CreateGlobalStringPtr("name:" + F.getName().str() + ",");
            builder.CreateCall(printfFunction, {functionName});

            // Print the arguments
            for (auto &Arg : F.args()) {
                string formatSpecifier, type;

                if (Arg.getType()->isIntegerTy()) {
                    type = "integer";
                    formatSpecifier = "%d";
                } else if (Arg.getType()->isFloatTy()) {
                    type = "float";
                    formatSpecifier = "%f";
                } else if (Arg.getType()->isDoubleTy()) {
                    type = "double";
                    formatSpecifier = "%lf";
                } else if (Arg.getType()->isPointerTy()) {
                    type = "pointer";
                    formatSpecifier = "%p";
                } else {
                    builder.CreateCall(printfFunction, {builder.CreateGlobalStringPtr("input:unknown_type,")});
                    continue;
                }

                string formatString = "input:" + type + ":" + formatSpecifier + ',';
                Value *formatConstant = builder.CreateGlobalStringPtr(formatString);

                Value *argValue = &Arg;
                builder.CreateCall(printfFunction, {formatConstant, argValue});
            }

            if (F.getReturnType()->isVoidTy()) {
                // If the function returns void, we are done
                Value *voidReturnConstant = builder.CreateGlobalStringPtr("output:void\n");
                builder.CreateCall(printfFunction, {voidReturnConstant});
                return PreservedAnalyses::none(); // Function has been modified
            }

            // Function does not return void

            // Print the output by inserting a print statement before every the return statement
            for (auto& BB : F) {
                ReturnInst *returnInst = dyn_cast<ReturnInst>(BB.getTerminator());
                if (!returnInst) {
                    continue;
                }

                Value *returnValue = returnInst->getReturnValue();
                if (!returnValue) {
                    continue;
                }

                string formatSpecifier, type;
                IRBuilder<> returnBuilder(returnInst);

                if (returnValue->getType()->isIntegerTy()) {
                    type = "integer";
                    formatSpecifier = "%d";
                } else if (returnValue->getType()->isFloatTy()) {
                    type = "float";
                    formatSpecifier = "%f";
                } else if (returnValue->getType()->isDoubleTy()) {
                    type = "double";
                    formatSpecifier = "%lf";
                } else if (returnValue->getType()->isPointerTy()) {
                    type = "pointer";
                    formatSpecifier = "%p";
                } else {
                    returnBuilder.CreateCall(printfFunction, {builder.CreateGlobalStringPtr("output:unknown_type,\n")});
                    continue;
                }

                string formatString = "output:" + type + ":" + formatSpecifier + ",\n";
                Value *formatConstant = builder.CreateGlobalStringPtr(formatString);

                returnBuilder.CreateCall(printfFunction, {formatConstant, returnValue});
            }

            return PreservedAnalyses::none(); // Function has been modified
        }

        static bool isRequired() { return true; } // Run even if the function is marked as noopt
    };
} // namespace

llvm::PassPluginLibraryInfo getPrintFunctionIOPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PrintFunctionIOPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "print-function-io") {
                    FPM.addPass(PrintFunctionIOPass());
                    return true;
                  }
                  return false;
                });
		  }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPrintFunctionIOPassPluginInfo();
}
