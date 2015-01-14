// Author Hao Wu and Ying-yu Chen

#include "llvm/IR/InstIterator.h"

#include "SDG.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Support/raw_os_ostream.h"
//#include "ptranalysis/include/anders.h"

char SDG::ID = 0;

static RegisterPass<SDG> B("SDG", "SDG Pass",
        true /* only looks at SFG */,
        true /* analysis pass */);

const char *nodeAttrString[] =
{
    "instruction",
    "entry",
    "callerAux",
    "calleeAux",
    "INVALID"
};

bool SDG::runOnModule(Module &M)
{
    SDGNode *src, *dst;

    //errs() << "pts: begin\n";
    //pts.runOnModule(M);
    //errs() << "pts: end\n";

    // Step 1: INTRAprocedure Analysis - CDG
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        if (it->isDeclaration())
        {
            continue;
        }
        Function &F = *it;
        CDG &cdg = getAnalysis<CDG>(F);

        // Add normal control edges
        CDG::_bbGraph_t _bbGraph = cdg.getBBGraph();
        // errs() << cdg.getBBGraph();

        std::set<BasicBlock *> &nodeSet = _bbGraph.getNodeSet();
        for (std::set<BasicBlock *>::iterator fIt = nodeSet.begin(),
                e = nodeSet.end(); fIt != e; ++fIt)
        {
            BasicBlock *D = *fIt;
            if (D == NULL)
            {
                continue;
            }
            Function *F = D->getParent();
            CDG::_bbGraph_t::nodeMap_t &predSet = _bbGraph.getPredSet(D);
            for (CDG::_bbGraph_t::nodeMap_t::const_iterator it = predSet.begin(),
                    et = predSet.end(); it != et; ++it)
            {
                BasicBlock *S = it->first;
                if (S != NULL)
                {
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
                    //XXX: Memory leak
                    graph.insert(src, dst, new SDGEdge(control));
                }
            }
        }

        // Add Callee aux nodes
        src = &entryNodeMap[&F];
        for (Function::arg_iterator arg_it = F.arg_begin(), arg_e = F.arg_end();
                arg_it != arg_e; ++arg_it)
        {
            Argument *arg = &*arg_it;
            calleeInputNodeMap.insert(std::pair<Argument*, SDGNode>(
                        arg, SDGNode(calleeAux, arg)));
            // Add Control Edge
            dst = &calleeInputNodeMap[arg];
            // XXX: Memory Leak
            graph.insert(src, dst, new SDGEdge(control));
        }
        calleeOutputNodeMap.insert(std::pair<Function*, SDGNode>(
                    &F, SDGNode(calleeAux, &F)));
        dst = &calleeOutputNodeMap[&F];
        // XXX: Memory Leak
        graph.insert(src, dst, new SDGEdge(control));
//        graph.insert(dst, src, new SDGEdge(control));
    }

    // Step 2: INTERprocedure Analysis
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        Function &F = *it;

        // Add caller aux nodes and relationships
        for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It)
        {
            Instruction *I = &*It;
            if (CallInst *CI = dyn_cast<CallInst>(I))
            {
                std::map<Value *, SDGNode> &callerInputMap = callerInputNodeMap[CI];
                std::map<Value *, SDGNode> &callerOutputMap = callerOutputNodeMap[CI];
                Function *CF = CI->getCalledFunction();
                if (CF->isDeclaration()) {
                    continue;
                }
                //                Function::ArgumentListType &CFAL = CF->getArgumentList();
                // Step 2.1: Add call edges
                src = &instNodeMap[CI];
                dst = &entryNodeMap[CF];
                assert(src->getValue());
                assert(dst->getValue());
                //XXX: Memory leak
                graph.insert(src, dst, new SDGEdge(call));
                // Step 2.2: Add aux nodes for function call.
                unsigned numArgs = CI->getNumArgOperands();
                Function::arg_iterator arg_it = CF->arg_begin();
                for (unsigned i = 0; i < numArgs; ++i)
                {
                    // The callee should have at least the same number of
                    // argument as the caller, otherwise the function call is
                    // invalid.
                    assert(arg_it != CF->arg_end());
                    Argument *calleeArg = &*arg_it;
                    ++arg_it;

                    // Add aux nodes for callee input
                    Value *callerArg = CI->getArgOperand(i);
                    callerInputMap.insert(std::pair<Value *, SDGNode>(
                                callerArg,
                                SDGNode(callerAux, callerArg)));
                    // Add control edges
                    src = &instNodeMap[CI];
                    dst = &callerInputMap[callerArg];
                    graph.insert(src, dst, new SDGEdge(control));
                    // Add aux edges for callee and caller input
                    src = &callerInputMap[callerArg];
                    dst = &calleeInputNodeMap[calleeArg];
                    // XXX: Memory leak
                    graph.insert(src, dst, new SDGEdge(paramIn));
                }
                // Add return value as output aux nodes
                callerOutputMap.insert(std::pair<Value *, SDGNode>(
                            CI,
                            SDGNode(callerAux, CI)));
                // Add control edges
                src = &instNodeMap[CI];
                dst = &callerOutputMap[CI];
                graph.insert(src, dst, new SDGEdge(control));

                // Add param out edges
                src = &calleeOutputNodeMap[CF];
                dst = &callerOutputMap[CI];
                // XXX: Memory leak
                graph.insert(src, dst, new SDGEdge(paramOut));
            }
        }
    }

    // Step 3: Create DDG without pointer analysis
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        Function &F = *it;
        generateIntraDDG(F);
    }

    // Step 4: Create DDG with pointer analysis
    generateDefNodeMap(M);
    generatePointerEdges(M);

    return false;
}

void SDG::generateIntraDDG(Function &F)
{
    // TODO: Consider pointers
    SDGNode *src, *dst;
    // Use du chain in LLVM
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    {
        assert(instNodeMap.find(&*I) != instNodeMap.end());
        dst = &instNodeMap[&*I];
        for (User::op_iterator i = I->op_begin(), e = I->op_end(); i != e; ++i)
        {
            Value *def = *i;
            if (Instruction *Inst = dyn_cast<Instruction>(def))
            {
                assert(instNodeMap.find(Inst) != instNodeMap.end());
                src = &instNodeMap[Inst];
                //XXX: Memory leak
                graph.insert(src, dst, new SDGEdge(flow));
            }
            else if (Argument *arg = dyn_cast<Argument>(def))
            {
                assert(calleeInputNodeMap.find(arg) != calleeInputNodeMap.end());
                src = &calleeInputNodeMap[arg];
                //XXX: Memory leak
                graph.insert(src, dst, new SDGEdge(flow));
            }
        }

        // Add return flow edge
        if (I->getOpcode() == Instruction::Ret)
        {
            src = &calleeOutputNodeMap[&F];
            //XXX: Memory leak
            graph.insert(dst, src, new SDGEdge(flow));
        }
        // Add call return flow edge
        if (CallInst *CI = dyn_cast<CallInst>(&*I))
        {
            src = &callerOutputNodeMap[CI][CI];
            //XXX: Memory leak
            graph.insert(src, dst, new SDGEdge(flow));
        }
    }
}

void SDG::generateDefNodeMap(Module &M)
{
    AndersenAA &andersaa = getAnalysis<AndersenAA>();

    AliasSetTracker tracker(andersaa);
    for(Module::iterator f = M.begin(),e=M.end(); f!=e; f++){
        for(Function::iterator b = f->begin(), eb=f->end(); b!=eb; b++){
            tracker.add(*b);
        }

    }
    for (Module::iterator mi = M.begin(), me = M.end(); mi != me; ++mi)
    {
        Function &F = *mi;
        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
        {
           if (I->getOpcode() == Instruction::Store)
           {
                StoreInst *SI = cast<StoreInst>(&*I);
                Value *v = SI->getPointerOperand();

                Type *type = v->getType();
//                if(type->isPointerTy()){ //We're storing a pointer in memory, we need to do pointer analysis on it
                    //std::vector<const llvm::Value*> ptSet;
                    //andersaa.anders->getPointsToSet(v,ptSet);

                if(type->isPointerTy()){
                    //AAMDNodes nodes;
                    //SI->getAAMetadata(nodes); //Should we use v instead of SI?
                    AliasSet& ptSet = tracker.getAliasSetForPointer(v,M.getDataLayout()->getTypeAllocSize(type),nullptr);

                    for(AliasSet::iterator asit = ptSet.begin(), asend=ptSet.end(); asit!=asend; asit++)
                    {
                        Value* val = &(*asit.getPointer());
                        assert(isa<Instruction>(val));
                        Instruction *inst = cast<Instruction>(val);
                        defNodeMap[inst].insert(&instNodeMap[&*I]);
                    }
                }



//                    for (std::vector<const llvm::Value*>::iterator it = ptSet.begin(), et = ptSet.end();
//                            it != et; ++it)
//                    {
//                        const Value * val = *it;
//                        assert(isa<Instruction>(val));
//                        const Instruction *inst = cast<const Instruction>(val);
//                        defNodeMap[const_cast<Instruction *>(inst)].insert(&instNodeMap[&*I]);
//                    }

//                }

//                Type *type = v->getType();
//                if(type->isPointerTy()){ //We're storing a pointer in memory, we need to do pointer analysis on it
//                    const std::vector<u32>* ptSet = andersaa.anders->pointsToSet(v,0);
//                    //pts.getPtsSet(v, ptSet);
//                    for (std::vector<u32>::const_iterator it = ptSet->begin(), et = ptSet->end();
//                            it != et; ++it)
//                    {
//                        Value* val = andersaa.anders->get_nodes()[*it]->get_val(); //Anders gives us back nodes, so we get the value for each item of points-to set
//                        assert(isa<Instruction>(val)); //The above should guarantee we only get instructions
//                        Instruction *inst = cast<Instruction>(val);
//                        defNodeMap[inst].insert(&instNodeMap[&*I]);
//                    }


//                }
                
            }
        }
    }
}

void SDG::generatePointerEdges(Module &M)
{
    SDGNode *src, *dst;
    for (std::map<Instruction *, std::set<SDGNode *> >::iterator it = defNodeMap.begin(),
            et = defNodeMap.end(); it != et; ++it)
    {
        Instruction *defInst = it->first;
        for (std::set<SDGNode*>::iterator srcIt = it->second.begin(),
                srcE = it->second.end(); srcIt != srcE; ++srcIt)
        {
            defInst->dump();
            src = *srcIt;
            src->getValue()->dump();
            for (Value::use_iterator dstValIt = defInst->use_begin(),
                    dstValE = defInst->use_end(); dstValIt != dstValE; ++dstValIt)
            {
                if (LoadInst *dstInst = dyn_cast<LoadInst>(dstValIt->getUser())) //For every load, we need to have flow-dependence on every pointer that could have modified the value we're loading
                {
                    dst = &instNodeMap[dstInst];
                    //XXX: Memeory Leak
                    graph.insert(src, dst, new SDGEdge(flow));
                }
            }
        }
    }
}

void SDGNode::print(raw_ostream &OS) const
{
    OS << nodeAttrString[attr] << ":";
    if (value == NULL)
        OS << "NULL";
    else if (isa<Instruction>(value))
        OS << *value;
    else
        OS << value->getName();
    OS << "\n";
}

void SDGEdge::print(raw_ostream &OS) const
{
    OS << attr;
}

