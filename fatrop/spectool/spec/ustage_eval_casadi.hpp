#pragma once
extern "C"
{
#include <blasfeo.h>
}
#include "fatrop/spectool/spec/ustage_eval_abstract.hpp"
#include "fatrop/spectool/function_evaluation/casadi_fe.hpp"
#include "fatrop/spectool/spec/ustage_quantities.hpp"
namespace fatrop
{
    namespace spectool
    {
        class FatropuStageEvalCasadi : public FatropuStageEvalAbstract
        {
        public:
            FatropuStageEvalCasadi(const uStageQuantities &sq, const cs::Dict &opts, CasadiJitCache &eval_cache);
            virtual int nu(const uStageQuantities &sq);
            virtual int nx(const uStageQuantities &sq);
            virtual int np_stage(const uStageQuantities &sq);
            virtual int np_global(const uStageQuantities &sq);
            virtual int ng_eq(const uStageQuantities &sq);
            virtual int ng_ineq(const uStageQuantities &sq);
            virtual int nxp1(const uStageQuantities &sq);
            virtual std::pair<cs::MX, cs::MX> dynamics_jacobian_sym(const uStageQuantities &sq, const cs::MX &xp1);
            virtual std::pair<cs::MX, cs::MX> equality_jacobian_sym(const uStageQuantities &sq);
            virtual std::pair<cs::MX, cs::MX> inequality_jacobian_sym(const uStageQuantities &sq);
            virtual std::pair<cs::MX, cs::MX> hess_lag_obj_sym(const uStageQuantities &sq);
            virtual std::pair<cs::MX, cs::MX> hess_lag_dyn_sym(const uStageQuantities &sq, const cs::MX &lam_dyn);
            virtual std::pair<cs::MX, cs::MX> hess_lag_eq_sym(const uStageQuantities &sq, const cs::MX &lam_g_equality);
            virtual std::pair<cs::MX, cs::MX> hess_lag_ineq_sym(const uStageQuantities &sq, const cs::MX &lam_g_inequality);
            virtual std::pair<cs::MX, cs::MX> hess_lag_sym(const uStageQuantities &sq, const cs::MX &lam_dyn, const cs::MX &lam_g_equality, const cs::MX &lam_g_inequality);

        public:
            virtual int get_nx() const;
            virtual int get_nu() const;
            virtual int get_ng() const;
            virtual int get_n_stage_params() const;
            virtual int get_ng_ineq() const;
            virtual int eval_BAbt(
                const double *states_kp1,
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                MAT *res);
            virtual int eval_RSQrqt(
                const double *objective_scale,
                const double *inputs_k,
                const double *states_k,
                const double *lam_dyn_k,
                const double *lam_eq_k,
                const double *lam_eq_ineq_k,
                const double *stage_params_k,
                const double *global_params,
                MAT *res);
            virtual int eval_Ggt(
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                MAT *res);
            virtual int eval_Ggt_ineq(
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                MAT *res);
            virtual int eval_b(
                const double *states_kp1,
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                double *res);
            virtual int eval_g(
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                double *res);
            virtual int eval_gineq(
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                double *res);
            virtual int eval_rq(
                const double *objective_scale,
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                double *res);
            virtual int eval_L(
                const double *objective_scale,
                const double *inputs_k,
                const double *states_k,
                const double *stage_params_k,
                const double *global_params,
                double *res);
            virtual int get_bounds(double *lower, double *upper) const;
            CasadiFEWrap RSQrqt_;
            CasadiFEWrap BAbt_;
            CasadiFEWrap L_;
            CasadiFEWrap b_; // actually provided as dynamics
            CasadiFEWrap g_equality_;
            CasadiFEWrap g_inequality_;
            CasadiFEWrap rq_;
            CasadiFEWrap Ggt_equality_;
            CasadiFEWrap Ggt_inequality_;
            std::vector<double> Lb_;
            std::vector<double> Ub_;
            int K_;
            int nu_;
            int nx_;
            int nxp1_;
            int np_stage_;
            int np_global_;
            int ng_eq_;
            int ng_ineq_;
        };
    }
}