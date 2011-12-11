
#include "PtsAnders.h"

void 
PtsAnders::getPtsSet(Value* V, vector<Value* >& set)
{
  u32 n = val_node[V];
  
  //If node #i was merged, print the edges and constraints of the rep node.
  const Node *N= nodes[cget_node_rep(n)];
  const vector<u32> *pts= bdd2vec(N->points_to);
  FORN(i, pts->size()){
    set.push_back(nodes[(*pts)[i]]->get_val());
  }
}

void 
PtsAnders::printVal(Value* V)
{
  assert(V);

  //Print the parent function for an insn or arg.
  Instruction *I= dyn_cast<Instruction>(V);
  if(I){
    errs() << "Instr:" << I->getParent()->getParent()->getName();
  }else if(Argument *A= dyn_cast<Argument>(V)){
    errs() << "Arg: " << A->getParent()->getName();
  }
  errs() << " :: ";

  if(V->hasName()){
    errs() << "hn: " << V->getName();
  }else if(I && tmp_num.count(I)){
    errs()<<"tmp%"<<tmp_num.find(I)->second;
  }else if(Constant *C= dyn_cast<Constant>(V)){
    errs() << "const: " << *C;
  }else if(I){
  errs() << "If(I)";
  /*
    if(n && first)
      pb.printf("<insn#%u.", n);
    else
      pb<<"<insn.";
    pb<<I->getOpcodeName();
    pb<<'>';
  */
  }else{
  errs() << "Else";
  /*
    if(n && first)
      pb.printf("<??\?#%u>", n);
    else
      pb<<"<??\?>";
      */
  }
}

