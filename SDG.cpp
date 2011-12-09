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

    // Step 1.2: Create DDG
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        Function &F = *it;
        generateIntraDDG(F);
    }

    // Step 2: INTERprocedure Analysis
    for (Module::iterator it = M.begin(), e = M.end(); it != e; ++it)
    {
        Function &F = *it;
        // Add Callee aux nodes
        for (Function::arg_iterator arg_it = F.arg_begin(), arg_e = F.arg_end();
                arg_it != arg_e; ++arg_it)
        {
            Argument *arg = &*arg_it;
            calleeInputNodeMap.insert(std::pair<Argument*, SDGNode>(
                        arg, SDGNode(calleeAux, arg)));
        }

        // Add caller aux nodes and relationships
        for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It)
        {
            Instruction *I = &*It;
            if (CallInst *CI = dyn_cast<CallInst>(I))
            {
                std::map<Value *, SDGNode> &callerMap = callerInputNodeMap[CI];
                Function *CF = CI->getCalledFunction();
//                Function::ArgumentListType &CFAL = CF->getArgumentList();
                // Step 2.1: Add call edges
                src = &instNodeMap[CI];
                dst = &entryNodeMap[CF];
                assert(src.getValue());
                assert(dst.getValue());
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
                    assert(arg_it != arg_end);
                    // Add aux nodes for callee input
                    Value *callerArg = CI->getArgOperand(i);
                    callerMap.insert(std::pair<Value *, SDGNode>(
                                callerArg,
                                SDGNode(callerAux, callerArg)));
                    // Add aux edges for callee and caller input
                    src = &callerMap[callerArg];
                    dst = &calleeInputNodeMap[calleeArg];
                    // XXX: Memory leak
                    graph.insert(src, dst, new SDGEdge(paramIn));
                    // TODO: Use pointer analysis to determine output aux nodes
                }
                // TODO: Add output aux nodes
                // Step 2.3: TODO: Add relationship between aux nodes
            }
        }
    }
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
                //XXX: Memory leak
                graph.insert(src, dst, new SDGEdge(flow));
            }
        }
    }
    return false;
}
