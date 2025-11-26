/**
 * Andersen.cpp
 * @author kisslune
 */

#include "A5Header.h"

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

    // TODO: complete the following method
    andersen.runPointerAnalysis();

    andersen.dumpResult();
    SVF::LLVMModuleSet::releaseLLVMModuleSet();
	return 0;
}
void Andersen::runPointerAnalysis()
{
    WorkList<unsigned> workList;

    for (auto &pair : *consg) {
        auto dstId = pair.first;
        SVF::ConstraintNode* node = pair.second;

        for (auto edge : node->getAddrInEdges()) {
            auto addrEdge = static_cast<SVF::AddrCGEdge*>(edge);
            auto srcId = addrEdge->getSrcID();

            if (pts[dstId].insert(srcId).second)
                workList.push(dstId);
        }
    }

    while (!workList.empty()) {
        unsigned p = workList.pop();
        SVF::ConstraintNode* pNode = consg->getConstraintNode(p);

        auto &pts_p = pts[p];   

        for (unsigned o : pts_p) {
            for (auto edge : pNode->getStoreInEdges()) {
                auto storeEdge = static_cast<SVF::StoreCGEdge*>(edge);
                auto q = storeEdge->getSrcID();

                SVF::ConstraintNode* qNode = consg->getConstraintNode(q);

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
                auto loadEdge = static_cast<SVF::LoadCGEdge*>(edge);
                auto r = loadEdge->getDstID();

                SVF::ConstraintNode* rNode = consg->getConstraintNode(r);

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
            auto copyEdge = static_cast<SVF::CopyCGEdge*>(edge);
            auto x = copyEdge->getDstID();

            auto &pts_x = pts[x];
            size_t oldSize = pts_x.size();
            pts_x.insert(pts_p.begin(), pts_p.end());

            if (pts_x.size() != oldSize)
                workList.push(x);
        }

        for (auto edge : pNode->getGepOutEdges()) {
            auto gepEdge = static_cast<SVF::GepCGEdge*>(edge);
            auto x = gepEdge->getDstID();

            auto &pts_x = pts[x];
            size_t oldSize = pts_x.size();

            for (unsigned o : pts_p) {
                unsigned fieldObj = consg->getGepObjVar(o, gepEdge);
                pts_x.insert(fieldObj);
            }
            if (pts_x.size() != oldSize)
                workList.push(x);
        }
    }
}