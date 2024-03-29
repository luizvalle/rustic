/* 
 * record_function_io.cpp
 *
 * An LLVM function pass that instruments the code to print the inputs and
 * output of a function's invocation at runtime.
 *
 * The records are printed to a file specified as a
 * command line argument.
 */
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace std;

namespace {

    // Command-line argument
    static cl::opt<string> outputFile(
            "record-output-file",
            cl::desc("Path to the file where records will be written to"),
            cl::init("record_function_io_out.txt"));

    void printName(
            const Function& F, IRBuilder<>& builder,
            FunctionCallee& fprintfFunction, Value *fd) {
        Value *functionName = builder.CreateGlobalStringPtr(
                "name:" + F.getName().str() + ",");
        builder.CreateCall(fprintfFunction, {fd, functionName});
    }

    void printArguments(
            const Function& F, IRBuilder<>& builder,
            FunctionCallee& fprintfFunction, Value *fd) {
        unsigned int numArgs = 0;
        for (auto& Arg : F.args()) {
            numArgs++;
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
                builder.CreateCall(
                        fprintfFunction,
                        {
                            fd,
                            builder.CreateGlobalStringPtr("input:unknown_type,")
                        });
                continue;
            }

            string formatString = "input:" + type + ":" + formatSpecifier + ',';
            Value *formatConstant = builder.CreateGlobalStringPtr(formatString);

            const Value *argValue = &Arg;
            builder.CreateCall(
                    fprintfFunction,
                    {fd, formatConstant, const_cast<Value*>(argValue)});
        }

        if (numArgs == 0) {
            builder.CreateCall(
                    fprintfFunction,
                    {fd, builder.CreateGlobalStringPtr("input:void,")});
        }

    }

    struct RecordFunctionIO : PassInfoMixin<RecordFunctionIO> {
        PreservedAnalyses run (Function& F, FunctionAnalysisManager&) {
            LLVMContext& context = F.getContext();
            Module *module = F.getParent();

            // Create the declaration for the
            // FILE *fopen(const char *, const char *)
            // function
            FunctionType *fopenType = FunctionType::get(
                    Type::getInt8PtrTy(context),
                    {
                        Type::getInt8PtrTy(context),
                        Type::getInt8PtrTy(context),
                    }, false);
            FunctionCallee fopenFunction = module->getOrInsertFunction(
                    "fopen", fopenType);

            // Create the declaration for the
            // size_t fprintf(FILE *, const char *, ...)
            // function
            FunctionType *fprintfType = FunctionType::get(
                    Type::getInt32Ty(context),
                    {Type::getInt8PtrTy(context)}, true);
            FunctionCallee fprintfFunction = module->getOrInsertFunction(
                    "fprintf", fprintfType);

            // Create the declaration for the
            // int fclose(FILE *)
            // function
            FunctionType *fcloseType = FunctionType::get(
                    Type::getInt32Ty(context),
                    {Type::getInt8PtrTy(context)}, false);
            FunctionCallee fcloseFunction = module->getOrInsertFunction(
                    "fclose", fcloseType);

            // Print the output by inserting a print statement before every
            // the return statement
            for (auto& BB : F) {
                ReturnInst *returnInst = dyn_cast<ReturnInst>(
                        BB.getTerminator());
                if (!returnInst) {
                    continue;
                }

                IRBuilder<> returnBuilder(returnInst);
            
                Value *fd = returnBuilder.CreateCall(
                        fopenFunction,
                        {
                            returnBuilder.CreateGlobalStringPtr(outputFile),
                            returnBuilder.CreateGlobalStringPtr("a")
                        });

                Value *returnValue = returnInst->getReturnValue();
                if (!returnValue) {
                    // Returns void
                    printName(F, returnBuilder, fprintfFunction, fd);
                    printArguments(F, returnBuilder, fprintfFunction, fd);
                    returnBuilder.CreateCall(fprintfFunction,
                            {fd, returnBuilder.CreateGlobalStringPtr(
                                    "output:void,\n")});
                    returnBuilder.CreateCall(fcloseFunction, {fd});
                    continue;
                }

                string formatSpecifier, type;

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
                    printName(F, returnBuilder, fprintfFunction, fd);
                    printArguments(F, returnBuilder, fprintfFunction, fd);
                    returnBuilder.CreateCall(
                            fprintfFunction,
                            {fd, returnBuilder.CreateGlobalStringPtr(
                                    "output:unknown_type,\n")});
                    returnBuilder.CreateCall(fcloseFunction, {fd});
                    continue;
                }

                printName(F, returnBuilder, fprintfFunction, fd);
                printArguments(F, returnBuilder, fprintfFunction, fd);

                string formatString = (
                        "output:" + type + ":" + formatSpecifier + ",\n");
                Value *formatConstant = returnBuilder.CreateGlobalStringPtr(
                        formatString);

                returnBuilder.CreateCall(
                        fprintfFunction, {fd, formatConstant, returnValue});

                returnBuilder.CreateCall(fcloseFunction, {fd});
            }

            // Function has been modified
            return PreservedAnalyses::none();
        }

        // Run even if the function is marked as noopt
        static bool isRequired() { return true; } 
    };
} // namespace

PassPluginLibraryInfo getRecordFunctionIOPluginInfo() {
    const auto callback = [](PassBuilder& PB) {
        PB.registerPipelineEarlySimplificationEPCallback(
                [&](ModulePassManager& MPM, auto) {
                    MPM.addPass(createModuleToFunctionPassAdaptor(RecordFunctionIO()));
                    return true;
                });
    };
    return {
            LLVM_PLUGIN_API_VERSION,
            "RecordFunctionIO",
            LLVM_VERSION_STRING,
            callback
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getRecordFunctionIOPluginInfo();
}
