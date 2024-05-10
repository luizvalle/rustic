/* 
 * collect_function_type_stats.cpp
 *
 * An LLVM function pass that prints the types of arguments that are tak
 *
 */
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <cstddef>
#include <string>

using namespace llvm;
using namespace std;

namespace {
    string typeToString(const Type *type) {
        string typeStr;
        raw_string_ostream rso(typeStr);
        type->print(rso);
        return typeStr;
    }

    struct CollectionFunctionTypes : PassInfoMixin<CollectionFunctionTypes> {
        PreservedAnalyses run (Function& F, FunctionAnalysisManager&) {
            string functionName = F.getName().str();
            errs() << functionName << ",args:[";
            size_t numArgs = 0;
            for (const auto& arg : F.args()) {
                string argType = typeToString(arg.getType());
                errs() << argType << ';';
                numArgs++;
            }
            errs() << "],";
            string returnType = typeToString(F.getReturnType());
            errs() << "ret:" << returnType << '\n';

            // Function has not been modified
            return PreservedAnalyses::all();
        }

        // Run even if the function is marked as noopt
        static bool isRequired() { return true; } 
    };
} // namespace

PassPluginLibraryInfo getCollectionFunctionTypesPluginInfo() {
    const auto callback = [](PassBuilder& PB) {
        PB.registerPipelineEarlySimplificationEPCallback(
                [&](ModulePassManager& MPM, auto) {
                    MPM.addPass(createModuleToFunctionPassAdaptor(CollectionFunctionTypes()));
                    return true;
                });
    };
    return {
            LLVM_PLUGIN_API_VERSION,
            "CollectionFunctionTypes",
            LLVM_VERSION_STRING,
            callback
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCollectionFunctionTypesPluginInfo();
}
