//
// File: Debug.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-04-28
// Last modified: 2017-04-28
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
#ifndef BPP_NEWPHYL_DEBUG_H
#define BPP_NEWPHYL_DEBUG_H

#include <iosfwd>
#include <string>

namespace bpp {
// Demangle a C++ symbol name
std::string demangle (const char * name);

namespace Topology {
	class Tree;

	// Output a dot format graph representing the tree
	void debugTree (std::ostream & os, const Tree & tree);
}
namespace DF {
	class Node;
	class Registry;

	// Output a dot format graph representing the dataflow dag
	void debugDag (std::ostream & os, const Node & entryPoint);

	// Outputs debugDag + registry pointers to dataflow dag
	void debugRegistry (std::ostream & os, const Registry & registry);
}
}

#endif // BPP_NEWPHYL_DEBUG_H