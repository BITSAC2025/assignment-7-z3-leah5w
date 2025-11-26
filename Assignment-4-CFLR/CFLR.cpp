/**
 * CFLR.cpp
 * @author kisslune 
 */

 #include "A4Header.h"

 using namespace SVF;
 using namespace llvm;
 using namespace std;
 
 int main(int argc, char **argv)
 {
     auto moduleNameVec =
             OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                      "[options] <input-bitcode...>");
 
     LLVMModuleSet::buildSVFModule(moduleNameVec);
 
     SVFIRBuilder builder;
     auto pag = builder.build();
     pag->dump();
 
     CFLR solver;
     solver.buildGraph(pag);
     // TODO: complete this method
     solver.solve();
     solver.dumpResult();
 
     LLVMModuleSet::releaseLLVMModuleSet();
     return 0;
 }
 
 void CFLR::initSolver()
 {
     std::set<unsigned> allNodes;
     auto &succMap = graph->getSuccessorMap();
     auto &predMap = graph->getPredecessorMap();
 
     for (auto &nodeItr : succMap)
     {
         unsigned src = nodeItr.first;
         allNodes.insert(src);
         for (auto &lblItr : nodeItr.second)
         {
             for (auto dst : lblItr.second)
                 allNodes.insert(dst);
         }
     }
     for (auto &nodeItr : predMap)
     {
         unsigned dst = nodeItr.first;
         allNodes.insert(dst);
         for (auto &lblItr : nodeItr.second)
         {
             for (auto src : lblItr.second)
                 allNodes.insert(src);
         }
     }
 
     const std::vector<EdgeLabel> emptyRuleSymbols = {VF, VFBar, VA};
 
     for (auto node : allNodes)
     {
         for (EdgeLabel sym : emptyRuleSymbols)
         {
             if (!graph->hasEdge(node, node, sym))
             {
                 graph->addEdge(node, node, sym);
                 workList.push(CFLREdge(node, node, sym));
             }
         }
     }
 
     for (auto &nodeItr : succMap)
     {
         unsigned src = nodeItr.first;
         for (auto &lblItr : nodeItr.second)
         {
             EdgeLabel label = lblItr.first;
             for (auto dst : lblItr.second)
             {
                 workList.push(CFLREdge(src, dst, label));
             }
         }
     }
 }
 
 std::unordered_set<EdgeLabel> CFLR::unarySumm(EdgeLabel lbl)
 {
     std::unordered_set<EdgeLabel> ret;
     if (lbl == Copy) ret.insert(VF);
     if (lbl == CopyBar) ret.insert(VFBar);
     return ret;
 }
 std::unordered_set<EdgeLabel> CFLR::binarySumm(EdgeLabel left, EdgeLabel right)
 {
     std::unordered_set<EdgeLabel> ret;
     if (left == VFBar && right == AddrBar) ret.insert(PT);
     if (left == Addr && right == VF) ret.insert(PTBar);
     if (left == VF && right == VF) ret.insert(VF);
     if (left == SV && right == Load) ret.insert(VF);
     if (left == PV && right == Load) ret.insert(VF);
     if (left == Store && right == VP) ret.insert(VF);
     if (left == VFBar && right == VFBar) ret.insert(VFBar);
     if (left == LoadBar && right == SVBar) ret.insert(VFBar);
     if (left == LoadBar && right == VP) ret.insert(VFBar);
     if (left == PV && right == StoreBar) ret.insert(VFBar);
     if (left == LV && right == Load) ret.insert(VA);
     if (left == VFBar && right == VA) ret.insert(VA);
     if (left == VA && right == VF) ret.insert(VA);
     if (left == Store && right == VA) ret.insert(SV);
     if (left == VA && right == StoreBar) ret.insert(SVBar);
     if (left == PTBar && right == VA) ret.insert(PV);
     if (left == VA && right == PT) ret.insert(VP);
     if (left == LoadBar && right == VA) ret.insert(LV);
     return ret;
 }
 
 inline void CFLR::addNewEdgeToGraphAndWorklist(unsigned src, unsigned dst, EdgeLabel lbl)
 {
     if (!graph->hasEdge(src, dst, lbl))
     {
         graph->addEdge(src, dst, lbl);
         workList.push(CFLREdge(src, dst, lbl));
     }
 }
 
 
 void CFLR::solve()
 {
     // TODO: complete this function. The implementations of graph and worklist are provided.
     //  You need to:
     //  1. implement the grammar production rules into code;
     //  2. implement the dynamic-programming CFL-reachability algorithm.
     //  You may need to add your new methods to 'CFLRGraph' and 'CFLR'.
     initSolver();
         while (!workList.empty())
     {
         CFLREdge edge = workList.pop();
         unsigned u = edge.src;
         unsigned v = edge.dst;
         EdgeLabel lab = edge.label;
 
         {
             auto newLabels = unarySumm(lab);
             for (EdgeLabel nl : newLabels)
                 addNewEdgeToGraphAndWorklist(u, v, nl);
         }
 
         {
             auto &succMap = graph->getSuccessorMap();
             auto it = succMap.find(v);
             if (it != succMap.end())
             {
                 for (auto &lblSuccs : it->second)
                 {
                     EdgeLabel succLabel = lblSuccs.first;
                     const std::unordered_set<unsigned> &succNodes = lblSuccs.second;
                     auto prod = binarySumm(lab, succLabel);
                     if (prod.empty()) continue;
                     for (unsigned w : succNodes)
                     {
                         for (EdgeLabel newLbl : prod)
                             addNewEdgeToGraphAndWorklist(u, w, newLbl);
                     }
                 }
             }
         }
 
         {
             auto &predMap = graph->getPredecessorMap();
             auto itp = predMap.find(u);
             if (itp != predMap.end())
             {
                 for (auto &lblPreds : itp->second)
                 {
                     EdgeLabel predLabel = lblPreds.first;
                     const std::unordered_set<unsigned> &predNodes = lblPreds.second;
                     auto prod = binarySumm(predLabel, lab);
                     if (prod.empty()) continue;
                     for (unsigned p : predNodes)
                     {
                         for (EdgeLabel newLbl : prod)
                             addNewEdgeToGraphAndWorklist(p, v, newLbl);
                     }
                 }
             }
         }
     }
 }
 