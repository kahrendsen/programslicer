//===- CDG.cpp - Print source line numbers as comments -------------===//
//
//              Developed under the LLVM Compiler Infrastructure
//
// Author: Ying-Yu Chen
// Date: 2011.12.13
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that create the Control Dependence Graph (CDG)
// for each procedure.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Support/FormattedStream.h"

#include "DirectedGraph.h"

using namespace llvm;

namespace {

  /// This struct implements the pass that prints the source line number as
  /// comments right after each instruction in a function.
  class CDG : public FunctionPass {
  public:
    // Pass Indentifier 
    static char ID;
    CDG() : FunctionPass(ID) {}
    
    virtual bool runOnFunction(Function &F) {
      errs() << "Function: " << F << "\n";
      
      DirectedGraph cdg;
      cdg
      
      return false;
    }
    
    virtual void getAnalysisUsage(AnalysisUsage& AU) const
    {
      AU.setPreservesCFG();
      AU.addRequired<PostDominatorTree>();
    }
  }; // end of struct CDG
  
  char CDG::ID = 0;
  
  static RegisterPass<CDG> X("CDG", "CDG Pass",
                        false /* only looks at CFG */,
                        false /* analysis pass */);
} // end of anonymous namespace

