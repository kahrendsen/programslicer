#ifndef DIRECTED_GRAPH_H
#define DIRECTED_GRAPH_H

#include <iostream>
#include <map>
#include <set>

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"

namespace llvm {
    template <typename NodeT, typename EdgeT>
    class DirectedGraph
    {
    public:
        typedef std::set<NodeT*> nodeSet_t;
        typedef std::pair<NodeT*, EdgeT*> nodePair_t;
        typedef std::map<NodeT*, EdgeT*> nodeMap_t;
        typedef std::map<NodeT*, nodeMap_t> edgeMap_t;

        std::string name;

        DirectedGraph(std::string name = "") : name(name) {}
        void insert(NodeT* A, NodeT* B, EdgeT* edge = NULL)
        {
            succMap[A][B] = edge;
            predMap[B][A] = edge;
            nodeSet.insert(A);
            nodeSet.insert(B);
        }

        void erase(NodeT* A, NodeT* B)
        {
            succMap[A].erase(B);
            predMap[B].erase(A);
            // TODO: erase node set entry
        }

        nodeMap_t& getSuccSet(NodeT* A)
        {
            return succMap[A];
        }

        nodeMap_t& getPredSet(NodeT* A)
        {
            return predMap[A];
        }

        nodeSet_t& getNodeSet()
        {
            return nodeSet;
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
            O << "digraph " << name << "{\n";
            typename edgeMap_t::const_iterator it, e;
            typename nodeMap_t::const_iterator jt, et;
            for (it = succMap.begin(), e = succMap.end(); it != e; ++it)
            {
                for (jt = it->second.begin(), et = it->second.end(); jt != et; ++jt)
                {
                    O << "\t" << "\"";
                    it->first==NULL ? O << "entry" : O << *it->first; 
                    O << "\"" << " -> " << "\"";
                    jt->first==NULL ? O << "entry" : O << *jt->first; 
                    O << "\"";
                    if (jt->second != NULL)
                    {
                        O << " [ label = \"" << *jt->second << "\" ]";
                    }
                    O << ";\n";
                }
            }
            O << "}\n";
        }

    private:
        edgeMap_t succMap;
        edgeMap_t predMap;
        nodeSet_t nodeSet;
    };

    template<typename NodeT, typename EdgeT>
    inline raw_ostream &operator <<(raw_ostream &OS, const DirectedGraph<NodeT, EdgeT> &G)
    {
        G.print(OS);
        return OS;
    }
}

#endif // DIRECTED_GRAPH_H

