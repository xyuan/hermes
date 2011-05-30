#include "hermes2d.h"

/* Nonlinearity lambda(u) = pow(u, alpha) */

class CustomNonlinearity : public HermesFunction
{
public:
  CustomNonlinearity(double alpha): HermesFunction()
  {
    this->is_const = false;
    this->alpha = alpha;
  }

  virtual scalar value(double u) const
  {
    return 1 + pow(u, alpha);
  }

  virtual Ord value(Ord u) const
  {
    // If alpha is not an integer, then the function
    // is non-polynomial. 
    // NOTE: Setting Ord to 10 is safe but costly,
    // one could save here by looking at special cases 
    // of alpha. 
    return Ord(10);
  }

  protected:
    double alpha;
};

/* Weak forms */

// NOTE: The linear problem in each step of the Picard's 
//       method is solved using the Newton's method.

class CustomWeakFormPicard : public WeakForm
{
public:
  CustomWeakFormPicard(Solution* prev_iter_sln, HermesFunction* lambda, HermesFunction* f) 
    : WeakForm(1)
  {
    // Jacobian (custom because of the external function).
    CustomJacobian* matrix_form = new CustomJacobian(0, 0, lambda);
    matrix_form->ext.push_back(prev_iter_sln);
    add_matrix_form(matrix_form);

    // Residual (custom because of the external function).
    CustomResidual* vector_form = new CustomResidual(0, lambda, f);
    vector_form->ext.push_back(prev_iter_sln);
    add_vector_form(vector_form);
  };

private:
  class CustomJacobian : public WeakForm::MatrixFormVol
  {
  public:
    CustomJacobian(int i, int j, HermesFunction* lambda) : WeakForm::MatrixFormVol(i, j), lambda(lambda)
    { 
    }

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *u,
                         Func<double> *v, Geom<double> *e, ExtData<scalar> *ext) const 
    {
      scalar result = 0;
      for (int i = 0; i < n; i++) 
      {
        result += wt[i] * lambda->value(ext->fn[0]->val[i]) 
                        * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i]);
      }
      return result;
    }

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v,
                    Geom<Ord> *e, ExtData<Ord> *ext) const 
    {
      Ord result = 0;
      for (int i = 0; i < n; i++) 
      {
        result += wt[i] * lambda->value(ext->fn[0]->val[i]) 
                        * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i]);
      }
      return result;
    }
    
    protected:
      HermesFunction* lambda;
  };

  class CustomResidual : public WeakForm::VectorFormVol
  {
  public:
    CustomResidual(int i, HermesFunction* lambda, HermesFunction* f) 
      : WeakForm::VectorFormVol(i), lambda(lambda), f(f) 
    { 
    }

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[],
                         Func<double> *v, Geom<double> *e, ExtData<scalar> *ext) const 
    {
      scalar result = 0;
      for (int i = 0; i < n; i++) 
      {
        result += wt[i] * lambda->value(ext->fn[0]->val[i]) 
                        * (u_ext[0]->dx[i] * v->dx[i] + u_ext[0]->dy[i] * v->dy[i]);
        result += wt[i] * f->value(e->x[i], e->y[i]) * v->val[i];
      }
      return result;
    }

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, 
                    Geom<Ord> *e, ExtData<Ord> *ext) const 
    {
      Ord result = 0;
      for (int i = 0; i < n; i++) 
      {
        result += wt[i] * lambda->value(ext->fn[0]->val[i]) * (u_ext[0]->dx[i] 
                        * v->dx[i] + u_ext[0]->dy[i] * v->dy[i]);
        result += wt[i] * f->value(e->x[i], e->y[i]) * v->val[i];
      }
      return result;
    }

    private:
      HermesFunction* lambda;
      HermesFunction* f;
  };
};

/* Essential boundary conditions */

class CustomEssentialBCNonConst : public EssentialBoundaryCondition {
public:
  CustomEssentialBCNonConst(std::string marker) 
           : EssentialBoundaryCondition(Hermes::vector<std::string>(marker))
  {
  }

  inline EssentialBCValueType get_value_type() const 
  { 
    return EssentialBoundaryCondition::BC_FUNCTION; 
  }

  virtual scalar value(double x, double y, double n_x, double n_y, 
                       double t_x, double t_y) const
  {
    return (x+10) * (y+10) / 100.;
  }
};
