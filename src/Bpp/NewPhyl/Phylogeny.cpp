//
// File: Phylogeny.cpp
// Authors:
// Created: 2017-06-06
// Last modified: 2017-06-06
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

#include <Bpp/NewPhyl/Debug.h>
#include <Bpp/NewPhyl/Likelihood.h>
#include <Bpp/NewPhyl/Model.h>
#include <Bpp/NewPhyl/Phylogeny.h>
#include <utility>

namespace bpp {
namespace Phyl {
	// ConditionalLikelihoodSpec

	bool ConditionalLikelihoodSpec::computed_from_data () const {
		return node.nbChildBranches () == 0;
	}
	DF::NodeSpecificationVec ConditionalLikelihoodSpec::computeDependencies () const {
		if (computed_from_data ()) {
			return DF::makeNodeSpecVec (
			    DF::NodeSpecReturnParameter{likParams.leafData.sequences->access (node).value ()});
		} else {
			DF::NodeSpecificationVec depSpecs;
			node.foreachChildBranch ([this, &depSpecs](Topology::Branch && branch) {
				depSpecs.emplace_back (ForwardLikelihoodSpec{likParams, branch});
			});
			return depSpecs;
		}
	}
	DF::NodeRef ConditionalLikelihoodSpec::buildNode (DF::NodeRefVec deps) const {
		if (computed_from_data ())
			return DF::createNode<ComputeConditionalLikelihoodFromDataNode> (
			    std::move (deps), likParams.leafData.nbSites, likParams.process.nbStates);
		else
			return DF::createNode<ComputeConditionalLikelihoodFromChildrensNode> (
			    std::move (deps), likParams.leafData.nbSites, likParams.process.nbStates);
	}
	std::type_index ConditionalLikelihoodSpec::nodeType () const {
		return computed_from_data () ? typeid (ComputeConditionalLikelihoodFromDataNode)
		                             : typeid (ComputeConditionalLikelihoodFromChildrensNode);
	}
	std::string ConditionalLikelihoodSpec::description () {
		return prettyTypeName<ConditionalLikelihoodSpec> ();
	}

	// ForwardLikelihoodSpec

	DF::NodeSpecificationVec ForwardLikelihoodSpec::computeDependencies () const {
		return DF::makeNodeSpecVec (
		    ConditionalLikelihoodSpec{likParams, branch.childNode ()},
		    ModelTransitionMatrixSpec (likParams.process.modelByBranch->access (branch).value (),
		                               likParams.process.branchLengths->access (branch).value (),
		                               likParams.process.nbStates));
	}
	DF::NodeRef ForwardLikelihoodSpec::buildNode (DF::NodeRefVec deps) const {
		return DF::createNode<ComputeForwardLikelihoodNode> (
		    std::move (deps), likParams.leafData.nbSites, likParams.process.nbStates);
	}
	std::type_index ForwardLikelihoodSpec::nodeType () {
		return typeid (ComputeForwardLikelihoodNode);
	}
	std::string ForwardLikelihoodSpec::description () {
		return prettyTypeName<ForwardLikelihoodSpec> ();
	}

	// LogLikelihoodSpec

	DF::NodeSpecificationVec LogLikelihoodSpec::computeDependencies () const {
		return DF::makeNodeSpecVec (
		    ConditionalLikelihoodSpec{likParams, likParams.process.tree->rootNode ()},
		    ModelEquilibriumFrequenciesSpec (
		        likParams.process.modelByBranch
		            ->access (likParams.process.tree->rootNode ().fatherBranch ())
		            .value (),
		        likParams.process.nbStates));
	}
	DF::NodeRef LogLikelihoodSpec::buildNode (DF::NodeRefVec deps) {
		return DF::createNode<ComputeLogLikelihoodNode> (std::move (deps));
	}
	std::type_index LogLikelihoodSpec::nodeType () { return typeid (ComputeLogLikelihoodNode); }
	std::string LogLikelihoodSpec::description () { return prettyTypeName<LogLikelihoodSpec> (); }
}
}