//
// File: Likelihood.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-05-03
// Last modified: 2017-05-03
//

/*
  Copyright or © or Copr. Bio++ Development Team, (November 16, 2004)

  This software is a computer program whose purpose is to provide classes
  for phylogenetic data analysis.

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

#pragma once
#ifndef BPP_NEWPHYL_LIKELIHOOD_H
#define BPP_NEWPHYL_LIKELIHOOD_H

#include <Bpp/NewPhyl/DataFlow.h>
#include <Bpp/NewPhyl/DataFlowTemplates.h>
#include <Bpp/NewPhyl/Model.h>
#include <Bpp/NewPhyl/PackedVector.h>
#include <Eigen/Core>
#include <string> // description

namespace bpp {

// Forward declare
class Sequence;

namespace Phyl {

	// TODO small classes with nice constructor arguments (like nbStates / nbSite)
	using LikelihoodVector = Eigen::VectorXd;
	using LikelihoodVectorBySite = PackedVector<LikelihoodVector>;

	struct ComputeConditionalLikelihoodFromDataOp
	    : public DF::OperationBase<ComputeConditionalLikelihoodFromDataOp> {
		using ResultType = LikelihoodVectorBySite;
		using ArgumentTypes = std::tuple<const Sequence *>;
		static void compute (LikelihoodVectorBySite & condLikBySite, const Sequence * sequence);
		static std::string description () { return "CondLikFromData"; }
	};
	using ComputeConditionalLikelihoodFromDataNode =
	    DF::GenericFunctionComputation<ComputeConditionalLikelihoodFromDataOp>;

	struct ComputeConditionalLikelihoodFromChildrensOp
	    : public DF::OperationBase<ComputeConditionalLikelihoodFromChildrensOp> {
		using ResultType = LikelihoodVectorBySite;
		using ArgumentType = LikelihoodVectorBySite;
		static void reset (LikelihoodVectorBySite & condLikBySite);
		static void reduce (LikelihoodVectorBySite & condLikBySite,
		                    const LikelihoodVectorBySite & fwdLikBySite);
		static std::string description () { return "CondLikFromChildrens"; }
	};
	using ComputeConditionalLikelihoodFromChildrensNode =
	    DF::GenericReductionComputation<ComputeConditionalLikelihoodFromChildrensOp>;

	struct ComputeForwardLikelihoodOp : public DF::OperationBase<ComputeForwardLikelihoodOp> {
		using ResultType = LikelihoodVectorBySite;
		using ArgumentTypes = std::tuple<LikelihoodVectorBySite, TransitionMatrix>;
		static void compute (LikelihoodVectorBySite & fwdLikBySite,
		                     const LikelihoodVectorBySite & condLikBySite,
		                     const TransitionMatrix & transitionMatrix);
		static std::string description () { return "FwdLik"; }
	};
	using ComputeForwardLikelihoodNode = DF::GenericFunctionComputation<ComputeForwardLikelihoodOp>;

	struct ComputeLogLikelihoodOp : public DF::OperationBase<ComputeLogLikelihoodOp> {
		using ResultType = double;
		using ArgumentTypes = std::tuple<LikelihoodVectorBySite, FrequencyVector>;
		static void compute (double & logLikelihood, const LikelihoodVectorBySite & condLikBySite,
		                     const FrequencyVector & equilibriumFreqs);
		static std::string description () { return "LogLikFromCondLik"; }
	};
	using ComputeLogLikelihoodNode = DF::GenericFunctionComputation<ComputeLogLikelihoodOp>;
}
}

#endif // BPP_NEWPHYL_LIKELIHOOD_H