// File: test_likelihood.cpp
// Authors:
//   Francois Gindraud (2017)
// Created: 23/02/2017

/*
Copyright or © or Copr. Bio++ Development Team, (November 17, 2004)

This software is a computer program whose purpose is to provide classes
for numerical calculus. This file is part of the Bio++ project.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#include <Bpp/Phyl/DF/DataFlowComputationClasses.h>
#include <Bpp/Utils/Cpp14.h>
#include <chrono>
#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

using bpp::Cpp14::make_unique;

TEST_CASE("Testing data flow system on simple int reduction tree")
{
  using namespace bpp::DF;

  // A parameter type
  using IntParam = ParameterNode<int>;

  // Define operations structs
  struct AddIntOp
  {
    using ResultType = int;
    using ArgumentTypes = std::tuple<int, int>;
    static void compute(int& r, int a, int b) { r = a + b; }
  };
  struct NegIntOp
  {
    using ResultType = int;
    using ArgumentTypes = std::tuple<int>;
    static void compute(int& r, int a) { r = -a; }
  };

  // Create nodes out of them
  using AddIntNode = HeterogeneousComputationNode<AddIntOp>;
  using NegIntNode = HeterogeneousComputationNode<NegIntOp>;

  IntParam p1(42), p2(1), p3(0), p4(3);
  AddIntNode n1, n2, n3;
  NegIntNode root;

  /* Build the following DAG:
   * p1__n1__n2__root
   * p2_/   /
   * p3____/__n3
   * p4______/
   */
  n1.setDependency<0>(p1);
  n1.setDependency<1>(p2);
  n2.setDependency<0>(n1);
  n2.setDependency<1>(p3);
  root.setDependency<0>(n2);
  n3.setDependency<0>(p3);
  n3.setDependency<1>(p4);

  // Initial state
  CHECK(p1.isValid());
  CHECK_FALSE(n2.isValid());
  CHECK_FALSE(root.isValid());
  CHECK_FALSE(n3.isValid());

  // Get an intermediate value
  CHECK(n2.getValue() == 43);
  CHECK(n2.isValid());
  CHECK_FALSE(root.isValid());
  CHECK_FALSE(n3.isValid());

  // Get root
  CHECK(root.getValue() == -43);
  CHECK(root.isValid());
  CHECK_FALSE(n3.isValid());

  // Get n3
  CHECK(n3.getValue() == 3);
  CHECK(root.isValid());
  CHECK(n3.isValid());

  // Change p3, check invalidations
  p3.setValue(10);
  CHECK(p3.isValid());
  CHECK_FALSE(root.isValid());
  CHECK_FALSE(n3.isValid());
  CHECK(n1.isValid()); // Not dependent on p3

  // Recompute root
  CHECK(root.getValue() == -53);
  CHECK(root.isValid());
  CHECK_FALSE(n3.isValid());

  // Recompute n3
  CHECK(n3.getValue() == 13);
  CHECK(n3.isValid());
}