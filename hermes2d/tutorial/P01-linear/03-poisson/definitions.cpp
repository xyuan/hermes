#include "definitions.h"

/* Weak forms */

CustomWeakFormPoisson::CustomWeakFormPoisson(std::string mat_al, HermesFunction* lambda_al,
                                             std::string mat_cu, HermesFunction* lambda_cu,
                                             HermesFunction* src_term) : WeakForm(1)
{
  // Jacobian forms.
  add_matrix_form(new WeakFormsH1::DefaultJacobianDiffusion(0, 0, mat_al, lambda_al));
  add_matrix_form(new WeakFormsH1::DefaultJacobianDiffusion(0, 0, mat_cu, lambda_cu));

  // Residual forms.
  add_vector_form(new WeakFormsH1::DefaultResidualDiffusion(0, mat_al, lambda_al));
  add_vector_form(new WeakFormsH1::DefaultResidualDiffusion(0, mat_cu, lambda_cu));
  add_vector_form(new WeakFormsH1::DefaultVectorFormVol(0, HERMES_ANY, src_term));
};
