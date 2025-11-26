/**
 * ICFG.cpp
 * @author kisslune 
 */

 #include "CFGA.h"

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
     auto icfg = pag->getICFG();
 
     CFGAnalysis analyzer = CFGAnalysis(icfg);
 
     // TODO: complete the following method: 'CFGAnalysis::analyze'
     analyzer.analyze(icfg);
     analyzer.dumpPaths();
     LLVMModuleSet::releaseLLVMModuleSet();
     return 0;
 }
 
 
 void CFGAnalysis::analyze(SVF::ICFG *icfg)
 {
     // Sources and sinks are specified when an analyzer is instantiated.
     for (auto src : sources)
         for (auto snk : sinks)
         {
             // TODO: DFS the graph, starting from src and detecting the paths ending at snk.
             // Use the class method 'recordPath' (already defined) to record the path you detected.
             //@{
             std::set<unsigned> visited;
             std::vector<unsigned> path;
             dfs(src,snk,icfg,visited,path);
             //@}
         }
 }
 
 void CFGAnalysis::dfs(unsigned src, unsigned snk, SVF::ICFG *icfg,
                       std::set<unsigned> &visited, std::vector<unsigned> &path) {
     if (!visited.insert(src).second) {
         return; 
     }
 
     path.push_back(src);
 
     if (src == snk) {
         recordPath(path);
     } else {
         const auto *node = icfg->getICFGNode(src);
         const auto &outEdges = node->getOutEdges();
 
         for (const auto *e : outEdges) {
             const auto *dstNode = e->getDstNode();
             const unsigned id = dstNode->getId();
 
             if (e->isIntraCFGEdge()) {
                 dfs(id, snk, icfg, visited, path);
             } 
             else if (e->isCallCFGEdge()) {
                 const auto *callEdge = llvm::dyn_cast<CallCFGEdge>(e);
                 const unsigned callId = callEdge->getCallSite()->getId();
 
                 callStack.push(callId);
                 dfs(id, snk, icfg, visited, path);
                 callStack.pop();
             } 
             else if (e->isRetCFGEdge()) {
                 const auto *retEdge = llvm::dyn_cast<RetCFGEdge>(e);
                 const unsigned callId = retEdge->getCallSite()->getId();
 
                 if (callStack.empty()) {
                     dfs(id, snk, icfg, visited, path);
                 } 
                 else if (callStack.top() == callId) {
                     callStack.pop();
                     dfs(id, snk, icfg, visited, path);
                     callStack.push(callId);
                 }
             }
         }
     }
 
     visited.erase(src);
     path.pop_back();
 }