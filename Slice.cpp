// Author: Hao Wu
//
#include "llvm/Support/InstIterator.h"

#include "SDG.h"
#include "Slice.h"

char Slice::ID = 0;

static RegisterPass<Slice> A("Slice", "Slice Pass",
        false /* only looks at CFG */,
        false /* analysis pass */);

bool Slice::runOnModule(Module &M)
{
    SDG &sdg = getAnalysis<SDG>();
    markVerticesOfSlice(sdg, markedNodes);
    // TODO: implement
    sliceModule(sdg, M);
    errs() << M << "\n";
    return true;
}

void Slice::sliceModule(SDG &sdg, Module &M)
{
    std::map<Instruction *, SDGNode> &instMap = sdg.getInstNodeMap();
    std::map<Function *, SDGNode> &entryMap = sdg.getEntryNodeMap();
    std::vector<Function *> functionToRemove;
    std::vector<Instruction *> instructionToRemove;
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        SDGNode *funcEntryNode = &entryMap[&*it];
        assert(funcEntryNode->getAttr() == entry);
        if (markedNodes.find(funcEntryNode) == markedNodes.end())
        {
            functionToRemove.push_back(&*it);
        }
    }
    for (std::vector<Function *>::iterator it = functionToRemove.begin(),
            e = functionToRemove.end(); it != e; ++it)
    {
        (*it)->removeFromParent();
    }
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        for (inst_iterator jt = inst_begin(*it), et = inst_end(*it);
                jt != et; ++jt)
        {
            Instruction *I = &*jt;
            SDGNode *instNode = &instMap[I];
            assert(instNode->getAttr() == instruction);
            if (markedNodes.find(instNode) == markedNodes.end())
            {
                instructionToRemove.push_back(I);
            }
        }
    }
    for (std::vector<Instruction *>::iterator it = instructionToRemove.begin(),
            e = instructionToRemove.end(); it != e; ++it)
    {
        (*it)->removeFromParent();
    }
}

bool Slice::markVerticesOfSlice(SDG &sdg, Slice::nodeSet_t &resultSet)
{
    // Initialize result list
    SDG::SDG_t &graph = sdg.getGraph();
    SDG::SDG_t::nodeSet_t &nodeSet = graph.getNodeSet();

    // Step 1: Slice without descending into called procedure
    markReachingVertices(sdg, resultSet, nodeSet,
            control | flow | call | paramIn);
    // Step 2: Slice called procedures, without aschending to call sites
    markReachingVertices(sdg, resultSet, resultSet,
            control | flow | paramOut);
    return false;
}

bool Slice::markReachingVertices(SDG &sdg, Slice::nodeSet_t &resultSet,
        Slice::nodeSet_t workSet, int mask)
{
    while (!workSet.empty())
    {
        SDGNode *node = *workSet.begin();
        workSet.erase(workSet.begin());
        resultSet.insert(node);

        SDG::SDG_t &graph = sdg.getGraph();
        const SDG::SDG_t::nodeMap_t &predMap(graph.getPredSet(node));
        for (SDG::SDG_t::nodeMap_t::const_iterator it = predMap.begin(), e = predMap.end();
                it != e; ++it)
        {
            if (resultSet.find(it->first) == resultSet.end() // Unmarked node
                    && it->second != NULL // There is an edge
                    && it->second->ifMask(mask)) // Of the specified type
            {
                workSet.insert(it->first);
            }
        }
    }
    return false;
}

