//
// File: NodeSpecification.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-05-15
// Last modified: 2017-05-15
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
#ifndef BPP_NEWPHYL_NODESPECIFICATION_H
#define BPP_NEWPHYL_NODESPECIFICATION_H

#include <Bpp/NewPhyl/DataFlow.h>
#include <Bpp/NewPhyl/Debug.h>
#include <Bpp/NewPhyl/Optional.h>
#include <Bpp/NewPhyl/Vector.h>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace bpp {
namespace DF {

	class Registry {
	public:
		class Key {
			/* Nodes are indexed by node type and dependencies.
			 * This is sufficient to ensure merging of DF nodes with similar values.
			 *
			 * Key is a temporary class (stores a ref to the dependency vector).
			 * When searching, the ref should point to the built dep vector.
			 * When stored, the ref points to the Node dep vector.
			 * The Key will be destroyed at the same time as the Node.
			 */
		public:
			Key (std::type_index nodeType, const NodeRefVec & dependencies)
			    : nodeType_ (nodeType), dependencies_ (dependencies) {}

			bool operator== (const Key & other) const noexcept {
				return nodeType_ == other.nodeType_ && dependencies_ == other.dependencies_;
			}
			std::size_t hashCode () const noexcept {
				auto nodeTypeHash = std::hash<std::type_index>{}(nodeType_);
				auto vecHash = std::hash<NodeRefVec>{}(dependencies_);
				return vecHash ^ (nodeTypeHash << 1);
			}

			std::type_index operation () const noexcept { return nodeType_; }
			const NodeRefVec & dependencies () const noexcept { return dependencies_; }

		private:
			std::type_index nodeType_;
			const NodeRefVec & dependencies_;
		};

		Optional<NodeRef> get (const Key & key) const {
			return optional_find (nodes_, key).cast<NodeRef> ();
		}

		void set (NodeRef nodeRef) {
			// Node is independent of NodeRef moves, so we can build a key referencing its dependencies.
			auto & node = *nodeRef;
			// By definition of the standard, typeid (node) performs lookup to derived type.
			auto result = nodes_.emplace (Key{typeid (node), node.dependencies ()}, std::move (nodeRef));
			if (!result.second)
				throw std::runtime_error ("Registry::set: key already used");
		}

		template <typename Callable> void foreachKeyValue (Callable callable) const {
			for (auto & it : nodes_)
				callable (it.first, it.second);
		}

	private:
		struct Hash {
			std::size_t operator() (const Key & k) const noexcept { return k.hashCode (); }
		};
		std::unordered_map<Key, NodeRef, Hash> nodes_;
	};

	/* Node specifications are structs that describe a phylogenetic value in a context.
	 * They are not template, but must follow the following interface :
	 * TODO required interface doc when stabilised
	 *
	 * NodeSpecification is a type that wraps any NodeSpec compatible type in a virtual hierarchy.
	 * This allow instantiation functions to be type independent, at the cost of memory allocations.
	 * TODO use duck::SmallUniqPtr is perf is too critical
	 */

	class NodeSpecification {
	public:
		// Constructors
		NodeSpecification () = delete;
		NodeSpecification (const NodeSpecification & other)
		    : specification_ (other.specification_->clone ()) {}
		NodeSpecification (NodeSpecification &&) = default;
		NodeSpecification & operator= (NodeSpecification &&) = default;
		NodeSpecification & operator= (const NodeSpecification & other) {
			return *this = NodeSpecification (other);
		}
		~NodeSpecification () = default;

		// TODO externalize for clarity
		template <typename T, typename Decayed = typename std::decay<T>::type,
		          typename = typename std::enable_if<
		              !std::is_base_of<NodeSpecification, Decayed>::value>::type>
		NodeSpecification (T && spec)
		    : specification_ (new Specification<Decayed> (std::forward<T> (spec))) {}

		// Wrappers
		Vector<NodeSpecification> computeDependencies () const {
			return specification_->computeDependencies ();
		}
		NodeRef buildNode (NodeRefVec dependencies) const {
			return specification_->buildNode (std::move (dependencies));
		}
		std::type_index nodeType () const noexcept { return specification_->nodeType (); }
		std::string description () const { return specification_->description (); }

		// Debug: generate an id that is probably unique (only use for debug !)
		std::size_t debugHashCode () const noexcept {
			auto ptr_hash = std::hash<std::unique_ptr<Interface>>{}(specification_);
			auto descr_hash = std::hash<std::string>{}(description ());
			return ptr_hash ^ (descr_hash << 1);
		}

	private:
		// Virtual value type pattern
		struct Interface {
			virtual ~Interface () = default;
			virtual Interface * clone () const = 0;
			virtual Vector<NodeSpecification> computeDependencies () const = 0;
			virtual NodeRef buildNode (NodeRefVec dependencies) const = 0;
			virtual std::type_index nodeType () const noexcept = 0;
			virtual std::string description () const = 0;
		};
		template <typename T> struct Specification final : public Interface {
			T spec_;
			Specification (const T & spec) : spec_ (spec) {}
			Specification (T && spec) : spec_ (std::move (spec)) {}
			Specification * clone () const override { return new Specification (*this); }
			Vector<NodeSpecification> computeDependencies () const override {
				return spec_.computeDependencies ();
			}
			NodeRef buildNode (NodeRefVec dependencies) const override {
				return spec_.buildNode (std::move (dependencies));
			}
			std::type_index nodeType () const noexcept override { return spec_.nodeType (); }
			std::string description () const override { return spec_.description (); }
		};
		std::unique_ptr<Interface> specification_;
	};

	// Build DF graph recursively without merging
	NodeRef instantiateNodeSpec (const NodeSpecification & nodeSpec);

	// Build DF graph while merging using the given registry
	NodeRef instantiateNodeSpecWithReuse (const NodeSpecification & nodeSpec, Registry & registry);

	// Convenience typedef / functions.
	using NodeSpecificationVec = Vector<NodeSpecification>;
	template <typename... Args> NodeSpecificationVec makeNodeSpecVec (Args &&... args) {
		return {NodeSpecification{std::forward<Args> (args)}...};
	}

	/* Defines all but computeDependencies () for a Spec that always generates the same node type.
	 */
	template <typename NodeType> struct NodeSpecAlwaysGenerate {
		static NodeRef buildNode (NodeRefVec deps) { return createNode<NodeType> (std::move (deps)); }
		static std::type_index nodeType () { return typeid (NodeType); }
		static std::string description () { return prettyTypeName<NodeSpecAlwaysGenerate> (); }
	};

	/* A dummy node spec that returns a stored node.
	 * nodeType () returns a dummy value, but is never used.
	 */
	class NodeSpecReturnParameter {
	public:
		NodeSpecReturnParameter (NodeRef n) : node_ (std::move (n)) {}
		static NodeSpecificationVec computeDependencies () { return {}; }
		NodeRef buildNode (NodeRefVec) const { return node_; }
		static std::type_index nodeType () { return typeid (void); }
		std::string description () const {
			return std::string ("Parameter(") + node_->description () + ")";
		}

	private:
		NodeRef node_;
	};
}
}

#endif // BPP_NEWPHYL_NODESPECIFICATION_H