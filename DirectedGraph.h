#ifndef DIRECTED_GRAPH_H
#define DIRECTED_GRAPH_H

#include <iostream>
#include <map>
#include <set>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"

namespace llvm {
    template <class NodeT>
    class DirectedGraph
    {
    public:
        typedef std::set<NodeT*> nodeSet_t;
        typedef std::map<NodeT*, nodeSet_t> edgeMap_t;

        DirectedGraph() {}
        void insert(NodeT* A, NodeT* B)
        {
            succMap[A].insert(B);
            predMap[B].insert(A);
        }

        void erase(NodeT* A, NodeT* B)
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

        /// print - Implement operator << on DirectedGraph
        ///
        void print(raw_ostream &O) const
        {
            O << "---------------------------------------\n";
            O << "SUCC MAP:\n";
            typename edgeMap_t::const_iterator it, e;
            typename nodeSet_t::const_iterator jt, et;
            for (it = succMap.begin(), e = succMap.end(); it != e; ++it)
            {
                for (jt = it->second.begin(), et = it->second.end(); jt != et; ++jt)
                {
                    O << "(" << it->first << ", " << *jt << ")\n";
                }
            }
            O << "PRED MAP:\n";
            for (it = predMap.begin(), e = predMap.end(); it != e; ++it)
            {
                for (jt = it->second.begin(), et = it->second.end(); jt != et; ++jt)
                {
                    O << "(" << it->first << ", " << *jt << ")\n";
                }
            }
            O << "---------------------------------------\n";
        }

    private:
        edgeMap_t succMap;
        edgeMap_t predMap;
    };

    template<typename NodeT>
    inline raw_ostream &operator <<(raw_ostream &OS, const DirectedGraph<NodeT> &G)
    {
        G.print(OS);
        return OS;
    }
}

#endif // DIRECTED_GRAPH_H

