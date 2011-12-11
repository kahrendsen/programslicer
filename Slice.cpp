// Author: Hao Wu
//
#include "llvm/Support/InstIterator.h"

#include "SDG.h"
#include "Slice.h"

char Slice::ID = 0;

const char* Slice::initFileName = "slice.conf";

static RegisterPass<Slice> A("Slice", "Slice Pass",
        false /* only looks at CFG */,
        false /* analysis pass */);

bool Slice::runOnModule(Module &M)
{
    SDG &sdg = getAnalysis<SDG>();
    ifstream fin(initFileName);
    readInit(sdg, M, fin);
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
//    SDG::SDG_t &graph = sdg.getGraph();
//    SDG::SDG_t::nodeSet_t &nodeSet = graph.getNodeSet();
    SDG::SDG_t::nodeSet_t &nodeSet = resultSet;

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

void Slice::readInit(SDG &sdg, Module &M, std::istream &in)
{
    std::map<std::string, std::set<long> > toSliceList;
    std::string funcName;
    int instNum;
    while (true)
    {
        in >> funcName >> instNum;
        if (in.eof())
        {
            break;
        }
        toSliceList[funcName].insert(instNum);
    }

#if 0
    SDG::SDG_t &graph = sdg.getGraph();
    SDG::SDG_t::nodeSet_t &nodeSet = graph.getNodeSet();
    for (SDG::SDG_t::nodeSet_t::iterator it = nodeSet.begin(), e = nodeSet.end();
            it != e; ++it)
    {
        SDGNode *node = *it;
        if (node != NULL && node->getAttr() == instruction)
        {
            if (Instruction *I = dyn_cast<Instruction>(node->getValue()))
            {
                errs() << "name=" << I->getName() << ": " << *I << "\n";
                Function *F = I->getParent()->getParent();
                if (F->getName() == funcName && I->getName() == instName)
                {
                    markedNodes.insert(node);
                }
            }
        }
    }
#endif

    for (Module::iterator it = M.begin(), et = M.end(); it != et; ++it)
    {
        Function &F = *it;
        std::set<long> &toSliceListForFunc = toSliceList[F.getName()];
        if (toSliceListForFunc.empty())
            continue;
        long count;
        count = 0;
        for (inst_iterator jt = inst_begin(F), et = inst_end(F); jt != et; ++jt)
        {
            Instruction *I = &*jt;
            if (toSliceListForFunc.find(count++) != toSliceListForFunc.end())
            {
                SDGNode *node = &sdg.getInstNodeMap()[I];
                markedNodes.insert(node);
            }
        }
    }
}

