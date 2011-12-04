//===---------- SDG.h - Control Dependence Graph Creation     -------------===//
//
//              Developed under the LLVM Compiler Infrastructure
//
// Author: Hao Wu & Ying-Yu Chen
// Date: 2011.12.13
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that create the System Dependence Graph (SDG)
// for each module.
//
//===----------------------------------------------------------------------===//

#ifndef SDG_H
#define SDG_H

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/raw_ostream.h"

#include "DirectedGraph.h"

using namespace llvm;

namespace {

    /// Different Edge Attribute Values
    enum EdgeAttr
    {
        control = 1,
        flow = 2,
        call = 4,
        paramIn = 8,
        paramOut = 16,
        MASK = 32
    };

    /// The class defines an edge type.
    /// TODO: more function call
    class SDGEdge
    {
        public:
            bool hasControl() const;
            bool hasFlow() const;
            bool hasCall() const;
            bool hasParamIn() const;
            bool hashasParamOut() const;
        private:
            int attr;
    }

    /// SDG Node Attr
    enum NodeAttr
    {
        instruction,
        entry,
        callerAux,
        calleeAux,
        SIZE
    }

    /// SDG node type
    // TODO: implement
    class SDGNode
    {
        private:
            NodeAttr attr;
            Instruction *inst;
            Function *func;
    }

    /// The class generates a control dependence graph for a function.
    class SDG : public ModulePass {
    public:
        // Pass Indentifier 
        static char ID;
        SDG() : ModulePass(ID), _instGraph("SDG") {}

        virtual bool runOnFunction(Function &F);

        virtual void getAnalysisUsage(AnalysisUsage& AU) const;

    private:
        // The system dependence graph based on instructions
        typedef DirectedGraph<Instruction, EdgeType> _instGraph_t;
        _instGraph_t _instGraph;
    };

    char SDG::ID = 0;

    static RegisterPass<SDG> X("SDG", "SDG Pass",
            false /* only looks at SFG */,
            false /* analysis pass */);
} // end of anonymous namespace

#endif // SDG_H

