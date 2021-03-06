/*  [print.cpp] Debug output for Anders.
 *  v. 0.16, 2008-11-27
 *------------------------------------------------------------------------------
 *  Copyright 2008 Andrey Petrov (see ../COPYING)
 */

#include "../include/anders.h"
#include "../include/printbuf.h"

namespace{

//Max length of the printed name of a single object, max length for a complete
//  value or node printout, and the width of the left column for some formats.
const u32 max_name_len= 1<<8, max_val_len= 1<<12, max_node_len= 1<<13,
    lhs_width= 40;

//If this was a member of Anders, none of the print methods could be const.
PrintBuf pb;

} //end anon namespace

//Let releaseMemory() free pb.
void Anders::_pb_free(){
  pb.free();
}

//Unless otherwise noted, print_foo() will only write into pb, while
//  print_foo_now() and print_all_foo() also output to stderr.

//------------------------------------------------------------------------------
//Print the name or description of value (V).
//(n) - the node ID that will be printed for some types (unless 0).
//(const_with_val) - print the values of constants (default true).
//(first) is used for recursion; do not set when calling from outside.
void Anders::print_val(Value *V, u32 n, bool const_with_val, bool first) const{
  assert(V);
  //Adding a single value should not extend the line past this size.
  static u32 val_end= 0;
  //Don't update the end size on recursive calls.
  if(first){
    val_end= pb.size() + max_val_len;
    //Not enough space in the buffer for the longest possible value.
    if(val_end > printbuf_sz){
      pb<<"<...>";
      return;
    }
  }

  //Print the parent function for an insn or arg.
  Instruction *I= dyn_cast<Instruction>(V);
  if(I){
    pb.nputs(max_name_len, I->getParent()->getParent()->getName());
    pb<<':';
  }else if(Argument *A= dyn_cast<Argument>(V)){
    pb.nputs(max_name_len, A->getParent()->getName());
    pb<<':';
  }

  if(V->hasName()){
    pb.nputs(max_name_len, V->getName());
  }else if(I && tmp_num.count(I)){
    //Unnamed temporary
    pb<<'%'<<tmp_num.find(I)->second;
  }else if(Constant *C= dyn_cast<Constant>(V)){
    _print_const(C, n, const_with_val, first);
  }else if(I){
    //Instruction without a name or number
    if(n && first)
      pb.printf("<insn#%u.", n);
    else
      pb<<"<insn.";
    pb<<I->getOpcodeName();
    pb<<'>';
  }else{
    //Unknown value
    if(n && first)
      pb.printf("<??\?#%u>", n);
    else
      pb<<"<??\?>";
  }
  if(first)
    pb.clip(val_end);
}
void Anders::print_val_now(Value *V, u32 n) const{
  pb.reset();
  print_val(V, n);
  PUTS(pb);
}

//------------------------------------------------------------------------------
//Print the description (optionally with value) for constant (C).
//  (for use by print_val() only)
void Anders::_print_const(Constant *C, u32 n, bool const_with_val, bool first)
    const{
  assert(C);
  if(n && first)
    pb.printf("<const#%u.", n);
  else
    pb<<"<const.";
  if(ConstantInt *K= dyn_cast<ConstantInt>(C)){
    if(const_with_val)
      pb.printf("int>(%lld)", K->getSExtValue());
    else
      pb<<"int>";
  }else if(isa<ConstantFP>(C))
    pb<<"FP>";
  else if(isa<ConstantAggregateZero>(C))
    pb<<"aggregate(0)>";
  else if(isa<ConstantVector>(C))
    pb<<"vector>";
  else if(isa<ConstantPointerNull>(C))
    pb<<"null-ptr>";
  else if(isa<UndefValue>(C))
    pb<<"undef>";
  else if(ConstantArray *K= dyn_cast<ConstantArray>(C)){
    //For arrays, print type and size (not contents).
    if(const_with_val){
      const char *tn= get_type_name(K->getType()->getElementType());
      u32 ne= K->getType()->getNumElements();
      pb<<"array>(";
      pb.nputs(max_name_len, tn);
      pb.printf("[%u])", ne);
    }else
      pb<<"array>";
  }else if(ConstantStruct *K= dyn_cast<ConstantStruct>(C)){
    //For structs, print type only.
    if(const_with_val){
      const char *tn= get_type_name(K->getType());
      pb<<"struct>(";
      pb.nputs(max_name_len, tn);
      pb<<')';
    }else
      pb<<"struct>";
  }else if(ConstantExpr *E= dyn_cast<ConstantExpr>(C)){
     pb<<"expr>";
     if(const_with_val)
       _print_const_expr(E);
  }else
    pb<<"??\?>";
}

//------------------------------------------------------------------------------
//Recursively print a constant expr. (for use by print_val() only)
void Anders::_print_const_expr(ConstantExpr *E) const{
  assert(E);
  pb<<'(';
  pb<<E->getOpcodeName();
  for(User::op_iterator it= E->op_begin(); it != E->op_end(); ++it){
    pb<<' ';
    //arg1: don't print the node # for the operands.
    //arg2: we can only get here if const_with_val is 1.
    //arg3: this is a recursive call.
    print_val(*it, 0, 1, 0);
  }
  pb<<')';
}

//------------------------------------------------------------------------------
//Print the node with ID (n), either the name or a description with (n).
//The printout is extended with dots on the right up to (width) (default 0).
void Anders::print_node(u32 n, u32 width) const{
  assert(n < nodes.size() && "node # out of range");
  assert(width < max_node_len && "width is too big");
  //End of the padding in the line.
  u32 pad_end= pb.size() + width;
  //Adding a single node should not extend the line past this size.
  u32 node_end= pb.size() + max_node_len;
  if(node_end > printbuf_sz){
    pb<<"<...>";
    return;
  }

  switch(n){
    //The 3 special IDs.
    case 0:
      pb<<"<none>";
      break;
    case i2p:
      pb<<"<i2p>";
      break;
    case p_i2p:
      pb<<"<p_i2p>";
      break;
    default:
      Value *V= nodes[n]->get_val();
      //Obj node for current value and its obj size (both 0 if V is null)
      u32 on= V ? get_obj_node(V, 1) : 0;
      u32 sz= nodes[on]->obj_sz;
      if(!V)
        pb.printf("<artificial#%u>", n);
      else if(Function *F= dyn_cast<Function>(V)){
        //A function-valued node may be a return value, vararg, the pointer
        //  referred to by the function's name, or its object's first node.
        pb.nputs(max_name_len, F->getName());
        if(n == get_ret_node(F))
          pb<<"<retval>";
        else if(n == get_vararg_node(F))
          pb<<"<vararg>";
        else if(n == get_val_node(F))
          pb<<"<fptr>";
        else if(n == on)
          pb<<"/0";
        else
          pb<<"<??\?>";
      }else{
        //Print non-function values with the number but without
        //  the values of constants.
        print_val(V, n, 0);
        //If this is an object node, mark it with the offset
        //  from the object's start.
        if(n-on < sz)
          pb.printf("/%u", n-on);
      }
  }
  pb.pad('.', pad_end);
  pb.clip(node_end);
}
void Anders::print_node_now(u32 n) const{
  pb.reset();
  print_node(n);
  PUTS(pb);
}

//------------------------------------------------------------------------------
void Anders::print_all_nodes(bool sorted) const{
  PUTS("==========  Node list  =====================================\n");
  vector<string> lines;
  FORN(i, nodes.size()){
    pb.reset();
    print_node(i, lhs_width);
    const Node *N= nodes[i];
    pb.printf(" #%u, sz= %u\n", i, N->obj_sz);
    if(sorted)
      lines.push_back(pb);
    else
      PUTS(pb);
  }
  if(sorted){
    sort(lines.begin(), lines.end());
    FORN(i, lines.size())
      PUTS(lines[i].c_str());
  }
}

//------------------------------------------------------------------------------
//Print the name of type (T) and its struct_info vectors to stderr.
void Anders::print_struct_info(const Type *T) const{
  assert(T);
  const StructType *ST= dyn_cast<StructType>(T);
  if(!ST){
    PUTS("!! print_struct_info called with non-struct type\n");
    return;
  }
  pb.reset();
  pb<<"--- ";
  pb.nputs(max_name_len, get_type_name(ST));
  pb<<" ---\nsz=";
  //We don't use get_struct_info() here because it may call _analyze_struct.
  structinfo_t::const_iterator it= struct_info.find(ST);
  assert(it != struct_info.end() && "struct_info is incomplete");
  const pair<vector<u32>, vector<u32> > &info= it->second;
  const vector<u32> &sz= info.first, &off= info.second;
  FORN(i, sz.size())
    pb.printf(" %u", sz[i]);
  pb<<"\noff=";
  FORN(i, off.size())
    pb.printf(" %u", off[i]);
  pb<<'\n';
  PUTS(pb);
}

//------------------------------------------------------------------------------
void Anders::print_all_structs() const{
  PUTS("==========  Struct info  ===================================\n");
  for(structinfo_t::const_iterator it= struct_info.begin(),
      ie= struct_info.end(); it != ie; ++it)
    print_struct_info(it->first);
}

//------------------------------------------------------------------------------
void Anders::print_constraint(const Constraint &C) const{
  if(C.type == store_cons)
    pb<<'*';
  pb<<'(';
  print_node(C.dest);
  pb<<')';
  if(C.type == store_cons && C.off)
    pb.printf(" + %u", C.off);
  pb<<" = ";
  if(C.type == addr_of_cons)
    pb<<'&';
  else if(C.type == load_cons)
    pb<<'*';
  pb<<'(';
  print_node(C.src);
  pb<<')';
  if(C.type != store_cons && C.off)
    pb.printf(" + %u", C.off);
}
void Anders::print_constraint_now(const Constraint &C) const{
  pb.reset();
  print_constraint(C);
  PUTS(pb);
}

//------------------------------------------------------------------------------
void Anders::print_all_constraints(bool sorted) const{
  PUTS("==========  Constraint list  ===============================\n");
  vector<string> lines;
  FORN(i, constraints.size()){
    const Constraint &C= constraints[i];
    pb.reset();
    print_constraint(C);
    pb<<'\n';
    if(sorted)
      lines.push_back(pb);
    else
      PUTS(pb);
  }
  if(sorted){
    sort(lines.begin(), lines.end());
    FORN(i, lines.size())
      PUTS(lines[i].c_str());
  }
}

//------------------------------------------------------------------------------
//Print all nodes with their points-to sets. If (points_to_only) is false
//  (default true), the copy edges and complex constraints are also printed.
//If (O) is not null, the output goes there instead of stderr.
void Anders::print_cons_graph(bool points_to_only, bool sorted,
    std::ostream *O) const{
  const char *header=
      "==========  Constraint graph ==============================\n";
  if(points_to_only)
    header= "==========  Points-to graph  ===============================\n";
  if(O)
    (*O)<<header;
  else
    PUTS(header);
  vector<string> lines;
  FORN(i, nodes.size()){
    //Non-rep nodes should be included in the final solution but not in
    //  the constraint graph.
    if(!points_to_only && !nodes[i]->is_rep())
      continue;
    pb.reset();
    print_node(i, lhs_width);
    pb<<"  ->";
    //A non-rep node has the same points-to as its rep.
    const Node *N= nodes[cget_node_rep(i)];
    const vector<u32> *pts= bdd2vec(N->points_to);
    FORN(i, pts->size()){
      pb<<"  ";
      print_node((*pts)[i]);
    }
    if(!points_to_only){
      if(!N->copy_to.empty()){
        pb<<"\n    COPY:";
        for(bitmap::iterator it= N->copy_to.begin(), ie= N->copy_to.end();
            it != ie; ++it){
          pb<<"  ";
          print_node(*it);
        }
      }
      if(!N->load_to.empty()){
        pb<<"\n    LOAD:";
        _print_node_cons(N->load_to);
      }
      if(!N->store_from.empty()){
        pb<<"\n    STORE:";
        _print_node_cons(N->store_from);
      }
      if(!N->gep_to.empty()){
        pb<<"\n    GEP:";
        _print_node_cons(N->gep_to);
      }
    }
    pb<<'\n';
    if(sorted)
      lines.push_back(pb);
    else if(O)
      (*O)<<pb;
    else
      PUTS(pb);
  }
  if(sorted){
    sort(lines.begin(), lines.end());
    FORN(i, lines.size()){
      if(O)
        (*O)<<lines[i];
      else
        PUTS(lines[i].c_str());
    }
  }
}

//------------------------------------------------------------------------------
//Print the complex constraints referenced by bitmap (L).
//  (for use by print_cons_graph() only)
void Anders::_print_node_cons(const bitmap &L) const{
  for(bitmap::iterator it= L.begin(), ie= L.end(); it != ie; ++it){
    pb<<"  ";
    const Constraint &C= cplx_cons[*it];
    //The node not printed here is the one that contains (L).
    switch(C.type){
      case load_cons:
        print_node(C.dest);
        break;
      case store_cons:
        print_node(C.src);
        break;
      case gep_cons:
        print_node(C.dest);
        break;
      default:
        assert(!"not a complex constraint");
    }
    if(C.off)
      pb.printf(" +%u", C.off);
  }
}

//------------------------------------------------------------------------------
//Print the descriptions and values of all defined stats.
void Anders::print_stats() const{
  PUTS("==========  Statistics  ====================================\n");
  FORN(i, num_stats){
    pb.reset();
    pb<<stat_desc[i];
    pb.pad('.', lhs_width);
    pb.printf(" %u\n", stats[i]);
    PUTS(pb);
  }
}

//------------------------------------------------------------------------------
//Print various metrics of the points-to solution.
void Anders::print_metrics() const{
  u32 nn= nodes.size();
  //Points-to size and density.
  //_uniq doesn't count the same points_to set more than once;
  //  the difference is a measure of remaining pointer equivalence.
  u32 n_pts= 0, n_pts_uniq= 0;
  u64 sum_pts= 0, sum_pts_uniq= 0;
  hash_set<u32> pts_seen;
  FORN(i, nn){
    bdd pts= nodes[i]->points_to;
    u32 sz= (u32)bdd_satcountset(pts, pts_dom);
    if(!sz)
      continue;
    ++n_pts;
    sum_pts += sz;
    if(!pts_seen.count(pts.id())){
      pts_seen.insert(pts.id());
      ++n_pts_uniq;
      sum_pts_uniq += sz;
    }
  }
  pts_seen.clear();
  double avg_pts= n_pts ? sum_pts/(double)n_pts : 0,
      avg_pts_uniq= n_pts_uniq ? sum_pts_uniq/(double)n_pts_uniq : 0;
  fprintf(stderr, "Points-to edges: %llu in %u sets, avg %0.3f\n", sum_pts,
      n_pts, avg_pts);
  fprintf(stderr, "- unique points-to: %llu in %u, avg %0.3f\n", sum_pts_uniq,
      n_pts_uniq, avg_pts_uniq);
}

//------------------------------------------------------------------------------
//Print a sorted list of pointer-relevant external functions (i.e. those which
//  return pointers or take them as args) not listed in extinfo.cpp.
void Anders::list_ext_unknown(Module &M) const{
  assert(extinfo);
  vector<string> names;
  for(Module::iterator it= M.begin(), ie= M.end(); it != ie; ++it){
    if(it->isDeclaration() || it->isIntrinsic()){
      bool rel= 0;
      if(isa<PointerType>(it->getReturnType())){
        rel= 1;
      }else for(Function::arg_iterator j= it->arg_begin(), je= it->arg_end();
          j != je; ++j){
        if(isa<PointerType>(j->getType())){
          rel= 1;
          break;
        }
      }
      if(rel && extinfo->get_type(it) == EFT_OTHER){
        names.push_back(it->getName().str());
      }
    }
  }

  sort(names.begin(), names.end());
  if(names.size())
    PUTS("!! Unknown ext. calls:\n");
  FORN(i, names.size()){
    PUTS(names[i].c_str());
    putc('\n', stderr);
  }
}

//------------------------------------------------------------------------------
//Statistic descriptions (in the same order as the enum in anders.h)
const char *const (Anders::stat_desc[])= {
  "Initial value nodes", "- object nodes", "LLVM instructions",
  "Initial constraints", "- addr_of", "- copy", "- load", "- store", "- GEP",
  "Remaining (rep) value nodes", "Reduced constraints",
  "- addr_of", "- copy", "- load", "- store", "- GEP",
  "Nodes merged in HVN", "HCD map entries", "- VARs merged offline",
  "- VARs merged online", "- SCCs detected online", "  - nodes in these",
  "Loads/stores removed by factoring",
  "Solver passes", "- nodes pushed", "- nodes popped", "- solve_node runs",
  "- copy edges added", "- copy edges deleted",
  "- complex constraints deleted", "- objects allocated by indirect calls",
  "- LCD runs", "  - SCCs detected",  "  - nodes in these"
};
