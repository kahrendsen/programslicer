#ifndef DIRECTED_GRAPH_H
#define DIRECTED_GRAPH_H

#include <utility>
#include <iostream>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"

namespace llvm {

template <class NodeT>
class DirectedGraph
{
public:
  //typedef std::set<NodeT*>::const_iterator const_node_it;

  DirectedGraph() {}
  Insert(NodeT* A, NodeT* B)
  {
    succMap[A].insert(B);
    predMap[B].insert(A);
  }
  
  Erase(NodeT* A, NodeT* B)
  {
    succMap[A].erase(B);
    predMap[B].erase(A);
  }
  
  const nodeSet_t& getSuccSet(NodeT* A)
  {
    return succMap[A];
  }
  
  const nodeSet_t& getPredSet(NodeT* A)
  {
    return predMap[A];
  }

private:
  typedef std::set<NodeT*> nodeSet_t;
  typedef std::map<NodeT*, nodeSet_t> edgeMap_t;
  
  edgeMap_t succMap;
  edgeMap_t predMap;
};

}

#endif
