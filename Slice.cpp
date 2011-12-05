// Author: Hao Wu

#include "SDG.h"
#include "Slice.h"

bool Slice::runOnModule(Module &M)
{
    SDG &sdg = getAnalysis<SDG>();
    return markVerticesOfSlice(sdg, markedNodes);
}

bool Slice::markVerticesOfSlice(SDG &sdg, Slice::nodeSet_t &resultSet)
{
    // Initialize result list
    const SDG::SDG_t &graph = sdg.getGraph();
    const SDG::SDG_t::nodeSet_t &nodeSet = graph.getNodeSet();

    // Step 1: Slice without descending into called procedure
    markReachingVertices(sdg, resultSet, nodeSet,
            control | flow | call | paramIn);
    // Step 2: Slice called procedures, without aschending to call sites
    markReachingVertices(sdg, resultSet, resultSet,
            control | flow | paramOut);
    return false;
}

bool markReachingVertices(SDG &sdg, Slice::nodeSet_t &resultSet,
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

