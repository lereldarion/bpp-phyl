//
// File: Vector.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-07-04 00:00:00
// Last modified: 2017-07-04
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

#ifndef BPP_NEWPHYL_VECTOR_H
#define BPP_NEWPHYL_VECTOR_H

#include <Bpp/NewPhyl/Signed.h>
#include <cassert>
#include <functional> // std::hash
#include <utility>
#include <vector>

namespace bpp {
/** std::vector equivalent with signed integer API.
 * Most uses of std::size_t come from interacting with std::vector.
 * Mixing unsigned and signed integer is dangerous (implicit conversions of -1 to UINT_MAX).
 * This class implements most of std::vector API, with all unsigned ints converted to signed.
 *
 * assert() has been used to ensure positive values for arguments (in debug mode).
 * emplace_back () returns a reference to the created element (like in C++17).
 * push_back () has not been implemented, use emplace_back instead.
 * Other missing methods can be added if needed.
 *
 * The inner std::vector can be accessed with asVector() for compatibility.
 */
template <typename T> class Vector {
private:
	using Container = std::vector<T>;

public:
	using size_type = SizeType;
	using reference = typename Container::reference;
	using const_reference = typename Container::const_reference;
	using iterator = typename Container::iterator;
	using const_iterator = typename Container::const_iterator;

	Vector () : vec_ () {}
	explicit Vector (size_type size_) : vec_ (static_cast<typename Container::size_type> (size_)) {}
	Vector (std::initializer_list<T> ilist) : vec_ (ilist) {}

	reference at (size_type i) {
		assert (0 <= i);
		return vec_.at (static_cast<typename Container::size_type> (i));
	}
	const_reference at (size_type i) const {
		assert (0 <= i);
		return vec_.at (static_cast<typename Container::size_type> (i));
	}

	reference operator[] (size_type i) {
		assert (0 <= i);
		assert (i < size ());
		return vec_[static_cast<typename Container::size_type> (i)];
	}
	const_reference operator[] (size_type i) const {
		assert (0 <= i);
		assert (i < size ());
		return vec_[static_cast<typename Container::size_type> (i)];
	}

	reference front () { return vec_.front (); }
	const_reference front () const { return vec_.front (); }
	reference back () { return vec_.back (); }
	const_reference back () const { return vec_.back (); }

	T * data () noexcept { return vec_.data (); }
	const T * data () const noexcept { return vec_.data (); }

	iterator begin () noexcept { return vec_.begin (); }
	const_iterator begin () const noexcept { return vec_.begin (); }
	iterator end () noexcept { return vec_.end (); }
	const_iterator end () const noexcept { return vec_.end (); }

	bool empty () const noexcept { return vec_.empty (); }
	size_type size () const noexcept { return static_cast<size_type> (vec_.size ()); }

	void clear () noexcept { vec_.clear (); }
	template <typename... Args> reference emplace_back (Args &&... args) {
		vec_.emplace_back (std::forward<Args> (args)...);
		return back ();
	}
	void reserve (size_type size_) {
		vec_.reserve (static_cast<typename Container::size_type> (size_));
	}
	void resize (size_type size_) {
		vec_.resize (static_cast<typename Container::size_type> (size_));
	}

	iterator erase (iterator first, iterator last) { return vec_.erase (first, last); }

	Container & asVector () noexcept { return vec_; }
	const Container & asVector () const noexcept { return vec_; }

private:
	Container vec_;
};

/// Enables == comparison. Others can be added if needed.
template <typename T> bool operator== (const Vector<T> & lhs, const Vector<T> & rhs) {
	return lhs.asVector () == rhs.asVector ();
}

/** Create a new vector filled with results from calling a function on another vector.
 * The type of the new vector is std::vector<T> for T the result type of the function.
 */
template <typename Container, typename Function>
auto mapToVector (const Container & container, Function function)
    -> Vector<decltype (function (container.front ()))> {
	Vector<decltype (function (container.front ()))> r;
	r.reserve (static_cast<SizeType> (container.size ()));
	for (const auto & v : container)
		r.emplace_back (function (v));
	return r;
}
} // namespace bpp

namespace std {
/** hash capability for bpp::Vector<T>.
 * bpp::Vector<T> can be used as a key for hash tables (std::unordered_map / set).
 * The vector itself must be const after insertion in the table.
 * If not, the key will change, which breaks the hash table invariants.
 */
template <typename T> struct hash<bpp::Vector<T>> {
	size_t operator() (const bpp::Vector<T> & vec) const {
		auto h = static_cast<size_t> (vec.size ());
		std::hash<T> hasher{};
		for (const auto & e : vec)
			h ^= hasher (e) + 0x9e3779b9 + (h << 6) + (h >> 2);
		return h;
	}
};
} // namespace std

#endif // BPP_NEWPHYL_VECTOR_H