// TODO: Add header

#include "CDG.h"

char CDG::ID = 0;

static RegisterPass<CDG> X("CDG", "CDG Pass",
        false /* only looks at CFG */,
        true /* analysis pass */);

bool CDG::runOnFunction(Function &F)
{
    errs() << "Function: " << F << "\n";

    PostDominatorTree &PDT = getAnalysis<PostDominatorTree>();

    // Iterate over basic blocks
    for (Function::iterator fIt = F.begin(), e = F.end(); fIt != e; ++fIt)
    {
        BasicBlock *B = &*fIt;
        DomTreeNode *BNode = PDT.getNode(B);

        for (succ_iterator suIt = succ_begin(B), suE = succ_end(B); suIt != suE; ++suIt)
        {
            BasicBlock *N = *suIt;
            DomTreeNode *NNode = PDT.getNode(N);

            while (BNode->getIDom() != NNode)
            {
                _bbGraph.insert(B, N);
                NNode = NNode->getIDom();
                if (NNode == NULL)
                {
                    break;
                }
                N = NNode->getBlock();
            }
        }
    }

    // Make blocks which has no dependence to be dependent on ENTRY
    for (Function::iterator fIt = F.begin(), e = F.end(); fIt != e; ++fIt)
    {
        BasicBlock *B = &*fIt;
        const _bbGraph_t::nodeMap_t &predSet = _bbGraph.getPredSet(B);
        if (predSet.empty())
        {
            _bbGraph.insert(NULL, B);
        }
    }
    errs() << _bbGraph;

    // Make instruction graph
#if 0
    Instruction *src, *dst;
    for (Function::iterator fIt = F.begin(), e = F.end(); fIt != e; ++fIt)
    {
        BasicBlock *D = &*fIt;
        const _bbGraph_t::nodeMap_t &predSet = _bbGraph.getPredSet(D);
        for (_bbGraph_t::nodeMap_t::const_iterator it = predSet.begin(), et = predSet.end();
                it != et; ++it)
        {
            BasicBlock *S = it->first;
            if (S != NULL)
            {
                src = S->getTerminator();
            }
            else
            {
                src = NULL;
            }
            for (BasicBlock::iterator i = D->begin(), e = D->end(); i != e; ++i)
            {
                dst = &*i;
                _instGraph.insert(src, dst);
            }
        }
    }

    errs() << _instGraph;
#endif

    return false;
}

