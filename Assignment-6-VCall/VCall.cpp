/**
 * VCall.cpp
 * @author kisslune
 */

#include "A6Header.h"

using namespace llvm;
using namespace std;

int main(int argc, char** argv)
{
    auto moduleNameVec =
            OptionBase::parseOptions(argc, argv, "Whole Program Points-to Analysis",
                                     "[options] <input-bitcode...>");

    SVF::LLVMModuleSet::buildSVFModule(moduleNameVec);

    SVF::SVFIRBuilder builder;
    auto pag = builder.build();
    auto consg = new SVF::ConstraintGraph(pag);
    consg->dump();

    Andersen andersen(consg);
    auto cg = pag->getCallGraph();

    // TODO: complete the following two methods
    andersen.runPointerAnalysis();
    andersen.updateCallGraph(cg);

    cg->dump();
    SVF::LLVMModuleSet::releaseLLVMModuleSet();
	return 0;
}

void Andersen::runPointerAnalysis()
{
    WorkList<unsigned> workList;

    for (auto& entry : *consg) {
        auto dstId = entry.first;
        auto* node = entry.second;

        for (auto edge : node->getAddrInEdges()) {
            auto* addrEdge = SVF::SVFUtil::dyn_cast<SVF::AddrCGEdge>(edge);
            auto srcId = addrEdge->getSrcID();

            if (pts[dstId].insert(srcId).second)
                workList.push(dstId);
        }
    }

    while (!workList.empty()) {
        unsigned p = workList.pop();
        auto* pNode = consg->getConstraintNode(p);
        auto& ptsP = pts[p];

        for (auto o : ptsP) {
            for (auto edge : pNode->getStoreInEdges()) {
                auto* storeEdge = SVF::SVFUtil::dyn_cast<SVF::StoreCGEdge>(edge);
                auto q = storeEdge->getSrcID();
                auto* qNode = consg->getConstraintNode(q);

                bool exist = false;
                for (auto copyEdge : qNode->getCopyOutEdges()) {
                    if (copyEdge->getDstID() == o) {
                        exist = true;
                        break;
                    }
                }

                if (!exist) {
                    consg->addCopyCGEdge(q, o);
                    workList.push(q);
                }
            }

            for (auto edge : pNode->getLoadOutEdges()) {
                auto* loadEdge = SVF::SVFUtil::dyn_cast<SVF::LoadCGEdge>(edge);
                auto r = loadEdge->getDstID();
                auto* rNode = consg->getConstraintNode(r);

                bool exist = false;
                for (auto copyEdge : rNode->getCopyInEdges()) {
                    if (copyEdge->getSrcID() == o) {
                        exist = true;
                        break;
                    }
                }

                if (!exist) {
                    consg->addCopyCGEdge(o, r);
                    workList.push(o);
                }
            }
        }

        for (auto edge : pNode->getCopyOutEdges()) {
            auto* copyEdge = SVF::SVFUtil::dyn_cast<SVF::CopyCGEdge>(edge);
            auto x = copyEdge->getDstID();
            auto& ptsX = pts[x];

            auto oldSize = ptsX.size();
            ptsX.insert(ptsP.begin(), ptsP.end());

            if (ptsX.size() != oldSize)
                workList.push(x);
        }

        for (auto edge : pNode->getGepOutEdges()) {
            auto* gepEdge = SVF::SVFUtil::dyn_cast<SVF::GepCGEdge>(edge);
            auto x = gepEdge->getDstID();
            auto& ptsX = pts[x];

            auto oldSize = ptsX.size();
            for (auto o : ptsP)
                ptsX.insert(consg->getGepObjVar(o, gepEdge));

            if (ptsX.size() != oldSize)
                workList.push(x);
        }
    }
}

void Andersen::updateCallGraph(SVF::CallGraph* cg)
{
    for (const auto& [callNode, funPtrId] : consg->getIndirectCallsites()) {
        auto callerFun = callNode->getCaller();
        const auto& ptsSet = pts[funPtrId];

        for (auto calleeId : ptsSet) {
            if (consg->isFunction(calleeId))
                cg->addIndirectCallGraphEdge(callNode, callerFun, consg->getFunction(calleeId));
        }
    }
}