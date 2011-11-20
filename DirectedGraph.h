#ifndef DIRECTED_GRAPH_H
#define DIRECTED_GRAPH_H

#include <iostream>
#include <map>
#include <set>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"

namespace llvm {
    template <typename NodeT, typename EdgeT>
    class DirectedGraph
    {
    public:
        typedef std::pair<NodeT*, EdgeT*> nodePair_t;
        typedef std::map<NodeT*, EdgeT*> nodeMap_t;
        typedef std::map<NodeT*, nodeMap_t> edgeMap_t;

        DirectedGraph() {}
        void insert(NodeT* A, NodeT* B, EdgeT* edge = NULL)
        {
            succMap[A][B] = edge;
            predMap[B][A] = edge;
        }

        void erase(NodeT* A, NodeT* B)
        {
            succMap[A].erase(B);
            predMap[B].erase(A);
        }

        const nodeMap_t& getSuccSet(NodeT* A)
        {
            return succMap[A];
        }

        const nodeMap_t& getPredSet(NodeT* A)
        {
            return predMap[A];
        }

        /// print - Implement operator << on DirectedGraph
        ///
        void print(raw_ostream &O) const
        {
            /*
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
            */
            printDot(O);
        }

        /// printDot - print out the graph as a dot graph.
        ///
        void printDot(raw_ostream &O) const
        {
            // TODO
        }

    private:
        edgeMap_t succMap;
        edgeMap_t predMap;
    };

    template<typename NodeT, typename EdgeT>
    inline raw_ostream &operator <<(raw_ostream &OS, const DirectedGraph<NodeT, EdgeT> &G)
    {
        G.print(OS);
        return OS;
    }
}

#endif // DIRECTED_GRAPH_H

