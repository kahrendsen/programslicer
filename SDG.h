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

namespace llvm {

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
            bool ifMask(int mask) const { return attr & mask; }

        private:
            int attr;
    };

    /// SDG Node Attr
    enum NodeAttr
    {
        instruction,
        entry,
        callerAux,
        calleeAux,
        SIZE
    };

    /// SDG node type
    // TODO: implement
    class SDGNode
    {
        public:
            NodeAttr getAttr() const { return attr; }
            Value *getValue() const { return value; }

        private:
            NodeAttr attr;
            Value *value;
    };

    /// The class generates a control dependence graph for a function.
    class SDG : public ModulePass {
        public:
            typedef DirectedGraph<SDGNode, SDGEdge> SDG_t;

            // Pass Indentifier 
            static char ID;
            SDG() : ModulePass(ID), graph("SDG") {}

            virtual bool runOnModule(Module &F);

            virtual void getAnalysisUsage(AnalysisUsage& AU) const;

            SDG_t &getGraph() { return graph; }

        private:
            SDG_t graph;
    };

    char SDG::ID = 0;

    static RegisterPass<SDG> X("SDG", "SDG Pass",
            true /* only looks at SFG */,
            true /* analysis pass */);
} // end of anonymous namespace

#endif // SDG_H

