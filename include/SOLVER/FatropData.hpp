// solver data
#ifndef FATROPDATAINCLUDED
#define FATROPDATAINCLUDED
#include "BLASFEO_WRAPPER/LinearAlgebraBlasfeo.hpp"
#include "TEMPLATES/NLPAlg.hpp"
#include "AUX/Common.hpp"
using namespace std;
namespace fatrop
{
    struct FatropData : public RefCountedObj
    {
        FatropData(const NLPDims &nlpdims) : nlpdims(nlpdims),
                                             memvars(nlpdims.nvars, 6),
                                             memeqs(nlpdims.neqs, 6),
                                             x_curr(memvars[0]),
                                             x_next(memvars[1]),
                                             delta_x(memvars[2]),
                                             x_scales(memvars[3]),
                                             lam_curr(memeqs[0]),
                                             lam_next(memeqs[1]),
                                             lam_calc(memeqs[2]),
                                             lam_scales(memeqs[3]),
                                             g_curr(memeqs[4]),
                                             g_next(memeqs[5]),
                                             grad_curr(memvars[4]),
                                             grad_next(memvars[5])

        {
        }
        int TryStep(double alpha_primal, double alpha_dual)
        {
            axpy(alpha_primal, delta_x, x_curr, x_next);
            axpby(alpha_primal, lam_calc, 1 - alpha_primal, lam_curr, lam_next);
            return 0;
        }
        int TakeStep()
        {
            x_curr.SwapWith(x_next);
            lam_curr.SwapWith(lam_next);
            return 0;
        }
        double CVLinfCurr()
        {
            return Linf(g_curr);
        }
        double CVLinfNext()
        {
            return Linf(g_next);
        }
        const NLPDims nlpdims;
        double obj_scale = 1.0;
        FatropMemoryVecBF memvars;
        FatropMemoryVecBF memeqs;
        FatropVecBF x_curr;
        FatropVecBF x_next;
        FatropVecBF delta_x;
        FatropVecBF x_scales;
        FatropVecBF lam_curr;
        FatropVecBF lam_next;
        FatropVecBF lam_calc;
        FatropVecBF lam_scales;
        FatropVecBF g_curr;
        FatropVecBF g_next;
        FatropVecBF grad_curr;
        FatropVecBF grad_next;
    };
}
#endif // FATROPDATAINCLUDED