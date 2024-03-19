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

using namespace llvm;
using namespace std;

namespace {

    struct CollectionFunctionTypeStats : PassInfoMixin<CollectionFunctionTypeStats> {
        PreservedAnalyses run (Function& F, FunctionAnalysisManager&) {
            // Function has not been modified
            return PreservedAnalyses::all();
        }

        // Run even if the function is marked as noopt
        static bool isRequired() { return true; } 
    };
} // namespace

PassPluginLibraryInfo getCollectionFunctionTypeStatsPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CollectionFunctionTypeStats", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "collect_function_types") {
                    FPM.addPass(CollectionFunctionTypeStats());
                    return true;
                  }
                  return false;
                });
		  }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCollectionFunctionTypeStatsPluginInfo();
}
