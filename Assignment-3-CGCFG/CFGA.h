/**
 * ICFG.h
 * @author kisslune 
 */

 #ifndef ANSWERS_ICFG_H
 #define ANSWERS_ICFG_H
 
 #include "Graphs/SVFG.h"
 #include "SVF-LLVM/SVFIRBuilder.h"
 
 class CFGAnalysis
 {
 public:
     explicit CFGAnalysis(SVF::ICFG *icfg);
     void analyze(SVF::ICFG *icfg);
     void dumpPaths();
 
 protected:
     void recordPath(const std::vector<unsigned> &path);
 
     std::stack<unsigned> callStack;
     std::set<unsigned> sources;
     std::set<unsigned> sinks;
     std::set<std::vector<unsigned>> reachablePaths;
     void dfs(unsigned src, unsigned snk, SVF::ICFG *icfg,std::set<unsigned> &visited,std::vector<unsigned> &path);
 };
 
 #endif //ANSWERS_ICFG_H