#define HERMES_REPORT_ALL
#define HERMES_REPORT_FILE "application.log"
#include "hermes2d.h"

using namespace RefinementSelectors;

//  This example shows how to set an arbitrary initial guess for the
//  Newton's method, and nonzero Dirichlet boundary conditions.
//
//  PDE: magnetostatics with nonlinear magnetic permeability
//  curl[1/mu curl u] = current_density.
//
//  Domain: unit square (-10,10)^2.
//
//  BC: Dirichlet, see function dir_lift() below.
//
//  The following parameters can be changed:

const int P_INIT = 3;                             // Initial polynomial degree.
const double NEWTON_TOL = 1e-10;                  // Stopping criterion for the Newton's method.
const int NEWTON_MAX_ITER = 1000;                 // Maximum allowed number of Newton iterations.
const double NEWTON_DAMPING = 1.0;                // Number between 0 and 1 to damp Newton's iterations.
const int INIT_REF_NUM = 0;                       // Number of initial uniform mesh refinements.
MatrixSolverType matrix_solver = SOLVER_UMFPACK;  // Possibilities: SOLVER_AMESOS, SOLVER_AZTECOO, SOLVER_MUMPS,
                                                  // SOLVER_PETSC, SOLVER_SUPERLU, SOLVER_UMFPACK.

// Problem parameters.
double MU_VACUUM = 4. * M_PI * 1e-7;
double INIT_COND = 0.0;                           // Initial condition for the magnetic potential.
double CURRENT_DENSITY = 1e9;                     // Volume source term.

// Material and boundary markers.
const std::string MAT_AIR = "2";
const std::string MAT_IRON_1 = "0";
const std::string MAT_IRON_2 = "3";
const std::string MAT_COPPER = "1";
const std::string BDY_DIRICHLET = "1";

// Weak forms.
#include "definitions.cpp"

int main(int argc, char* argv[])
{
  // Instantiate a class with global functions.
  Hermes2D hermes2d;

  // Define nonlinear magnetic permeability via a cubic spline.
  Hermes::vector<double> mu_inv_pts(0.0,      0.5,      0.9,      1.0,      1.1,      1.2,      1.3,
                                    1.4,      1.6,      1.7,      1.8,      1.9,      3.0,      5.0,     10.0);
  Hermes::vector<double> mu_inv_val(1/1500.0, 1/1480.0, 1/1440.0, 1/1400.0, 1/1300.0, 1/1150.0, 1/950.0,
                                    1/750.0,  1/250.0,  1/180.0,  1/175.0,  1/150.0,  1/20.0,   1/10.0,  1/5.0);
  /*
  // This is for debugging (iron is assumed linear with mu_r = 300.0
  Hermes::vector<double> mu_inv_pts(0.0,      10.0);
  Hermes::vector<double> mu_inv_val(1/300.0,   1/300.0);
  */

  // Create the cubic spline (and plot it for visual control).
  double bc_left = 0.0;
  double bc_right = 0.0;
  bool first_der_left = false;
  bool first_der_right = false;
  bool extrapolate_der_left = false;
  bool extrapolate_der_right = false;
  CubicSpline mu_inv_iron(mu_inv_pts, mu_inv_val, bc_left, bc_right, first_der_left, first_der_right,
                          extrapolate_der_left, extrapolate_der_right);
  info("Saving cubic spline into a Pylab file spline.dat.");
  double interval_extension = 1.0; // The interval of definition of the spline will be
                                   // extended by "interval_extension" on both sides.
  bool plot_derivative = false;
  mu_inv_iron.plot("spline.dat", interval_extension, plot_derivative);
  plot_derivative = true;
  mu_inv_iron.plot("spline_der.dat", interval_extension, plot_derivative);

  // Load the mesh.
  Mesh mesh;
  H2DReader mloader;
  mloader.load("actuator.mesh", &mesh);

  // Perform initial mesh refinements.
  for(int i = 0; i < INIT_REF_NUM; i++) mesh.refine_all_elements();

  //MeshView mv("Mesh", new WinGeom(0, 0, 400, 400));
  //mv.show(&mesh);

  // Initialize boundary conditions.
  DefaultEssentialBCConst bc_essential(BDY_DIRICHLET, 0.0);
  EssentialBCs bcs(&bc_essential);

  // Create an H1 space with default shapeset.
  H1Space space(&mesh, &bcs, P_INIT);
  info("ndof: %d", Space::get_num_dofs(&space));

  // Initialize the weak formulation
  int order_inc = 3; // This determines the increase of integration order
                     // for the axisymmetric term containing 1/r. Default is 3.
  CustomWeakFormMagnetostatics wf(MAT_IRON_1, MAT_IRON_2, &mu_inv_iron, MAT_AIR,
                                  MAT_COPPER, MU_VACUUM, CURRENT_DENSITY, order_inc);

  // Initialize the FE problem.
  DiscreteProblem dp(&wf, &space);

  // Set up the solver, matrix, and rhs according to the solver selection.
  SparseMatrix* matrix = create_matrix(matrix_solver);
  Vector* rhs = create_vector(matrix_solver);
  Solver* solver = create_linear_solver(matrix_solver, matrix, rhs);

  // Initialize the solution.
  Solution sln(&mesh, INIT_COND);

  // Project the initial condition on the FE space to obtain initial
  // coefficient vector for the Newton's method.
  info("Projecting to obtain initial vector for the Newton's method.");
  scalar* coeff_vec = new scalar[Space::get_num_dofs(&space)] ;
  OGProjection::project_global(&space, &sln, coeff_vec, matrix_solver);

  // Perform Newton's iteration.
  bool verbose = true;
  bool residual_as_function = false;
  bool jacobian_changed = true;
  if (!hermes2d.solve_newton(coeff_vec, &dp, solver, matrix, rhs, jacobian_changed,
           NEWTON_TOL, NEWTON_MAX_ITER, verbose, residual_as_function,
                             NEWTON_DAMPING)) error("Newton's iteration failed.");

  // Translate the resulting coefficient vector into the Solution sln.
  Solution::vector_to_solution(coeff_vec, &space, &sln);

  // Cleanup.
  delete [] coeff_vec;
  delete matrix;
  delete rhs;
  delete solver;

  // Visualise the solution and mesh.
  ScalarView s_view1("Vector potencial", new WinGeom(0, 0, 350, 450));
  FilterVectorPotencial vector_potencial(Hermes::vector<MeshFunction *>(&sln, &sln),
                                         Hermes::vector<int>(H2D_FN_VAL, H2D_FN_VAL));
  s_view1.show_mesh(false);
  s_view1.show(&vector_potencial);

  ScalarView s_view2("Flux density", new WinGeom(360, 0, 350, 450));
  FilterFluxDensity flux_density(Hermes::vector<MeshFunction *>(&sln, &sln));
  s_view2.show_mesh(false);
  s_view2.show(&flux_density);

  OrderView o_view("Mesh", new WinGeom(720, 0, 350, 450));
  o_view.show(&space);

  // Wait for all views to be closed.
  View::wait();
  return 0;
}

