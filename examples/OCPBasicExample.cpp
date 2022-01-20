#include "OCP/BFOCPBasic.hpp"
#include "OCP/BFOCP.hpp"
#include "OCP/BFOCPAdapter.hpp"
#include "AUX/SmartPtr.hpp"
#include "AUX/FatropMemory.hpp"
#include "OCP/OCPAlg.hpp"
#include "OCP/OCPLinearSolver.hpp"
#include "OCP/OCPLSRiccati.hpp"
#include "AUX/FatropVector.hpp"
using namespace fatrop;
int main()
{
    RefCountPtr<BFOCP> ocptemplatebasic =
        new BFOCPBasic(BFOCPBasic::from_shared_lib("./f.so", 150));
    RefCountPtr<OCP> ocptempladapter = new BFOCPAdapter(ocptemplatebasic);
    RefCountPtr<OCPLinearSolver> ocplsriccati = new OCPLSRiccati(ocptempladapter->GetOCPDims());
    FatropOCP ocpalg(ocptempladapter, ocplsriccati);
    NLPDims nlpdims = ocpalg.GetNLPDims();
    blasfeo_timer timer;
    FatropMemoryVecBF ux(nlpdims.nvars, 2);
    FatropMemoryVecBF lags(nlpdims.neqs, 2);
    int N = 1000;
    blasfeo_tic(&timer);
    for (int i = 0; i < N; i++)
    {
        ocpalg.EvalHess(1.0, ux[0], ux[0], lags[0], lags[0]);
        ocpalg.EvalJac(ux[0], ux[0], lags[0]);
    }
    double el = blasfeo_toc(&timer);
    cout << "time elapsed " << el/N << endl;
}