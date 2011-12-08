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

#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/raw_ostream.h"

#include "CDG.h"
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
    class SDGNode
    {
        public:
            SDGNode() : attr(SIZE), value(NULL) {}

            SDGNode(NodeAttr attr, Value *value) : attr(attr), value(value) {
                assert(attr < SIZE);
            }

            NodeAttr getAttr() const { return attr; }
            Value *getValue() const { return value; }

        private:
            NodeAttr attr;
            Value *value;
    };

    /// The class generates a system dependence graph for a function.
    class SDG : public ModulePass {
        public:
            typedef DirectedGraph<SDGNode, SDGEdge> SDG_t;

            // Pass Indentifier 
            static char ID;
            SDG() : ModulePass(ID), graph("SDG") {}

            virtual bool runOnModule(Module &F);

            virtual void getAnalysisUsage(AnalysisUsage& AU) const
            {
                AU.setPreservesCFG();
                AU.addRequired<CDG>();
            }


            SDG_t &getGraph() { return graph; }

        private:
            // The system dependence graph
            SDG_t graph;
            // The map for all instruction nodes
            std::map<Instruction *, SDGNode> instNodeMap;
            // The map for all function entry nodes
            std::map<Function *, SDGNode> entryNodeMap;

            bool generateIntraDDG(Function &F);
    };

    static RegisterPass<SDG> B("SDG", "SDG Pass",
            true /* only looks at SFG */,
            true /* analysis pass */);
} // end of anonymous namespace

#endif // SDG_H

