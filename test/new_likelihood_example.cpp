//
// File: new_phyl_dataflow.cpp
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-04-19 00:00:00
// Last modified: 2017-06-08
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

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#define ENABLE_OLD
#define ENABLE_NEW

// Common stuff
#include <Bpp/Numeric/AutoParameter.h>
#include <Bpp/Numeric/Function/ConjugateGradientMultiDimensions.h>
#include <Bpp/Numeric/Function/Optimizer.h>
#include <Bpp/Numeric/Function/SimpleNewtonMultiDimensions.h>
#include <Bpp/Numeric/Parameter.h>
#include <Bpp/Numeric/ParameterList.h>
#include <Bpp/Phyl/Model/Nucleotide/T92.h>
#include <Bpp/Seq/Alphabet/AlphabetTools.h>
#include <Bpp/Seq/Container/VectorSiteContainer.h>
#include <chrono>

// Old likelihood
#ifdef ENABLE_OLD
#include <Bpp/Numeric/Prob/ConstantDistribution.h>
#include <Bpp/Phyl/Likelihood/RHomogeneousTreeLikelihood.h>
#include <Bpp/Phyl/Tree/TreeTemplate.h>
#endif
// Newlik
#ifdef ENABLE_NEW
#include <Bpp/Phyl/Io/Newick.h>
#include <Bpp/Phyl/NewLikelihood/PhyloLikelihoods/SingleProcessPhyloLikelihood.h>
#include <Bpp/Phyl/NewLikelihood/SimpleSubstitutionProcess.h>
#endif
// DF
#include <Bpp/NewPhyl/LikelihoodExample.h>
#include <Bpp/Phyl/Io/Newick.h>
#include <Bpp/Phyl/NewLikelihood/PhyloLikelihoods/SingleProcessPhyloLikelihood.h>

static bool enableDotOutput = false;

static void dotOutput(const std::string& testName, const std::vector<const bpp::dataflow::Node*>& nodes)
{
  if (enableDotOutput)
  {
    using bpp::dataflow::DotOptions;
    writeGraphToDot(
      "debug_" + testName + ".dot", nodes, DotOptions::DetailedNodeInfo | DotOptions::ShowDependencyIndex);
  }
}

namespace
{
  using TimePoint = typename std::chrono::high_resolution_clock::time_point;
  TimePoint timingStart(void) { return std::chrono::high_resolution_clock::now(); }
  void timingEnd(TimePoint start, const std::string& prefix)
  {
    auto end = timingStart(); // ill named, just to get now()
    std::cout << "[time-ns] " << prefix << " "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << "\n";
  }
  void printLik(double logLik, const std::string& prefix)
  {
    std::cout << "[log-lik] " << prefix << " " << logLik << "\n";
  }

  template<typename Func>
  void do_func_multiple_times(const std::string& timePrefix, Func f)
  {
    constexpr std::size_t updatesNbIterations = 1000;
    auto ts = timingStart();
    for (std::size_t i = 0; i < updatesNbIterations; ++i)
      f();
    timingEnd(ts, timePrefix);
  }
  void do_param_changes_multiple_times(bpp::DerivableSecondOrder& llh,
                                       const std::string& timePrefix,
                                       const bpp::ParameterList& p1,
                                       const bpp::ParameterList& p2)
  {
    llh.matchParametersValues(p1);
    printLik(llh.getValue(), timePrefix);
    llh.matchParametersValues(p2);
    printLik(llh.getValue(), timePrefix);

    do_func_multiple_times(timePrefix, [&]() {
      llh.matchParametersValues(p1);
      llh.getValue();
      llh.matchParametersValues(p2);
      llh.getValue();
    });
  }

  void optimize_for_params(bpp::DerivableSecondOrder& llh,
                           const std::string& prefix,
                           const bpp::ParameterList& branchParams)
  {
    auto ts = timingStart();
    bpp::ConjugateGradientMultiDimensions optimizer(&llh);
    // bpp::SimpleNewtonMultiDimensions optimizer(&llh);
    optimizer.setVerbose(0);
    optimizer.setProfiler(nullptr);       // bpp::ApplicationTools::message);
    optimizer.setMessageHandler(nullptr); // bpp::ApplicationTools::message);
    optimizer.setMaximumNumberOfEvaluations(100);
    optimizer.getStopCondition()->setTolerance(0.000001);
    optimizer.setConstraintPolicy(bpp::AutoParameter::CONSTRAINTS_AUTO);
    optimizer.init(branchParams);
    optimizer.optimize();
    timingEnd(ts, prefix);
    printLik(llh.getValue(), prefix);
  }

  struct CommonStuff
  {
    const bpp::NucleicAlphabet& alphabet;
    bpp::VectorSiteContainer sites;
    const char* treeStr;
    bpp::ParameterList paramModel1;
    bpp::ParameterList paramModel2;
    bpp::ParameterList paramBrLen1;
    bpp::ParameterList paramBrLen2;

    CommonStuff()
      : alphabet(bpp::AlphabetTools::DNA_ALPHABET)
      , sites(&alphabet)
      , treeStr("((A:0.01, B:0.02):0.03,C:0.01,D:0.1);")
    {
      // Init sequences
      sites.addSequence(
        bpp::BasicSequence("A", "ATCCAGACATGCCGGGACTTTGCAGAGAAGGAGTTGTTTCCCATTGCAGCCCAGGTGGATAAGGAACAGC", &alphabet));
      sites.addSequence(
        bpp::BasicSequence("B", "CGTCAGACATGCCGTGACTTTGCCGAGAAGGAGTTGGTCCCCATTGCGGCCCAGCTGGACAGGGAGCATC", &alphabet));
      sites.addSequence(
        bpp::BasicSequence("C", "GGTCAGACATGCCGGGAATTTGCTGAAAAGGAGCTGGTTCCCATTGCAGCCCAGGTAGACAAGGAGCATC", &alphabet));
      sites.addSequence(
        bpp::BasicSequence("D", "TTCCAGACATGCCGGGACTTTACCGAGAAGGAGTTGTTTTCCATTGCAGCCCAGGTGGATAAGGAACATC", &alphabet));

      // Set of parameters to apply to tree + model
      paramModel1.addParameter(bpp::Parameter("T92.kappa", 0.1));
      paramModel2.addParameter(bpp::Parameter("T92.kappa", 0.2));
      paramBrLen1.addParameter(bpp::Parameter("BrLen1", 0.1));
      paramBrLen2.addParameter(bpp::Parameter("BrLen1", 0.2));
    }
  };
}

#ifdef ENABLE_OLD
TEST_CASE("old")
{
  const CommonStuff c;
  auto ts = timingStart();
  auto model = new bpp::T92(&c.alphabet, 3.);
  auto distribution = new bpp::ConstantDistribution(1.0);
  auto tree = std::unique_ptr<bpp::TreeTemplate<bpp::Node>>(bpp::TreeTemplateTools::parenthesisToTree(c.treeStr));
  bpp::RHomogeneousTreeLikelihood llh(*tree, c.sites, model, distribution, false, false);
  timingEnd(ts, "old_setup");
  ts = timingStart();
  llh.initialize();
  auto logLik = llh.getValue();
  timingEnd(ts, "old_init_value");
  printLik(logLik, "old_init_value");

  std::cout << "[dbrlen1] " << llh.getFirstOrderDerivative("BrLen1") << "\n";
  do_param_changes_multiple_times(llh, "old_param_model_change", c.paramModel1, c.paramModel2);
  do_param_changes_multiple_times(llh, "old_param_brlen_change", c.paramBrLen1, c.paramBrLen2);
  optimize_for_params(llh, "old_brlens_opt", llh.getBranchLengthsParameters());
}
#endif

#ifdef ENABLE_NEW
TEST_CASE("new")
{
  const CommonStuff c;
  auto ts = timingStart();
  auto model = new bpp::T92(&c.alphabet, 3.);
  bpp::Newick reader;
  auto phyloTree = std::unique_ptr<bpp::PhyloTree>(reader.parenthesisToPhyloTree(c.treeStr, false, "", false, false));
  auto paramPhyloTree = new bpp::ParametrizablePhyloTree(*phyloTree);
  auto process =
    std::unique_ptr<bpp::SimpleSubstitutionProcess>(new bpp::SimpleSubstitutionProcess(model, paramPhyloTree));
  auto likelihoodCompStruct = std::unique_ptr<bpp::RecursiveLikelihoodTreeCalculation>(
    new bpp::RecursiveLikelihoodTreeCalculation(c.sites, process.get(), false, true));
  bpp::SingleProcessPhyloLikelihood llh(process.get(), likelihoodCompStruct.release());
  timingEnd(ts, "new_setup");
  ts = timingStart();
  llh.computeLikelihood();
  auto logLik = llh.getValue();
  timingEnd(ts, "new_init_value");
  printLik(logLik, "new_init_value");

  std::cout << "[dbrlen1] " << llh.getFirstOrderDerivative("BrLen1") << "\n";
  do_param_changes_multiple_times(llh, "new_param_model_change", c.paramModel1, c.paramModel2);
  do_param_changes_multiple_times(llh, "new_param_brlen_change", c.paramBrLen1, c.paramBrLen2);
  optimize_for_params(llh, "new_brlens_opt", llh.getBranchLengthParameters());
}
#endif

TEST_CASE("df")
{
  const CommonStuff c;
  auto ts = timingStart();
  // Read tree structure
  bpp::Newick reader;
  auto phyloTree = std::unique_ptr<bpp::PhyloTree>(reader.parenthesisToPhyloTree(c.treeStr, false, "", false, false));

  bpp::dataflow::Context context;

  // Model: create simple leaf nodes as model parameters
  auto model = std::unique_ptr<bpp::T92>(new bpp::T92(&c.alphabet, 3.));
  auto modelParameters = bpp::dataflow::createParameterMapForModel(context, *model);
  auto modelNode = bpp::dataflow::ConfiguredModel::create(
    context,
    bpp::dataflow::createDependencyVector(
      *model, [&modelParameters](const std::string& paramName) { return modelParameters[paramName]; }),
    std::move(model));

  // Build likelihood value node
  auto l = makeSimpleLikelihoodNodes(context, *phyloTree, c.sites, modelNode);

  // Assemble a bpp::Optimizer compatible interface to SimpleLikelihoodNodes.
  bpp::ParameterList brlenOnlyParameters;
  for (const auto& p : l.branchLengthValues)
  {
    auto param = bpp::DataFlowParameter("BrLen" + std::to_string(p.first), p.second);
    param.setConstraint(bpp::Parameter::R_PLUS.clone(), true);
    brlenOnlyParameters.addParameter(std::move(param));
  }
  bpp::ParameterList allParameters;
  allParameters.addParameters(brlenOnlyParameters);
  for (const auto& p : modelParameters)
  {
    allParameters.addParameter(bpp::DataFlowParameter("T92." + p.first, p.second));
  }

  bpp::DataFlowFunction llh(context, l.totalLogLikelihood, allParameters);
  timingEnd(ts, "df_setup");

  ts = timingStart();
  auto logLik = llh.getValue();
  timingEnd(ts, "df_init_value");
  printLik(logLik, "df_init_value");
  dotOutput("likelihood_example_value", {l.totalLogLikelihood.get()});

  // Manual access to dbrlen1
  auto dlogLik_dbrlen1 = l.totalLogLikelihood->deriveAsValue(context, *l.branchLengthValues[1]);
  std::cout << "[dbrlen1] " << dlogLik_dbrlen1->getValue() << "\n";
  dotOutput("likelihood_example_dbrlen1", {dlogLik_dbrlen1.get()});

  do_param_changes_multiple_times(llh, "df_param_model_change", c.paramModel1, c.paramModel2);
  do_param_changes_multiple_times(llh, "df_param_brlen_change", c.paramBrLen1, c.paramBrLen2);
  optimize_for_params(llh, "df_brlens_opt", brlenOnlyParameters);
  // TODO test optimization with model params
}

int main(int argc, char** argv)
{
  const std::string keyword = "dot_output";
  for (int i = 1; i < argc; ++i)
  {
    if (argv[i] == keyword)
    {
      enableDotOutput = true;
    }
  }
  doctest::Context context;
  context.applyCommandLine(argc, argv);
  return context.run();
}
