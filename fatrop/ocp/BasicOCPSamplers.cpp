#include "BasicOCPSamplers.hpp"
using namespace fatrop;
int StageEvaluator::Size()
{
    return n_rows() * n_cols();
}
IndexEvaluator::IndexEvaluator(const bool control, const vector<int> offsets_in, const vector<int> offsets_out) : _no_var(offsets_in.size()),
                                                                                                                  _offsets_in(offsets_in),
                                                                                                                  _offsets_out(offsets_out),
                                                                                                                  _control(control)
{
}
void IndexEvaluator::Eval(const double *u, const double *x, const double *global_params, const double *stage_params, double *res)
{
    if (_control)
    {
        for (int i = 0; i < _no_var; i++)
        {
            res[_offsets_in.at(i)] = u[_offsets_out.at(i)];
        }
    }
    else
    {
        for (int i = 0; i < _no_var; i++)
        {
            res[_offsets_in.at(i)] = x[_offsets_out.at(i)];
        }
    }
};
int IndexEvaluator::n_rows()
{
    return _no_var;
}
int IndexEvaluator::n_cols()
{
    return 1;
}
EvalBaseSE::EvalBaseSE(const shared_ptr<EvalBase> &evalbase) : evalbase_(evalbase), n_rows_(evalbase->out_m), n_cols_(evalbase->out_n)
{
}
void EvalBaseSE::Eval(const double *u, const double *x, const double *global_params, const double *stage_params, double *res)
{
    const double *arg[] = {u, x, stage_params, global_params};
    evalbase_->eval_array(arg, res);
}
int EvalBaseSE::n_rows()
{
    return n_rows_;
}
int EvalBaseSE::n_cols()
{
    return n_cols_;
}
OCPSolutionSampler::OCPSolutionSampler(int nu, int nx, int no_stage_params, int K, const shared_ptr<StageEvaluator> &eval) : nu(nu),
                                                                                                                             nx(nx),
                                                                                                                             no_stage_params(no_stage_params),
                                                                                                                             K_(K),
                                                                                                                             eval_(eval)
{
}

int OCPSolutionSampler::Size()
{
    return K_ * eval_->Size();
}
int OCPSolutionSampler::n_rows()
{
    return eval_->n_rows();
}
int OCPSolutionSampler::n_cols()
{
    return eval_->n_cols();
}
int OCPSolutionSampler::K()
{
    return K_;
}

int OCPSolutionSampler::Sample(const FatropVecBF &solution, const vector<double> &global_params, const vector<double> &stage_params, vector<double> &sample)
{
    double *sol_p = ((VEC *)solution)->pa;
    double *res_p = sample.data();
    const double *global_params_p = global_params.data();
    const double *stage_params_p = stage_params.data();
    int size = eval_->Size();
    for (int k = 0; k < K_ - 1; k++)
    {
        eval_->Eval(sol_p + k * (nu + nx), sol_p + k * (nu + nx) + nu, global_params_p, stage_params_p + k * no_stage_params, res_p + k * size);
    };
    eval_->Eval(sol_p + (K_ - 2) * (nu + nx), sol_p + (K_ - 1) * (nu + nx), global_params_p, stage_params_p + (K_ - 1) * no_stage_params, res_p + (K_ - 1) * size);
    return 0;
}
vector<double> OCPSolutionSampler::Sample(const FatropVecBF &solution, const vector<double> &global_params, const vector<double> &stage_params)
{
    vector<double> res(Size());
    Sample(solution, global_params, stage_params, res);
    return res;
}

ParameterSetter::ParameterSetter(const shared_ptr<BFOCPAdapter> &ocp, const vector<int> &offsets_in, const vector<int> &offsets_out, const int no_stage_params, const int no_var, const int K, const bool global) : ocp_(ocp), _offsets_in(offsets_in), _offsets_out(offsets_out), no_stage_params(no_stage_params), _no_var(no_var), K(K), _global(global)
{
}
void ParameterSetter::SetValue(const double value[])
{
    if (_global)
    {
        double *params = ocp_->GetGlobalParams();
        for (int i = 0; i < _no_var; i++)
        {
            params[_offsets_out.at(i)] = value[_offsets_in.at(i)];
        }
    }
    else // stage paramter
    {
        double *params = ocp_->GetStageParams();
        for (int k = 0; k < K; k++)
        {
            for (int i = 0; i < _no_var; i++)
            {
                params[_offsets_out.at(i) + k * no_stage_params] = value[_offsets_in.at(i)];
            }
        }
    }
}
void ParameterSetter::SetValue(const initializer_list<double> il_)
{
    assert((int)il_.size() == _no_var);
    SetValue(il_.begin());
}