//===- TestPA.cpp - Print source line numbers as comments -------------===//
//
//              Developed under the LLVM Compiler Infrastructure
//
// Author: Ying-Yu Chen
// Date: 2011.09.07
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that prints source line numbers for each 
// instruction as comments.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/DebugInfo.h"

#include "../PA/PtsAnders.h"
#include "../PA/PtsSfs.h"

#include <vector>

using namespace llvm;

namespace {
  struct TestPA : public ModulePass {
    // Pass Indentifier 
    static char ID;
    TestPA() : ModulePass(ID) {}
    
    virtual bool runOnModule(Module &M) {
      errs() << "run Sfs M begin\n";
      //ptsAnders.runOnModule(M);
      ptsSfs.runOnModule(M);
      errs() << "run M end\n";
      
      for(Module::iterator it=M.begin(); it!=M.end(); ++it)
      {
        Function &F = *it;
        runOnFunction(F);
      }
      
      return false;
    }
    
    void runOnFunction(Function &F)
    {
      for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
      {
        std::vector<Value*> ptsSet;
        //ptsAnders.getPtsSet(&*I, ptsSet);
        ptsSfs.getPtsSet(&*I, ptsSet);
        errs() << *I << " : ";
        for(std::vector<Value*>::iterator it=ptsSet.begin(); it!= ptsSet.end(); ++it)
        {
          if(*it == NULL) continue;
          //ptsAnders.printVal(*it);
          errs() << **it;
          errs() << ", ";
        }
        errs() << "\n";
      }
    }
    
    protected:
      PtsAnders ptsAnders;
      PtsSfs ptsSfs;
  }; // end of struct TestPA
  
  char TestPA::ID = 0;
  
  static RegisterPass<TestPA> X("testpa", "Test PA Pass",
                        false /* only looks at CFG */,
                        false /* analysis pass */);
} // end of anonymous namespace

