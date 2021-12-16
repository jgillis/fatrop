/**
 * @file FatropSparse.hpp
 * @author your name (you@domain.com)
 * @brief this file contains functions for representing a block-sparse KKT matrix, only used for DEBUG and TESTING purposes. This code does NOT aim to be efficient, neither in computational nor memory efficiency.
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef FATROP_SPARSE_INCLUDED
#define FATROP_SPARSE_INCLUDED
/* TODO 
make a separate file for OCP's
*/
#include "../OCP/FatropOCPKKT.hpp"
#include "../DEBUG/FatropLinearAlgebraEigen.hpp"
#include <vector>
#include <memory>
#include <cstdlib>
#include <string>
#include <iomanip> // std::setprecision
#include <eigen3/Eigen/Sparse>
using namespace std;
namespace fatrop
{
    /* TODO -> I'm not happy with how expressions are built up here (using shared pointers). We should make use of nodes that are reference counted and "Expression elements"
    class elem
    node*
    upon destruction node->ref_count -- if 0 -> free
    copy -> node_ref_count ++;


    class sum: node
    members:
    ref_count 
    elem1
    elem2

    elem operator+(elem1, elem2)
    elem res
    res.node = new sum(1,2)
    return res
    */
    struct triplet
    {
        triplet(const int ai, const int aj, const double value) : ai(ai), aj(aj), val(value){};
        int row() const { return ai; };
        int col() const { return aj; };
        double value() const { return val; };
        int ai;
        int aj;
        double val;
    };
    class variable
    {
    public:
        variable(int size) : size(size){};
        int size;
        int offset;
        void set_grad(const vector<double> &grad_)
        {
            grad.assign(size, 0.0);
            for (int i = 0; i < size; i++)
            {
                grad.at(i) = grad_.at(i);
            }
        };
        void add_rhs(vector<double> &rhs)
        {
            for (int i = 0; i < size; i++)
            {
                rhs.at(offset + i) = grad.at(i);
            }
        }
        vector<double> grad;
    };
    typedef shared_ptr<variable> var_sp;
    class KKT_matrix_base
    {
    };
    class matrix_vector_base
    {
    public:
        matrix_vector_base(const Eig &mat, var_sp var) : fsm(mat), var(var){};
        Eig fsm;
        var_sp var;
    };

    class fatrop_expression
    {
    public:
        virtual bool is_matrix_vector() { return false; };
        virtual void add_to_mv_vec(vector<matrix_vector_base> &mv_vec) = 0;
        virtual int get_size() = 0;
    };
    typedef shared_ptr<fatrop_expression> fe_sp;
    class equation
    {
    public:
        equation(int size) : size(size){};
        int size;
        int offset;
        void add_expression(const fe_sp &express, vector<double> &rhs_)
        {
            rhs = rhs_;
            express->add_to_mv_vec(mv_veceq);
        }
        void add_triplets(vector<triplet> &tripl, int offs_H)
        {
            for (unsigned long int i = 0; i < mv_veceq.size(); i++)
            {
                matrix_vector_base mvi = mv_veceq.at(i);
                int offs_var = mvi.var->offset;
                Eig &fsm = mvi.fsm;
                int m = fsm.nrows();
                int n = fsm.ncols();
                for (int i = 0; i < m; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        double val = fsm.get_el(i, j);
                        if (!fsm.iszero(i, j))
                        {

                            tripl.push_back(triplet(offs_H + offset + i, offs_var + j, val));
                        }
                    }
                }
            }
        }
        void add_rhs(vector<double> &rhs_, int offs_H)
        {
            for (int i = 0; i < size; i++)
            {
                rhs_.at(offs_H + offset + i) = rhs.at(i);
            }
        }
        vector<matrix_vector_base> mv_veceq;
        vector<double> rhs;
    };
    typename std::shared_ptr<equation> eq_sp;

    class fatrop_sum1 : public fatrop_expression
    {
    public:
        fatrop_sum1(const fe_sp &fe1, const fe_sp &fe2) : child1(fe1), child2(fe2){};
        void add_to_mv_vec(vector<matrix_vector_base> &mv_vec)
        {
            child1->add_to_mv_vec(mv_vec);
            child2->add_to_mv_vec(mv_vec);
        };
        int get_size()
        {
            return child1->get_size();
        }
        const fe_sp child1;
        const fe_sp child2;
    };
    fe_sp operator+(const fe_sp &fe1, const fe_sp &fe2)
    {
        fe_sp res = make_shared<fatrop_sum1>(fe1, fe2);
        return res;
    }
    class matrix_vector : public fatrop_expression, public matrix_vector_base
    {
    public:
        matrix_vector(const Eig &mat, var_sp var) : matrix_vector_base(mat, var){};
        bool is_matrix_vector() { return true; };
        void add_to_mv_vec(vector<matrix_vector_base> &mv_vec)
        {
            matrix_vector_base *mvb = static_cast<matrix_vector_base *>(this);
            mv_vec.push_back(*mvb);
        }
        int get_size()
        {
            return fsm.nrows();
        }
    };
    fe_sp operator*(const Eig &mat, var_sp var)
    {
        fe_sp res = make_shared<matrix_vector>(mat, var);
        return res;
    };

    class hess_block
    {
    public:
        hess_block(const Eig &mat, var_sp var1, var_sp var2) : fsm(mat), var1(var1), var2(var2){};
        Eig fsm;
        var_sp var1;
        var_sp var2;
        void add_triplets(vector<triplet> &tripl)
        {
            int offs_var1 = var1->offset;
            int offs_var2 = var2->offset;
            int m = fsm.nrows();
            int n = fsm.ncols();
            for (int i = 0; i < m; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    double val = fsm.get_el(i, j);
                    if (!fsm.iszero(i, j)) // only add nonzero's
                    {
                        int ai = i + offs_var1;
                        int aj = j + offs_var2;
                        if (ai >= aj) // only lower triangular matrix
                        {
                            tripl.push_back(triplet(ai, aj, val));
                        }
                        else
                        {
                            if (offs_var1 != offs_var2)
                            {
                                tripl.push_back(triplet(aj, ai, val));
                            }
                        }
                    }
                }
            }
        }
    };

    class KKT_matrix : public KKT_matrix_base
    {
    public:
        var_sp get_variable(int size)
        {
            var_sp var = make_shared<variable>(size);
            variable_vec.push_back(var);
            return variable_vec.back();
        };
        equation &get_equation(int size)
        {
            equation eq(size);
            equation_vec.push_back(eq);
            return equation_vec.back();
        };
        void set_equation(fe_sp expr, vector<double> rhs)
        {
            equation &eq = get_equation(expr->get_size());
            eq.add_expression(expr, rhs);
        }
        void set_hess_block(const Eig &mat, var_sp var1, var_sp var2)
        {
            hess_block hb(mat, var1, var2);
            hess_block_vec.push_back(hb);
        }

        vector<var_sp> variable_vec;
        vector<equation> equation_vec;
        vector<hess_block> hess_block_vec;
        void set_offsets()
        {
            int offs_curr = 0;
            for (long unsigned int i = 0; i < variable_vec.size(); i++)
            {
                variable_vec.at(i)->offset = offs_curr;
                offs_curr += variable_vec.at(i)->size;
            }
            offs_curr = 0;
            for (long unsigned int i = 0; i < equation_vec.size(); i++)
            {
                equation_vec.at(i).offset = offs_curr;
                offs_curr += equation_vec.at(i).size;
            }
        }
        // TODO vector<triple> get_triplets, easier to use
        void get_triplets(vector<triplet> &tripl_vec)
        {
            // tripl_vec.resize(0);
            this->set_offsets();
            for (long unsigned int i = 0; i < hess_block_vec.size(); i++)
            {
                hess_block_vec.at(i).add_triplets(tripl_vec);
            }
            int offs_vars = variable_vec.back()->offset + variable_vec.back()->size;
            // std::cout << "offs "<< offs_vars << std::endl;
            for (long unsigned int i = 0; i < equation_vec.size(); i++)
            {
                equation_vec.at(i).add_triplets(tripl_vec, offs_vars);
            }
        }
        vector<double> get_rhs()
        {
            vector<double> rhs;
            this->set_offsets();
            rhs.assign(get_size(), 0.0);
            for (long unsigned int i = 0; i < variable_vec.size(); i++)
            {
                variable_vec.at(i)->add_rhs(rhs);
            }
            int offs_vars = variable_vec.back()->offset + variable_vec.back()->size;
            // cout << "offs "<< offs_vars << std::endl;
            for (long unsigned int i = 0; i < equation_vec.size(); i++)
            {
                equation_vec.at(i).add_rhs(rhs, offs_vars);
            }
            return rhs;
        }
        int get_size()
        {
            set_offsets();
            return equation_vec.back().offset + equation_vec.back().size + variable_vec.back()->offset + variable_vec.back()->size;
        }

        void print(const char *type_id)
        {

            vector<triplet> testvec;
            get_triplets(testvec);
            //printing (matrix)
            if (string(type_id) == "matrix")
            {
                cout << " " << get_size() << " " << get_size() << endl
                     << " ";
                cout << fixed;

                Eigen::MatrixXd mat(get_size(), get_size() + 1);

                for (long unsigned int i = 0; i < testvec.size(); i++)
                {
                    // std::cout << testvec.at(i).col() + 1 << " " << testvec.at(i).row()  << " " << std::setprecision(20) << testvec.at(i).value() << std::endl
                    mat(testvec.at(i).row(), testvec.at(i).col()) = testvec.at(i).value();
                    mat(testvec.at(i).col(), testvec.at(i).row()) = testvec.at(i).value();
                }
                vector<double> rhs = get_rhs();
                for (int i = 0; i < get_size(); i++)
                {
                    mat(i, get_size()) = rhs.at(i);
                }
                cout << mat << endl;
            }
            //printing (ma57)
            if (string(type_id) == "ma57")
            {
                cout << " " << get_size() << " " << testvec.size() << endl
                     << " ";
                cout << fixed;

                for (long unsigned int i = 0; i < testvec.size(); i++)
                {
                    cout << testvec.at(i).col() + 1 << " " << testvec.at(i).row() + 1 << " " << setprecision(20) << testvec.at(i).value() << endl
                         << " ";
                }
                vector<double> rhs = get_rhs();
                for (int i = 0; i < get_size(); i++)
                {
                    cout << rhs.at(i) << " ";
                }
                cout << endl;
            }
            //printing (MUMPS)
            if (string(type_id) == "mumps")
            {
                cout << get_size() << "     :N \n"
                     << testvec.size() << "     :NZ ";
                cout << fixed;

                for (long unsigned int i = 0; i < testvec.size(); i++)
                {
                    cout << endl
                         << testvec.at(i).col() + 1 << " " << testvec.at(i).row() + 1 << " " << setprecision(20) << testvec.at(i).value();
                }
                cout << "            :values";
                vector<double> rhs = get_rhs();
                for (int i = 0; i < get_size(); i++)
                {
                    cout << endl
                         << rhs.at(i);
                }
                cout << "            :RHS";
                cout << endl;
            }
        }
    };
#define Id Eigen::MatrixXd::Identity
    KKT_matrix Sparse_OCP(OCP_dims &dims, OCP_KKT &OCP, bool Guzero = false)
    {
        int K = dims.K;
        KKT_matrix KKT;
        vector<var_sp> u_vec;
        vector<var_sp> x_vec;
        // initialize variables
        for (int k = 0; k < K - 1; k++)
        {
            int nu = dims.nu.at(k);
            int nx = dims.nx.at(k);
            u_vec.push_back(KKT.get_variable(nu));
            x_vec.push_back(KKT.get_variable(nx));
        }
        {
            int nx = dims.nx.at(K - 1);
            x_vec.push_back(KKT.get_variable(nx));
        }
        for (int k = 0; k < K - 1; k++)
        {
            int nu = dims.nu.at(k);
            int nx = dims.nx.at(k);
            int ng = dims.ng.at(k);
            vector<double> rhs_dyn;
            vector<double> rhs_con;
            vector<double> grad_u;
            vector<double> grad_x;
            for (int i = 0; i < nx; i++)
            {
                rhs_dyn.push_back(OCP.BAbt[k].get_el(nu + nx, i));
            };
            for (int i = 0; i < ng; i++)
            {
                rhs_con.push_back(OCP.Ggt[k].get_el(nx, i));
            };
            for (int i = 0; i < nu; i++)
            {
                grad_u.push_back(OCP.RSQrqt[k].get_el(nu + nx, i));
            };
            for (int i = 0; i < nx; i++)
            {
                grad_x.push_back(OCP.RSQrqt[k].get_el(nu + nx, nu + i));
            };
            x_vec.at(k)->set_grad(grad_x);
            u_vec.at(k)->set_grad(grad_u);
            KKT.set_hess_block(Eig(OCP.RSQrqt[k].block(0, 0, nu, nu)), u_vec.at(k), u_vec.at(k));
            KKT.set_hess_block(Eig(OCP.RSQrqt[k].block(nu, nu, nx, nx)), x_vec.at(k), x_vec.at(k));
            KKT.set_hess_block(Eig(OCP.RSQrqt[k].block(nu, 0, nx, nu)), x_vec.at(k), u_vec.at(k));
            Eig B(Eig(Eig(OCP.BAbt[k].block(0, 0, nu, nx)).transpose()));
            Eig A(Eig(Eig(OCP.BAbt[k].block(nu, 0, nx, nx)).transpose()));
            // TODO EYE IS HERE CONSIDERED AS A DENSE MATRIX
            KKT.set_equation(B * u_vec.at(k) + A * (x_vec.at(k)) + Eig(-Id(dims.nx.at(k + 1), dims.nx.at(k + 1))) * (x_vec.at(k + 1)), rhs_dyn);
            if (Guzero)
            {
                Eig Gx(Eig(OCP.Ggt[k].block(nu, 0, nx, ng)).transpose());
                KKT.set_equation(Gx * x_vec.at(k), rhs_con);
            }
            else
            {
                Eig Gu(Eig(OCP.Ggt[k].block(0, 0, nu, ng)).transpose());
                Eig Gx(Eig(OCP.Ggt[k].block(nu, 0, nx, ng)).transpose());
                KKT.set_equation(Gu * u_vec.at(k) + Gx * x_vec.at(k), rhs_con);
            }
        }
        // K - 1
        {
            int nu = dims.nu.at(K - 1);
            int nx = dims.nx.at(K - 1);
            int ng = dims.ng.at(K - 1);
            vector<double> rhs_con;
            vector<double> grad_x;
            for (int i = 0; i < ng; i++)
            {
                rhs_con.push_back(OCP.Ggt[K - 1].get_el(nx, i));
            };
            for (int i = 0; i < nx; i++)
            {
                grad_x.push_back(OCP.RSQrqt[K - 1].get_el(nu + nx, nu + i));
            };
            Eig Gx(Eig(OCP.Ggt[K - 1].block(nu, 0, nx, ng)).transpose());
            x_vec.at(K - 1)->set_grad(grad_x);
            KKT.set_hess_block(Eig(OCP.RSQrqt[K - 1].block(nu, nu, nx, nx)), x_vec.at(K - 1), x_vec.at(K - 1));
            KKT.set_equation(Gx * x_vec.at(K - 1), rhs_con);
        }
        return KKT;
    };
} // namespace fatrop

#endif //FATROP_SPARSE_INCLUDED