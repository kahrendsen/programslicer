//===---------- CDG.h - Control Dependence Graph Creation     -------------===//
//
//              Developed under the LLVM Compiler Infrastructure
//
// Author: Hao Wu & Ying-Yu Chen
// Date: 2011.12.13
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that create the Control Dependence Graph (CDG)
// for each procedure.
//
//===----------------------------------------------------------------------===//

#ifndef CDG_H
#define CDG_H

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/raw_ostream.h"

#include "DirectedGraph.h"

using namespace llvm;

namespace llvm {

    /// The class generates a control dependence graph for a function.
    class CDG : public FunctionPass {
    public:
        typedef DirectedGraph<BasicBlock, void> _bbGraph_t;
        typedef DirectedGraph<Instruction, void> _instGraph_t;

        // Pass Indentifier 
        static char ID;
        CDG() : FunctionPass(ID), _bbGraph("blockCDG")/*, _instGraph("instructionCDG")*/ {}

        virtual bool runOnFunction(Function &F);

        virtual void getAnalysisUsage(AnalysisUsage& AU) const
        {
            AU.setPreservesCFG();
            AU.addRequired<PostDominatorTree>();
        }

        //_instGraph_t getInstGraph() const { return _instGraph; } 
        _bbGraph_t getBBGraph() const { return _bbGraph; }

    private:
        // The control dependence graph based on basic blocks
        _bbGraph_t _bbGraph;
        // The control dependence graph based on instructions
        //_instGraph_t _instGraph;
    };

    static RegisterPass<CDG> X("CDG", "CDG Pass",
            false /* only looks at CFG */,
            true /* analysis pass */);
} // end of anonymous namespace

#endif // CDG_H

