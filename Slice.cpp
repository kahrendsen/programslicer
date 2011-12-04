// Author: Hao Wu

#include "Slice.h"

bool Slice::runOnModule(Module &M)
{
    SDG &sdg = getAnalysis<SDG>();
    return markVerticesOfSlice(sdg, markedNodes);
}

bool Slice::markVerticesOfSlice(const SDG &sdg, Slice::nodeList_t &resultList)
{
    // Initialize result list
    const SDG::SDG_t &graph = sdg.getGraph();
    const SDG::SDG_t::nodeSet_t &nodeSet = graph.getNodeSet();
    resultList.assign(nodeSet.begin(), nodeSet.end());

    // Step 1: Slice without descendinginto called procedure
    // Step 2: Slice called procedures, without aschending to call sites
    return false;
}

bool markReachingVertices(const SDG &sdg, Slice::nodeList_t &resultList,
        Slice::nodeList_t workList, int mask)
{
    // TODO
    return false;
}

