// TODO: Add header

#include "CDG.h"

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

            while (NNode->getIDom() != BNode)
            {
                _bbGraph.Insert(B, N);
                NNode = NNode->getIDom();
                N = NNode->getBlock();
            }
        }
    }

    return false;
}

