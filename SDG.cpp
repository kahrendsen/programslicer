// Author Hao Wu and Ying-yu Chen

#include "SDG.h"

char SDG::ID = 0;

bool SDG::runOnModule(Module &M)
{
    // Step 1: INTRAprocedure Analysis
    // Step 1.1: Insert CDG
    CDG &cdg = getAnalysis<CDG>();
    CDG::_bbGraph_t _bbGraph = cdg.getBBGraph();
    errs() << cdg.getBBGraph();

    std::set<BasicBlock *> &nodeSet = _bbGraph.getNodeSet();
    SDGNode *src, *dst;
    for (std::set<BasicBlock *>::iterator fIt = nodeSet.begin(),
            e = nodeSet.end(); fIt != e; ++fIt)
    {
        BasicBlock *D = *fIt;
        Function *F = D->getParent();
        CDG::_bbGraph_t::nodeMap_t &predSet = _bbGraph.getPredSet(D);
        for (CDG::_bbGraph_t::nodeMap_t::const_iterator it = predSet.begin(),
                et = predSet.end(); it != et; ++it)
        {
            BasicBlock *S = it->first;
            if (S != NULL)
            {
                // XXX: Memory leak
                src = new SDGNode(instruction, S->getTerminator());
            }
            else
            {
                src = new SDGNode(entry, F);
            }
            for (BasicBlock::iterator i = D->begin(), e = D->end(); i != e; ++i)
            {
                dst = new SDGNode(instruction, &*i);
                graph.insert(src, dst);
            }
        }
    }

    // Step 2: INTERprocedure Analysis
    // Aux node for function call, etc.
    // Step X: TODO
    //
    return false;
}

