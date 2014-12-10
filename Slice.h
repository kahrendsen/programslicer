//===---------- Slice.h - Program Slicing given System Dependence Graph ---===//
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

#ifndef SLICE_H
#define SLICE_H

#include "DirectedGraph.h"
#include "SDG.h"

using namespace llvm;

namespace llvm {

    /// The class generates a control dependence graph for a function.
    class Slice : public ModulePass {
    public:
        typedef std::set<SDGNode*> nodeSet_t;
        // Pass Indentifier 
        static char ID;
        Slice() : ModulePass(ID) {}

        virtual bool runOnModule(Module &F);

        virtual void getAnalysisUsage(AnalysisUsage& AU) const
        {
            AU.setPreservesAll();
            AU.addRequired<SDG>();
            AU.addRequired<AndersAA>();
        }

    private:

        /// Mark the reaching vertices to construct a slice, depending on a
        /// system dependence graph.
        /// sdg: system dependence graph
        /// resultSet: the result marked node list
        static bool markVerticesOfSlice(SDG &sdg, nodeSet_t &resultSet);

        /// mark the reaching vertices to a set of vertices, with a certain edge
        /// type mask.
        /// sdg: system dependence graph
        /// workSet: the initial work list, COPY-ON-VALUE
        /// resultSet: the result marked node list
        /// mask: edge mask to be considered.
        static bool markReachingVertices(SDG &sdg, nodeSet_t &resultSet,
                nodeSet_t workSet, int mask);

        /// Slice only nodes within the markedNodes
        void sliceModule(SDG &sdg, Module &M);

        /// Read the initial nodes from a given list
        void readInit(SDG &sdg, Module &M, std::istream &in);

        /// The input file
        static const char *initFileName;

        nodeSet_t markedNodes;
    };
} // end of anonymous namespace

#endif // SLICE_H

