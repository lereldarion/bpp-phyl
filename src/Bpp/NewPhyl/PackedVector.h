//
// File: PackedVector.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-06-30
// Last modified: 2017-06-30
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

#ifndef BPP_NEWPHYL_PACKEDVECTOR_H
#define BPP_NEWPHYL_PACKEDVECTOR_H

#include <Bpp/NewPhyl/Signed.h>
#include <Bpp/NewPhyl/Vector.h>
#include <Eigen/Core>
#include <cassert>

namespace bpp {
template <typename T> class PackedVector : public Vector<T> {};

template <typename T> class PackedVector<Eigen::Matrix<T, Eigen::Dynamic, 1>> {
	// Specialisation for Vectors : pack all in a Matrix
	// Lose iterators
private:
	using Container = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

public:
	using size_type = SizeType;
	using reference = decltype (std::declval<Container> ().col (0));
	using const_reference = decltype (std::declval<const Container> ().col (0));

	PackedVector (size_type size, Eigen::Index vectorSize) : matrix_ (vectorSize, size) {}

	reference operator[] (size_type i) {
		assert (0 <= i);
		assert (i < size ());
		return matrix_.col (static_cast<typename Eigen::Index> (i));
	}
	const_reference operator[] (size_type i) const {
		assert (0 <= i);
		assert (i < size ());
		return matrix_.col (static_cast<typename Eigen::Index> (i));
	}

	size_type size () const noexcept { return static_cast<size_type> (matrix_.cols ()); }

	Container & asMatrix () noexcept { return matrix_; }
	const Container & asMatrix () const noexcept { return matrix_; }

private:
	// Vectors are stored in each column (as by default, Eigen::Matrix is ColMajor)
	Container matrix_;
};
}

#endif // BPP_NEWPHYL_PACKEDVECTOR_H