//
// File: new_optimizer.cpp
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-07-07 00:00:00
// Last modified: 2017-07-07
//

/*
  Copyright or © or Copr. Bio++ Development Team, (November 17, 2004)

  This software is a computer program whose purpose is to provide classes
  for numerical calculus. This file is part of the Bio++ project.

  This software is governed by the CeCILL license under French law and
  abiding by the rules of distribution of free software. You can use,
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info".

  As a counterpart to the access to the source code and rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty and the software's author, the holder of the
  economic rights, and the successive licensors have only limited
  liability.

  In this respect, the user's attention is drawn to the risks associated
  with loading, using, modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean that it is complicated to manipulate, and that also
  therefore means that it is reserved for developers and experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or
  data to be ensured and, more generally, to use and operate it in the
  same conditions as regards security.

  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <Bpp/NewPhyl/DataFlowTemplates.h>
#include <Bpp/NewPhyl/Debug.h>
#include <Bpp/NewPhyl/Optimizer.h>

using namespace bpp::DF;

// x -> x^2 and its derivatives

struct SquareOp : public OperationBase<SquareOp>
{
  using ArgumentTypes = std::tuple<double>;
  using ResultType = double;
  static void compute(ResultType& r, const double& x) { r = x * x; }
  static NodeRef derive(Node& self, const Node& variable);
  static std::string description() { return "x^2"; }
};
using SquareNode = GenericFunctionComputation<SquareOp>;

struct DSquareOp : public OperationBase<DSquareOp>
{
  using ArgumentTypes = std::tuple<double, double>;
  using ResultType = double;
  static void compute(ResultType& r, const double& x, const double& dx_dv) { r = 2 * x * dx_dv; }
  static NodeRef derive(Node& self, const Node& variable);
  static std::string description() { return "2 * x * dx/dvar"; }
};
using DSquareNode = GenericFunctionComputation<DSquareOp>;

struct DDSquareOp : public OperationBase<DDSquareOp>
{
  using ArgumentTypes = std::tuple<double, double, double, double>;
  using ResultType = double;
  static void compute(ResultType& r, const double& x, const double& y, const double& dx_dv, const double& dy_dv)
  {
    r = 2 * (dx_dv * y + x * dy_dv);
  }
};
using DDSquareNode = GenericFunctionComputation<DDSquareOp>;

NodeRef SquareOp::derive(Node& self, const Node& variable)
{
  auto& x = self.dependencies()[0];
  return createNode<DSquareNode>({x, x->derive(variable)});
}
NodeRef DSquareOp::derive(Node& self, const Node& variable)
{
  auto& x = self.dependencies()[0];
  auto& y = self.dependencies()[1];
  return createNode<DDSquareNode>({x, y, x->derive(variable), y->derive(variable)});
}

// Addition
struct AdditionOp : public OperationBase<AdditionOp>
{
  using ArgumentType = double;
  using ResultType = double;
  static void reset(ResultType& r) { r = 0; }
  static void reduce(ResultType& r, const double& x) { r += x; }
  static NodeRef derive(Node& self, const Node& variable);
  static std::string description() { return "+"; }
};
using AdditionNode = GenericReductionComputation<AdditionOp>;

NodeRef AdditionOp::derive(Node& self, const Node& variable)
{
  NodeRefVec derivatives;
  for (auto& subExpr : self.dependencies())
    derivatives.emplace_back(subExpr->derive(variable));
  return createNode<AdditionNode>(std::move(derivatives));
}

// TODO define a bpp::Function to represent a DF::Value<double>
// +manually set its ParameterList of DFParameter

TEST_CASE("derive constant")
{
  auto konst = createNode<Constant<double>>(42.0);
  CHECK(getUpToDateValue(konst) == 42.0);
  CHECK(konst->isConstant());

  auto dummy = createNode<Parameter<double>>(0);
  auto derived = convertRef<Value<double>>(konst->derive(*dummy));
  CHECK(derived->isConstant());
  CHECK(getUpToDateValue(derived) == 0);
}

TEST_CASE("derive parameter")
{
  auto x = createNode<Parameter<double>>(42.0);
  auto dummy = createNode<Parameter<double>>(3);

  auto dx_dx = convertRef<Value<double>>(x->derive(*x));
  CHECK(dx_dx->isConstant());
  CHECK(getUpToDateValue(dx_dx) == 1.0);

  auto dx_dummy = convertRef<Value<double>>(x->derive(*dummy));
  CHECK(dx_dummy->isConstant());
  CHECK(getUpToDateValue(dx_dummy) == 0.0);
}

#include <Bpp/Numeric/AutoParameter.h>
#include <Bpp/Numeric/Function/ConjugateGradientMultiDimensions.h>
#include <Bpp/Numeric/Function/Optimizer.h>
#include <fstream>
#include <iostream>

TEST_CASE("test")
{
  // x^2 + (y - 3)^2
  bpp::DataFlowParameter xp{"x", 2.0};
  bpp::DataFlowParameter yp{"y", -3.0};

  auto& x = xp.getDataFlowParameter();
  auto& y = yp.getDataFlowParameter();
  auto x2 = createNode<SquareNode>({x});
  auto konst3 = createNode<Constant<double>> (-3.);
  auto sy = createNode<AdditionNode> ({y, konst3});
  auto y2 = createNode<SquareNode>({sy});
  auto f = createNode<AdditionNode>({x2, y2});

#if 0
  std::cout << "x2 + y2 = " << getUpToDateValue(f) << "\n";
  auto df_dx = f->derive(*x);
  auto df2_dx2 = df_dx->derive(*x);
  std::cout << "d2(x2 + y2)/dx2 = " << getUpToDateValue(convertRef<Value<double>>(df2_dx2)) << "\n";
  std::ofstream fd("df_debug");
  debugDag(fd, df2_dx2, DebugOptions::ShowDependencyIndex | DebugOptions::FollowUpwardLinks);
#endif

  bpp::ParameterList params;
  params.addParameter(xp);
  params.addParameter(yp);
  bpp::DataFlowFunction dfFunc{f, params};

  bpp::ConjugateGradientMultiDimensions optimizer(&dfFunc);
  optimizer.setVerbose(1);
  optimizer.setProfiler(bpp::ApplicationTools::message);
  optimizer.setMessageHandler(bpp::ApplicationTools::message);
  optimizer.setMaximumNumberOfEvaluations(1000000);
  optimizer.getStopCondition()->setTolerance(0.000001);
  optimizer.setConstraintPolicy(bpp::AutoParameter::CONSTRAINTS_AUTO);
  optimizer.init(dfFunc.getParameters());
  optimizer.optimize();

  std::cout << "(x, y) == (" << xp.getValue() << ", " << yp.getValue() << ")\n";
}