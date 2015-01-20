// Author: Hao Wu
//
#include "llvm/IR/InstIterator.h" 
#include "SDG.h"
#include "ForwardSlice.h"
#include <fstream>
#include "llvm/Support/raw_ostream.h"
#include <iostream>


char ForwardSlice::ID = 0;

const char* ForwardSlice::initFileName = "slice.conf";

static RegisterPass<ForwardSlice> A("forward-slice", "Forward Slice Pass",
        false /* only looks at CFG */,
        false /* analysis pass */);

//std::map<SDGNode*, std::pair<int,int>> boundsMap;
//bool useRange=true;

bool ForwardSlice::runOnModule(Module &M)
{
    SDG &sdg = getAnalysis<SDG>();
    //raw_os_ostream &o(std::cout);
    outs()<<sdg.getGraph();
    std::ifstream fin(initFileName);
    if (fin.is_open() == false) {
        std::cerr << "Error opening: " << initFileName << "\n";
        return false;
    }

    readInit(sdg, M, fin);
    fin.close();
    markVerticesOfForwardSlice(sdg, markedNodes);
    // TODO: implement
    sliceModule(sdg, M);
    // errs() << M << "\n";
    return true;
}

void ForwardSlice::sliceModule(SDG &sdg, Module &M)
{
    std::map<Instruction *, SDGNode> &instMap = sdg.getInstNodeMap();
    std::map<Function *, SDGNode> &entryMap = sdg.getEntryNodeMap();
    std::vector<Function *> functionToRemove;
    std::vector<Instruction *> instructionToRemove;
    //for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    //{
        //Function& function = *it;
        //if (entryMap.find(&function) == entryMap.end()) {
            //std::cerr << "Function " << function.getName().str() << " not found in entryMap\n";
            //continue;
        //}
        
        //SDGNode *funcEntryNode = &entryMap[&*it];
        //assert(funcEntryNode->getAttr() == entry);
        //if (markedNodes.find(funcEntryNode) == markedNodes.end())
        //{
            //errs() << "Function Removed: " << *it;
            //functionToRemove.push_back(&*it);
        //}
    //}
    //for (std::vector<Function *>::iterator it = functionToRemove.begin(),
            //e = functionToRemove.end(); it != e; ++it)
    //{
        //Function* function = *it;
        //errs() << "removing function: " << function->getName().str() << "\n";
        
        //(*it)->removeFromParent();
    //}
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
        errs() << "removing instruction: ";
        Instruction* instr = *it;
        instr->dump();
        (*it)->removeFromParent();
    }
}

bool ForwardSlice::markVerticesOfForwardSlice(SDG &sdg, ForwardSlice::nodeSet_t &resultSet)
{
    // Initialize result list
    SDG::SDG_t &graph = sdg.getGraph();
    // SDG::SDG_t::nodeSet_t &nodeSet = graph.getNodeSet();
    SDG::SDG_t::nodeSet_t &nodeSet = resultSet;

    // Step 1: Slice without descending into called procedure
    markReachingVerticesForForwardSlice(sdg, resultSet, nodeSet,
            control | flow | call | paramIn);
    // Step 2: Slice called procedures, without aschending to call sites
    markReachingVerticesForForwardSlice(sdg, resultSet, resultSet,
            control | flow | paramOut);
    return false;
}

bool ForwardSlice::markReachingVerticesForForwardSlice(SDG &sdg, ForwardSlice::nodeSet_t &resultSet,
        ForwardSlice::nodeSet_t workSet, int mask)
{
    while (!workSet.empty())
    {
        SDGNode *node = *workSet.begin();
        workSet.erase(workSet.begin());
        resultSet.insert(node);

        SDG::SDG_t &graph = sdg.getGraph();
        const SDG::SDG_t::nodeMap_t &succMap(graph.getSuccSet(node));
        InterProceduralRA<Cousot> &ra = getAnalysis<InterProceduralRA<Cousot> >();
        for (SDG::SDG_t::nodeMap_t::const_iterator it = succMap.begin(), e = succMap.end();
                it != e; ++it)
        {
            Instruction* i=NULL;

            //There's no easy way to convert from SDGNode to instruction...
            //for(std::map<Instruction*,SDGNode>::iterator pair = sdg.getInstNodeMap().begin(); pair!=sdg.getInstNodeMap().end();pair++){
                //if(&(pair->second) == it->first){
                    //i=pair->first;
                    //break;
                //}
            //}
             
            //assert(i!=NULL);
            //if(i==NULL)
            //{
                //workSet.insert(it->first);
            //}
            //else{
            
                //Range r = ra.getRange(i);
                if (resultSet.find(it->first) == resultSet.end() // Unmarked node
                        && it->second != NULL // There is an edge
                        && it->second->ifMask(mask))// Of the specified type
                        //&& i!=NULL 
                        //&& boundsMap[it->first].first<r.getUpper().getSExtValue()
                        //&& boundsMap[it->first].second>r.getLower().getSExtValue())//and it's within range 
                {
    //                if (it->first->getValue()->getName() == "add")
                    errs() << "ADD: " << *it->first << "\n";
                    workSet.insert(it->first);
                }
            //}
        }
    }
    return false;
}



void ForwardSlice::readInit(SDG &sdg, Module &M, std::istream &in)
{
    std::map<std::string, std::set<long> > toSliceList;
    std::string funcName;
    int instNum;
    int lower;
    int upper;
    std::map<int, std::pair<int, int>> lineToBounds;
    while (true)
    {
        in >> funcName >> instNum;
        if(!(in >> lower) || !(in >> upper))
            //useRange=false;
        std::cerr << "read: " << funcName << ", " << instNum << "\n";
        std::cerr << "range: " << lower << " , " << upper;
        //assert(0);
        if (in.eof())
        {
            break;
        }
        toSliceList[funcName].insert(instNum);
        lineToBounds[instNum]=std::make_pair(lower,upper);

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
                errs() << "Marked Node: " << *node << "\n";//this is where we mark the initial nodes to begin slicing on
                markedNodes.insert(node);
                //boundsMap[node]=std::pair<int,int>(lineToBounds[instNum].first,lineToBounds[instNum].second);
                

            }
        }
    }
    if(markedNodes.empty()){

        printf("Could not find any instructions from slice.conf! Perhaps you miscounted, or something is wrong with SDG\n") ;
    }
}

