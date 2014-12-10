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

#include "llvm/IR/Argument.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/raw_ostream.h"

#include "CDG.h"
#include "DirectedGraph.h"
#include "ptranalysis/include/anders-aa.h"

//#include "ptranalysis/include/PtsAnders.h"

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
            SDGEdge() : attr(0) {}
            SDGEdge(int attr) : attr(attr) {}
            bool hasControl() const;
            bool hasFlow() const;
            bool hasCall() const;
            bool hasParamIn() const;
            bool hashasParamOut() const;
            bool ifMask(int mask) const { return attr & mask; }

            void print(raw_ostream &O) const;

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

            void print(raw_ostream &O) const;

        protected:
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
                AU.addRequired<AndersAA>();
                
            }

            SDG_t &getGraph() { return graph; }

            std::map<Instruction *, SDGNode> &getInstNodeMap()
            {
                return instNodeMap;
            }

            std::map<Function *, SDGNode> &getEntryNodeMap()
            {
                return entryNodeMap;
            }

        private:
            // The system dependence graph
            SDG_t graph;
            // The map for all instruction nodes
            std::map<Instruction *, SDGNode> instNodeMap;
            // The map for all function entry nodes
            std::map<Function *, SDGNode> entryNodeMap;
            // The map for all callee input aux nodes
            std::map<Argument *, SDGNode> calleeInputNodeMap;
            // The map for all caller input aux nodes
            std::map<CallInst *, std::map<Value *, SDGNode> >callerInputNodeMap;
            // The map for all callee output aux nodes
            std::map<Function *, SDGNode> calleeOutputNodeMap;
            // The map for all caller output aux nodes
            std::map<CallInst *, std::map<Value *, SDGNode> >callerOutputNodeMap;
            // The map which records the def nodes of an allocation
            std::map<Instruction *, std::set<SDGNode *> > defNodeMap;
            // PtsSet
            //PtsAnders pts;

            void generateDefNodeMap(Module &M);
            void generateIntraDDG(Function &F);
            void generatePointerEdges(Module &M);
    };

    inline raw_ostream& operator <<(raw_ostream &OS, const SDGEdge &edge)
    {
        edge.print(OS);
        return OS;
    }

    inline raw_ostream& operator <<(raw_ostream &OS, const SDGNode &node)
    {
        node.print(OS);
        return OS;
    }

}

#endif // SDG_H

