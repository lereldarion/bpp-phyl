//
// File: Registry.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-04-25
// Last modified: 2017-04-25
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
#ifndef BPP_NEWPHYL_REGISTRY_H
#define BPP_NEWPHYL_REGISTRY_H

#include <Bpp/NewPhyl/DataFlow.h>
#include <Bpp/NewPhyl/Debug.h>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace bpp {
namespace DF {
/*
	class Registry {
	public:


		bool trySetNode (const Key & key, Node n) {
			auto result = dataflowNodes_.emplace (key, std::move (n));
			return result.second;
		}
		void setNode (const Key & key, Node n) {
			if (!trySetNode (key, std::move (n)))
				throw std::runtime_error ("Node already set for key");
		}

		Node instantiate (const NodeSpecification & key) {
			// Use already built node
			auto currentNode = dataflowNodes_.find (key);
			if (currentNode != dataflowNodes_.end ())
				return currentNode->second;
			// Instantiate dependencies
			auto & factory = Builder::functions (key.operation ());
			std::vector<Node> deps;
			for (auto & depKey : factory.computeDependencies (key))
				deps.emplace_back (instantiate (depKey));
			// Build node
			auto node = factory.buildNode (std::move (deps));
			dataflowNodes_.emplace (key, node);
			return node;
		}

		const std::unordered_map<NodeSpecification, Node, NodeSpecification::Hash> &
		rawAccess () const {
			return dataflowNodes_;
		}

	private:
		std::unordered_map<NodeSpecification, Node, NodeSpecification::Hash> dataflowNodes_;
	};
  */

  //TODO convert to a pure DF registry
}
}

#endif // BPP_NEWPHYL_REGISTRY_H