// Author Hao Wu and Ying-yu Chen

#include "llvm/Support/InstIterator.h"

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
                Instruction *I = S->getTerminator();
                instNodeMap.insert(std::pair<Instruction*, SDGNode>(I, SDGNode(instruction, I)));
                src = &instNodeMap[I];
            }
            else
            {
                entryNodeMap.insert(std::pair<Function*, SDGNode>(F, SDGNode(entry, F)));
                src = &entryNodeMap[F];
            }
            for (BasicBlock::iterator i = D->begin(), e = D->end(); i != e; ++i)
            {
                Instruction *I = &*i;
                instNodeMap.insert(std::pair<Instruction*, SDGNode>(I, SDGNode(instruction, I)));
                dst = &instNodeMap[I];
                graph.insert(src, dst);
            }
        }
    }

    // Step 1.2: Create DDG
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        Function &F = *it;
        generateIntraDDG(F);
    }

    // Step 2: INTERprocedure Analysis
    // Aux node for function call, etc.
    // Step X: TODO
    //
    return false;
}

bool SDG::generateIntraDDG(Function &F)
{
    // TODO: Consider pointers
    SDGNode *src, *dst;
    // Use use-def chain in LLVM
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    {
        errs() << *I << "\n";
        for (Value::use_iterator i = I->use_begin(), e = I->use_end(); i != e; ++i)
        {
            if (Instruction *Inst = dyn_cast<Instruction>(*i)) {
                errs() << "I is used in instruction:\n";
                errs() << *Inst << "\n";
                // Assert that I has been inserted during CDG construction
                assert(instNodeMap.find(&*I) != instNodeMap.end());
                assert(instNodeMap.find(&*Inst) != instNodeMap.end());
                src = &instNodeMap[&*I];
                dst = &instNodeMap[Inst];
                graph.insert(src, dst);
            }
        }
    }
    return false;
}

