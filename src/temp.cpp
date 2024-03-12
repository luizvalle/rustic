#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <iostream>
#include <string>

using namespace llvm;
using namespace std;

namespace {
    struct RuntimePrintFunctionCallCountPass : public FunctionPass {
        static char ID;
        RuntimePrintFunctionCallCountPass() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function& F) {
            LLVMContext& context = F.getContext();
            auto module = F.getParent();

            FunctionType *printfType = FunctionType::get(Type::getInt32Ty(context), {Type::getInt8PtrTy(context)}, true);
            FunctionCallee printfFunction = module->getOrInsertFunction("printf", printfType);

            string functionName = F.getName().str();
            string functionCallVarName = functionName + "_callCount";
            GlobalVariable *functionCallCount = module->getGlobalVariable(functionCallVarName);
            if (!functionCallCount) {
                functionCallCount = new GlobalVariable(
                        *module, Type::getInt32Ty(context), false,
                        GlobalValue::CommonLinkage, 0, functionCallVarName);
                functionCallCount->setInitializer(ConstantInt::get(Type::getInt32Ty(context), 0));
            }

            Instruction *firstInstruction = &F.front().front();
            IRBuilder<> builder(firstInstruction);

            Value *loadedCallCount = builder.CreateLoad(functionCallCount);
            Value *addedCallCount = builder.CreateAdd(loadedCallCount, builder.getInt32(1));
            builder.CreateStore(addedCallCount, functionCallCount);

            string printLog = functionName + "%d\n";
            Value *funcNamePtr = builder.CreateGlobalStringPtr(printLog);
            builder.CreateCall(printfFunction, {funcNamePtr, addedCallCount});

            return true;
        }
    };
} // namespace

char RuntimePrintFunctionCallCountPass::ID = 0;

static RegisterPass<RuntimePrintFunctionCallCountPass> X("rpfccp", "Runtime Function Call & Count Print Pass", false, false);
