// Author Hao Wu and Ying-yu Chen

#include "llvm/Support/InstIterator.h"

#include "SDG.h"

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

    errs() << "pts: begin\n";
    //    pts.runOnModule(M);
    errs() << "pts: end\n";

    // pts.getPtsSet(Value* V, vector<Value* >& set);

    // Step 1: INTRAprocedure Analysis - CDG
    errs() << "create CDG\n";
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
        errs() << cdg.getBBGraph();

        std::set<BasicBlock *> &nodeSet = _bbGraph.getNodeSet();
        for (std::set<BasicBlock *>::iterator fIt = nodeSet.begin(),
                e = nodeSet.end(); fIt != e; ++fIt)
        {
            BasicBlock *D = *fIt;
            if (D == NULL)
            {
                continue;
            }
            errs() << "D addr = " << D << "\n";
            Function *F = D->getParent();
            //errs() << "F=" << *F <<"\n";
            CDG::_bbGraph_t::nodeMap_t &predSet = _bbGraph.getPredSet(D);
            for (CDG::_bbGraph_t::nodeMap_t::const_iterator it = predSet.begin(),
                    et = predSet.end(); it != et; ++it)
            {
                BasicBlock *S = it->first;
                errs() << "S addr = " << S << "\n";
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
    }

    errs() << "create INTER\n";
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
                for (unsigned i = 0; i < numArgs; ++i)
                {
                    Function::arg_iterator arg_it = F.arg_begin();
                    Argument *calleeArg = &*arg_it;
                    // The callee should have at least the same number of
                    // argument as the caller, otherwise the function call is
                    // invalid.
                    //assert(arg_it != arg_end);
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
                callerInputMap.insert(std::pair<Value *, SDGNode>(
                            CI,
                            SDGNode(callerAux, CI)));
                src = &callerOutputMap[CI];
                dst = &calleeOutputNodeMap[&F];
                // XXX: Memory leak
                graph.insert(src, dst, new SDGEdge(paramOut));
            }
        }
    }

    errs() << "create DDG\n";
    // Step 3: Create DDG without pointer analysis
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        Function &F = *it;
        generateIntraDDG(F);
    }

    errs() << graph << "\n";
    return false;
}

bool SDG::generateIntraDDG(Function &F)
{
    // TODO: Consider pointers
    SDGNode *src, *dst;
    // Use du chain in LLVM
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    {
        errs() << *I << "\n";
#if 0
        for (Value::use_iterator i = I->use_begin(), e = I->use_end(); i != e; ++i)
        {
            if (Instruction *Inst = dyn_cast<Instruction>(*i)) {
                // Assert that I has been inserted during CDG construction
                assert(instNodeMap.find(&*I) != instNodeMap.end());
                assert(instNodeMap.find(&*Inst) != instNodeMap.end());
                src = &instNodeMap[&*I];
                dst = &instNodeMap[Inst];
                //XXX: Memory leak
                graph.insert(src, dst, new SDGEdge(flow));
            }
        }
#endif
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
    }
    return false;
}

void SDGNode::print(raw_ostream &OS) const
{
    OS << nodeAttrString[attr] << ":";
    if (value == NULL)
        OS << "NULL";
    else
        OS << *value;
    OS << "\n";
}

void SDGEdge::print(raw_ostream &OS) const
{
    OS << attr;
}

