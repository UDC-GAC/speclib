/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     SpecAtomic.h
/// \brief    SpecAtomic class wraper to speculate on atomic types
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef __SPECATOMIC_H
#define __SPECATOMIC_H

#include <type_traits>
#include <utility>
#include <ostream>
#include <istream>
#include <atomic>

namespace SpecLib {

template<typename T>
class SpecAtomic {

	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;

	template<typename C, typename=void>
	struct has_fetch_add : std::false_type {};
	template<typename C>
	struct has_fetch_add<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_add((typename C::value_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_sub : std::false_type {};
	template<typename C>
	struct has_fetch_sub<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_sub((typename C::value_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_and : std::false_type {};
	template<typename C>
	struct has_fetch_and<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_and((typename C::value_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_or : std::false_type {};
	template<typename C>
	struct has_fetch_or<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_or((typename C::value_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_xor : std::false_type {};
	template<typename C>
	struct has_fetch_xor<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_xor((typename C::value_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_addptr : std::false_type {};
	template<typename C>
	struct has_fetch_addptr<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_add((typename C::difference_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_subptr : std::false_type {};
	template<typename C>
	struct has_fetch_subptr<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_sub((typename C::difference_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_andptr : std::false_type {};
	template<typename C>
	struct has_fetch_andptr<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_and((typename C::difference_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_orptr : std::false_type {};
	template<typename C>
	struct has_fetch_orptr<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_or((typename C::difference_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	template<typename C, typename=void>
	struct has_fetch_xorptr : std::false_type {};
	template<typename C>
	struct has_fetch_xorptr<C, typename std::enable_if<std::is_convertible<decltype(std::declval<C>().fetch_xor((typename C::difference_type)1, std::memory_order_seq_cst)), typename C::value_type>::value>::type> : std::true_type {};

	std::atomic<T> value;								// The real value is stored here

public:
	using value_type = T;

#if __cplusplus >= 201703L
	static constexpr bool is_always_lock_free = std::atomic<T>::is_always_lock_free;
#endif

	// Overload of cast operator (default)
	////////////////////////////////////
	operator T() const noexcept {						// Default cast operator (T)
		return value.load(std::memory_order_seq_cst);
	}
	operator T() const volatile noexcept {				// Default cast operator (T)
		return value.load(std::memory_order_seq_cst);
	}

	// Constructors and destructor
	////////////////////////////////////

	SpecAtomic() noexcept = default;																	// Default constructor

	explicit SpecAtomic(const T _value, std::memory_order memord = std::memory_order_seq_cst) noexcept	// Default constructor
	{
		value.store(_value, memord);
	}

	~SpecAtomic() noexcept = default;																	// Destructor

	SpecAtomic(const SpecAtomic& other, std::memory_order memord = std::memory_order_seq_cst) noexcept 	// Copy constructor
	{
		value.store(other.value.load(std::memory_order_seq_cst), memord);
	}

	SpecAtomic(SpecAtomic&& other, std::memory_order memord = std::memory_order_seq_cst) noexcept 		// Move constructor
	{
		value.store(other.value.load(std::memory_order_seq_cst), memord);
	}

	// Overload of assignment operators
	////////////////////////////////////

	SpecAtomic& operator=(const SpecAtomic& other) noexcept {											// Copy assignment
		value.store(other.value, std::memory_order_seq_cst);
		return *this;
	}
	SpecAtomic& operator=(const volatile SpecAtomic& other) volatile noexcept {							// Copy assignment
		value.store(other.value, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);
	}
	SpecAtomic& operator=(SpecAtomic&& other) noexcept {												// Move assignment
		value.store(other.value, std::memory_order_seq_cst);
		return *this;
	}
	SpecAtomic& operator=(volatile SpecAtomic&& other) volatile noexcept {								// Move assignment
		value.store(other.value, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);
	}
	template<typename R>
	std::enable_if_t<!std::is_same<expr_type<R>, SpecAtomic>::value, SpecAtomic&>
	operator=(R oval) noexcept {																		// Direct assigment
		value.store(oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename R>
	std::enable_if_t<!std::is_same<expr_type<R>, SpecAtomic>::value, SpecAtomic&>
	operator=(R oval) volatile noexcept {																// Direct assigment
		value.store(oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);
	}
//	template<typename R = T>
//	std::enable_if_t<!std::is_same<expr_type<R>, SpecAtomic>::value, SpecAtomic&>
//	operator=(T&& oval) noexcept {															// Move assignment (not necessary)
//		value.store(std::forward<T>(oval), std::memory_order_seq_cst);
//		return *this;
//	}
//	template<typename R = T>
//	std::enable_if_t<!std::is_same<expr_type<R>, SpecAtomic>::value, SpecAtomic&>
//	operator=(T&& oval) volatile noexcept {													// Move assignment (not necessary)
//		value.store(std::forward<T>(oval), std::memory_order_seq_cst);
//		return const_cast<SpecAtomic&>(*this);
//	}
//	template<typename R = T>
//	std::enable_if_t<!std::is_same<expr_type<R>, SpecAtomic>::value, SpecAtomic&>
//	operator=(const T& oval) noexcept {														// Copy assignment (not necessary)
//		value.store(oval, std::memory_order_seq_cst);
//		return *this;
//	}
//	template<typename R = T>
//	std::enable_if_t<!std::is_same<expr_type<R>, SpecAtomic>::value, SpecAtomic&>
//	operator=(const T& oval) volatile noexcept {											// Copy assignment (not necessary)
//		value.store(oval, std::memory_order_seq_cst);
//		return const_cast<SpecAtomic&>(*this);
//	}
//
//	SpecAtomic& operator=(SpecAtomic obj) {													// Direct assigment (not necessary and creates ambiguity)
//		value.store(obj.value, std::memory_order_seq_cst);
//		return *this;
//	}
//	template<typename P>
//	SpecAtomic& operator=(P oval) {															// Direct assigment (not necessary and creates ambiguity)
//		value.store(oval, std::memory_order_seq_cst);
//		return *this;
//	}

	// Overload of comparison operators
	////////////////////////////////////

	bool operator==(const SpecAtomic& obj) const noexcept {												// == Equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) == obj.value.load(std::memory_order_seq_cst));
	}
	bool operator==(const SpecAtomic& obj) const volatile noexcept {									// == Equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) == obj.value.load(std::memory_order_seq_cst));
	}

//	bool operator==(const T& oval) const noexcept {														// == Equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) == oval);
//	}
//	bool operator==(const T& oval) const volatile noexcept {											// == Equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) == oval);
//	}
//
//	bool operator==(const volatile T& oval) const noexcept {											// == Equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) == oval);
//	}
//	bool operator==(const volatile T& oval) const volatile noexcept {									// == Equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) == oval);
//	}

	bool operator!=(const SpecAtomic& obj) const noexcept {												// != Not-equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) != obj.value.load(std::memory_order_seq_cst));
	}
	bool operator!=(const SpecAtomic& obj) const volatile noexcept {									// != Not-equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) != obj.value.load(std::memory_order_seq_cst));
	}

//	bool operator!=(const T& oval) const noexcept {														// != Not-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) != oval);
//	}
//	bool operator!=(const T& oval) const volatile noexcept {											// != Not-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) != oval);
//	}
//
//	bool operator!=(const volatile T& oval) const noexcept {											// != Not-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) != oval);
//	}
//	bool operator!=(const volatile T& oval) const volatile noexcept {									// != Not-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) != oval);
//	}

	bool operator< (const SpecAtomic& obj) const noexcept {												// < Less-than comparison operator
		return (value.load(std::memory_order_seq_cst) < obj.value.load(std::memory_order_seq_cst));
	}
	bool operator< (const SpecAtomic& obj) const volatile noexcept {									// < Less-than comparison operator
		return (value.load(std::memory_order_seq_cst) < obj.value.load(std::memory_order_seq_cst));
	}

//	bool operator< (const T& oval) const noexcept {														// < Less-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) < oval);
//	}
//	bool operator< (const T& oval) const volatile noexcept {											// < Less-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) < oval);
//	}
//
//	bool operator< (const volatile T& oval) const noexcept {											// < Less-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) < oval);
//	}
//	bool operator< (const volatile T& oval) const volatile noexcept {									// < Less-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) < oval);
//	}

	bool operator> (const SpecAtomic& obj) const noexcept {												// > Greater-than comparison operator
		return (value.load(std::memory_order_seq_cst) > obj.value.load(std::memory_order_seq_cst));
	}
	bool operator> (const SpecAtomic& obj) const volatile noexcept {									// > Greater-than comparison operator
		return (value.load(std::memory_order_seq_cst) > obj.value.load(std::memory_order_seq_cst));
	}

//	bool operator> (const T& oval) const noexcept {														// > Greater-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) > oval);
//	}
//	bool operator> (const T& oval) const volatile noexcept {											// > Greater-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) > oval);
//	}
//
//	bool operator> (const volatile T& oval) const noexcept {											// > Greater-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) > oval);
//	}
//	bool operator> (const volatile T& oval) const volatile noexcept {									// > Greater-than comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) > oval);
//	}

	bool operator<=(const SpecAtomic& obj) const noexcept {												// <= Less-than-or-equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) <= obj.value.load(std::memory_order_seq_cst));
	}
	bool operator<=(const SpecAtomic& obj) const volatile noexcept {									// <= Less-than-or-equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) <= obj.value.load(std::memory_order_seq_cst));
	}

//	bool operator<=(const T& oval) const noexcept {														// <= Less-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) <= oval);
//	}
//	bool operator<=(const T& oval) const volatile noexcept {											// <= Less-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) <= oval);
//	}
//
//	bool operator<=(const volatile T& oval) const noexcept {											// <= Less-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) <= oval);
//	}
//	bool operator<=(const volatile T& oval) const volatile noexcept {									// <= Less-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) <= oval);
//	}

	bool operator>=(const SpecAtomic& obj) const noexcept {												// >= Greater-than-or-equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) >= obj.value.load(std::memory_order_seq_cst));
	}
	bool operator>=(const SpecAtomic& obj) const volatile noexcept {									// >= Greater-than-or-equal-to comparison operator
		return (value.load(std::memory_order_seq_cst) >= obj.value.load(std::memory_order_seq_cst));
	}

//	bool operator>=(const T& oval) const noexcept {														// >= Greater-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) >= oval);
//	}
//	bool operator>=(const T& oval) const volatile noexcept {											// >= Greater-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) >= oval);
//	}
//
//	bool operator>=(const volatile T& oval) const noexcept {											// >= Greater-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) >= oval);
//	}
//	bool operator>=(const volatile T& oval) const volatile noexcept {									// >= Greater-than-or-equal-to comparison operator (other classes)
//		return (value.load(std::memory_order_seq_cst) >= oval);
//	}

	// Overload of stream operators
	////////////////////////////////////

	std::ostream& operator<<(std::ostream& out) const noexcept {										// << ostream operator
		return out << value.load(std::memory_order_seq_cst);
	}
	std::ostream& operator<<(std::ostream& out) const volatile noexcept {								// << ostream operator
		return out << value.load(std::memory_order_seq_cst);
	}
	std::istream& operator>>(std::istream& is) noexcept {												// << istream operator
		return is >> value.load(std::memory_order_seq_cst);
	}
	std::istream& operator>>(std::istream& is) volatile noexcept {										// << istream operator
		return is >> value.load(std::memory_order_seq_cst);
	}

	// Overload of arithmetic operators
	////////////////////////////////////

	SpecAtomic operator+() const noexcept {																// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	SpecAtomic operator+() const volatile noexcept {													// (+) Unary plus (integer promotion) arithmetic operator
		return *(const_cast<SpecAtomic*>(this));
	}
	SpecAtomic operator-() const noexcept {																// (-) Unary minus (additive inverse) arithmetic operator
		return SpecAtomic{-(value.load(std::memory_order_seq_cst))};
	}
	SpecAtomic operator-() const volatile noexcept {													// (-) Unary minus (additive inverse) arithmetic operator
		return SpecAtomic{-(value.load(std::memory_order_seq_cst))};
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), SpecAtomic&>
	operator++() noexcept {																				// ++ (Prefix) Increment arithmetic operator
		value.fetch_add(1, std::memory_order_seq_cst);
		return *this;
 	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), SpecAtomic&>
	operator++() volatile noexcept {																	// ++ (Prefix) Increment arithmetic operator
		value.fetch_add(1, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && !has_fetch_addptr<R>()), SpecAtomic&>
	operator++() noexcept {																				// ++ (Prefix) Increment arithmetic operator
		fetch_add((T)1, std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && !has_fetch_addptr<R>()), SpecAtomic&>
	operator++() volatile noexcept {																	// ++ (Prefix) Increment arithmetic operator
		fetch_add((T)1, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && has_fetch_addptr<R>()), SpecAtomic&>
	operator++() noexcept {																				// ++ (Prefix) Increment arithmetic operator
		fetch_add((typename R::difference_type)1, std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && has_fetch_addptr<R>()), SpecAtomic&>
	operator++() volatile noexcept {																	// ++ (Prefix) Increment arithmetic operator
		fetch_add((typename R::difference_type)1, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	SpecAtomic operator++(int) const noexcept {															// ++ (Postfix) Increment arithmetic operator
		return SpecAtomic(value.load(std::memory_order_seq_cst)+1);
 	}
	SpecAtomic operator++(int) const volatile noexcept {												// ++ (Postfix) Increment arithmetic operator
		return SpecAtomic(value.load(std::memory_order_seq_cst)+1);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), SpecAtomic&>
	operator--() noexcept {																				// -- (Prefix) Decrement arithmetic operator
		value.fetch_sub(1, std::memory_order_seq_cst);
		return *this;
 	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), SpecAtomic&>
	operator--() volatile noexcept {																	// -- (Prefix) Decrement arithmetic operator
		value.fetch_sub(1, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && !has_fetch_subptr<R>()), SpecAtomic&>
	operator--() noexcept {																				// -- (Prefix) Decrement arithmetic operator
		fetch_sub((T)1, std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && !has_fetch_subptr<R>()), SpecAtomic&>
	operator--() volatile noexcept {																	// -- (Prefix) Decrement arithmetic operator
		fetch_sub((T)1, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && has_fetch_subptr<R>()), SpecAtomic&>
	operator--() noexcept {																				// -- (Prefix) Decrement arithmetic operator
		fetch_sub((typename R::difference_type)1, std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && has_fetch_subptr<R>()), SpecAtomic&>
	operator--() volatile noexcept {																	// -- (Prefix) Decrement arithmetic operator
		fetch_sub((typename R::difference_type)1, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	SpecAtomic operator--(int) const noexcept {															// -- (Postfix) Decrement arithmetic operator
		return SpecAtomic(value.load(std::memory_order_seq_cst)-1);
 	}
	SpecAtomic operator--(int) const volatile noexcept {												// -- (Postfix) Decrement arithmetic operator
		return SpecAtomic(value.load(std::memory_order_seq_cst)-1);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), SpecAtomic&>
	operator+=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		value.fetch_add(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), SpecAtomic&>
	operator+=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		value.fetch_add(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), SpecAtomic&>
	operator+=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_add((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), SpecAtomic&>
	operator+=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_add((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && has_fetch_addptr<R>()), SpecAtomic&>
	operator+=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_add((typename R::difference_type)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && has_fetch_addptr<R>()), SpecAtomic&>
	operator+=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_add((typename R::difference_type)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>()), SpecAtomic&>
	operator+=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		fetch_add(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>()), SpecAtomic&>
	operator+=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		fetch_add(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && !has_fetch_addptr<R>()), SpecAtomic&>
	operator+=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		fetch_add((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && !has_fetch_addptr<R>()), SpecAtomic&>
	operator+=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		fetch_add((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	SpecAtomic operator+(const SpecAtomic& obj) const noexcept {										// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)+obj.value.load(std::memory_order_seq_cst)};
	}
	SpecAtomic operator+(const SpecAtomic& obj) const volatile noexcept {								// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)+obj.value.load(std::memory_order_seq_cst)};
	}
	template<typename P>
	SpecAtomic operator+(const P& oval) const noexcept {												// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)+(T)oval};
	}
	template<typename P>
	SpecAtomic operator+(const P& oval) const volatile noexcept {										// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)+(T)oval};
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), SpecAtomic&>
	operator-=(const SpecAtomic& obj) noexcept {														// -= Subtraction assignment arithmetic operator
		value.fetch_sub(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), SpecAtomic&>
	operator-=(const SpecAtomic& obj) volatile noexcept {												// -= Subtraction assignment arithmetic operator
		value.fetch_sub(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), SpecAtomic&>
	operator-=(const P& oval) noexcept {																// -= Subtraction assignment arithmetic operator (other classes)
		value.fetch_sub((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), SpecAtomic&>
	operator-=(const P& oval) volatile noexcept {														// -= Subtraction assignment arithmetic operator (other classes)
		value.fetch_sub((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && has_fetch_subptr<R>()), SpecAtomic&>
	operator-=(const P& oval) noexcept {																// -= Subtraction assignment arithmetic operator (other classes)
		value.fetch_sub((typename R::difference_type)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && has_fetch_subptr<R>()), SpecAtomic&>
	operator-=(const P& oval) volatile noexcept {														// -= Subtraction assignment arithmetic operator (other classes)
		value.fetch_sub((typename R::difference_type)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>()), SpecAtomic&>
	operator-=(const SpecAtomic& obj) noexcept {														// -= Subtraction assignment arithmetic operator
		fetch_sub(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>()), SpecAtomic&>
	operator-=(const SpecAtomic& obj) volatile noexcept {												// -= Subtraction assignment arithmetic operator
		fetch_sub(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && !has_fetch_subptr<R>()), SpecAtomic&>
	operator-=(const P& oval) noexcept {																// -= Subtraction assignment arithmetic operator (other classes)
		fetch_sub((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && !has_fetch_subptr<R>()), SpecAtomic&>
	operator-=(const P& oval) volatile noexcept {														// -= Subtraction assignment arithmetic operator (other classes)
		fetch_sub((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	SpecAtomic operator-(const SpecAtomic& obj) const noexcept {										// - Subtraction arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)-obj.value.load(std::memory_order_seq_cst)};
	}
	SpecAtomic operator-(const SpecAtomic& obj) const volatile noexcept {								// - Subtraction arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)-obj.value.load(std::memory_order_seq_cst)};
	}
	template<typename P>
	SpecAtomic operator-(const P& oval) const noexcept {												// - Subtraction arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)-(T)oval};
	}
	template<typename P>
	SpecAtomic operator-(const P& oval) const volatile noexcept {										// - Subtraction arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)-(T)oval};
	}

	SpecAtomic& operator*=(const SpecAtomic& obj) noexcept {											// *= Multiplication assignment arithmetic operator
		value.store(value.load(std::memory_order_seq_cst)*obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	SpecAtomic& operator*=(const SpecAtomic& obj) volatile noexcept {									// *= Multiplication assignment arithmetic operator
		value.store(value.load(std::memory_order_seq_cst)*obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P>
	SpecAtomic& operator*=(const P& oval) noexcept {													// *= Multiplication assignment arithmetic operator (other classes)
		value.store(value.load(std::memory_order_seq_cst)*(T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P>
	SpecAtomic& operator*=(const P& oval) volatile noexcept {											// *= Multiplication assignment arithmetic operator (other classes)
		value.store(value.load(std::memory_order_seq_cst)*(T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	SpecAtomic operator*(const SpecAtomic& obj) const noexcept {										// * Multiplication arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)*obj.value.load(std::memory_order_seq_cst)};
	}
	SpecAtomic operator*(const SpecAtomic& obj) const volatile noexcept {								// * Multiplication arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)*obj.value.load(std::memory_order_seq_cst)};
	}
	template<typename P>
	SpecAtomic operator*(const P& oval) const noexcept {												// * Multiplication arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)*(T)oval};
	}
	template<typename P>
	SpecAtomic operator*(const P& oval) const volatile noexcept {										// * Multiplication arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)*(T)oval};
	}

	SpecAtomic& operator/=(const SpecAtomic& obj) noexcept {											// /= Division assignment arithmetic operator
		value.store(value.load(std::memory_order_seq_cst)/obj.value, std::memory_order_seq_cst);
		return *this;
	}
	SpecAtomic& operator/=(const SpecAtomic& obj) volatile noexcept {									// /= Division assignment arithmetic operator
		value.store(value.load(std::memory_order_seq_cst)/obj.value, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P>
	SpecAtomic& operator/=(const P& oval) noexcept {													// /= Division assignment arithmetic operator (other classes)
		value.store(value.load(std::memory_order_seq_cst)/(T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P>
	SpecAtomic& operator/=(const P& oval) volatile noexcept {											// /= Division assignment arithmetic operator (other classes)
		value.store(value.load(std::memory_order_seq_cst)/(T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	SpecAtomic operator/(const SpecAtomic& obj) const noexcept {										// / Division arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)/obj.value.load(std::memory_order_seq_cst)};
 	}
	SpecAtomic operator/(const SpecAtomic& obj) const volatile noexcept {								// / Division arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst)/obj.value.load(std::memory_order_seq_cst)};
	}
	template<typename P>
 	SpecAtomic operator/(const P& oval) const noexcept {												// / Division arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)/(T)oval};
 	}
	template<typename P>
	SpecAtomic operator/(const P& oval) const volatile noexcept {										// / Division arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst)/(T)oval};
	}

	// Modulo and Binary operators not available on floating point numbers
	////////////////////////////////////

	// Overload of logical operators
	////////////////////////////////////

	bool operator!() const noexcept {																	// ! NOT logical operator
		return !(value.load(std::memory_order_seq_cst));
	}
	bool operator!() const volatile noexcept {															// ! NOT logical operator
		return !(value.load(std::memory_order_seq_cst));
	}

	bool operator&&(const SpecAtomic& obj) const noexcept {												// && AND logical operator
		return (value.load(std::memory_order_seq_cst) && obj.value.load(std::memory_order_seq_cst));
	}
	bool operator&&(const SpecAtomic& obj) const volatile noexcept {									// && AND logical operator
		return (value.load(std::memory_order_seq_cst) && obj.value.load(std::memory_order_seq_cst));
	}
	template<typename P>
	bool operator&&(const P& oval) const noexcept {														// && AND logical operator (other clases)
		return (value.load(std::memory_order_seq_cst) && (T)oval);
	}
	template<typename P>
	bool operator&&(const P& oval) const volatile noexcept {											// && AND logical operator (other clases)
		return (value.load(std::memory_order_seq_cst) && (T)oval);
	}

	bool operator||(const SpecAtomic& obj) const noexcept {												// || OR logical operator
		return (value.load(std::memory_order_seq_cst) || obj.value.load(std::memory_order_seq_cst));
	}
	bool operator||(const SpecAtomic& obj) const volatile noexcept {									// || OR logical operator
		return (value.load(std::memory_order_seq_cst) || obj.value.load(std::memory_order_seq_cst));
	}
	template<typename P>
	bool operator||(const P& oval) const noexcept {														// || OR logical operator (other clases)
		return (value.load(std::memory_order_seq_cst) || (T)oval);
	}
	template<typename P>
	bool operator||(const P& oval) const volatile noexcept {											// || OR logical operator (other clases)
		return (value.load(std::memory_order_seq_cst) || (T)oval);
	}

	// Overload of bitwise operators
	////////////////////////////////////

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_and<R>()), SpecAtomic&>
	operator&=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		value.fetch_and(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_and<R>()), SpecAtomic&>
	operator&=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		value.fetch_and(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>()), SpecAtomic&>
	operator&=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		fetch_and(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>()), SpecAtomic&>
	operator&=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		fetch_and(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_and<R>()), SpecAtomic&>
	operator&=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_and((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_and<R>()), SpecAtomic&>
	operator&=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_and((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && has_fetch_andptr<R>()), SpecAtomic&>
	operator&=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_and((typename R::difference_type)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && has_fetch_andptr<R>()), SpecAtomic&>
	operator&=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_and((typename R::difference_type)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && !has_fetch_andptr<R>()), SpecAtomic&>
	operator&=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		fetch_and((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && !has_fetch_andptr<R>()), SpecAtomic&>
	operator&=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		fetch_and((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	SpecAtomic operator&(const SpecAtomic& obj) const noexcept {										// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst) & obj.value.load(std::memory_order_seq_cst)};
	}
	SpecAtomic operator&(const SpecAtomic& obj) const volatile noexcept {								// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst) & obj.value.load(std::memory_order_seq_cst)};
	}
	template<typename P>
	SpecAtomic operator&(const P& oval) const noexcept {												// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst) & (T)oval};
	}
	template<typename P>
	SpecAtomic operator&(const P& oval) const volatile noexcept {										// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst) & (T)oval};
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_or<R>()), SpecAtomic&>
	operator|=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		value.fetch_or(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_or<R>()), SpecAtomic&>
	operator|=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		value.fetch_or(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>()), SpecAtomic&>
	operator|=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		fetch_or(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>()), SpecAtomic&>
	operator|=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		fetch_or(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_or<R>()), SpecAtomic&>
	operator|=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_or((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_or<R>()), SpecAtomic&>
	operator|=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_or((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && has_fetch_orptr<R>()), SpecAtomic&>
	operator|=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_or((typename R::difference_type)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && has_fetch_orptr<R>()), SpecAtomic&>
	operator|=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_or((typename R::difference_type)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && !has_fetch_orptr<R>()), SpecAtomic&>
	operator|=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		fetch_or((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && !has_fetch_orptr<R>()), SpecAtomic&>
	operator|=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		fetch_or((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	SpecAtomic operator|(const SpecAtomic& obj) const noexcept {										// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst) | obj.value.load(std::memory_order_seq_cst)};
	}
	SpecAtomic operator|(const SpecAtomic& obj) const volatile noexcept {								// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst) | obj.value.load(std::memory_order_seq_cst)};
	}
	template<typename P>
	SpecAtomic operator|(const P& oval) const noexcept {												// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst) | (T)oval};
	}
	template<typename P>
	SpecAtomic operator|(const P& oval) const volatile noexcept {										// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst) | (T)oval};
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_xor<R>()), SpecAtomic&>
	operator^=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		value.fetch_xor(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_xor<R>()), SpecAtomic&>
	operator^=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		value.fetch_xor(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>()), SpecAtomic&>
	operator^=(const SpecAtomic& obj) noexcept {														// += Addition assignment arithmetic operator
		fetch_xor(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return *this;
	}
	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>()), SpecAtomic&>
	operator^=(const SpecAtomic& obj) volatile noexcept {												// += Addition assignment arithmetic operator
		fetch_xor(obj.value.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_xor<R>()), SpecAtomic&>
	operator^=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_xor((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_xor<R>()), SpecAtomic&>
	operator^=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_xor((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && has_fetch_xorptr<R>()), SpecAtomic&>
	operator^=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		value.fetch_xor((typename R::difference_type)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && has_fetch_xorptr<R>()), SpecAtomic&>
	operator^=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		value.fetch_xor((typename R::difference_type)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && !has_fetch_xorptr<R>()), SpecAtomic&>
	operator^=(const P& oval) noexcept {																// += Addition assignment arithmetic operator (other classes)
		fetch_xor((T)oval, std::memory_order_seq_cst);
		return *this;
	}
	template<typename P, typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && !has_fetch_xorptr<R>()), SpecAtomic&>
	operator^=(const P& oval) volatile noexcept {														// += Addition assignment arithmetic operator (other classes)
		fetch_xor((T)oval, std::memory_order_seq_cst);
		return const_cast<SpecAtomic&>(*this);;
	}

	SpecAtomic operator^(const SpecAtomic& obj) const noexcept {										// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst) ^ obj.value.load(std::memory_order_seq_cst)};
	}
	SpecAtomic operator^(const SpecAtomic& obj) const volatile noexcept {								// + Addition arithmetic operator
		return SpecAtomic{value.load(std::memory_order_seq_cst) ^ obj.value.load(std::memory_order_seq_cst)};
	}
	template<typename P>
	SpecAtomic operator^(const P& oval) const noexcept {												// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst) ^ (T)oval};
	}
	template<typename P>
	SpecAtomic operator^(const P& oval) const volatile noexcept {										// + Addition arithmetic operator (other classes)
		return SpecAtomic{value.load(std::memory_order_seq_cst) ^ (T)oval};
	}

	// Overload of other operators
	////////////////////////////////////

#ifdef SLSPECATOMIC_OVERLOAD_ADDRESSOF_OP
	std::atomic<T>* operator&() noexcept {																// & Address-of operator (??)
		return &value;
	}
	std::atomic<T>* operator&() volatile noexcept {														// & Address-of operator (??)
		return &value;
	}
#endif

//	SpecAtomic operator,(SpecAtomic& obj) noexcept {													// , Comma operator (not applicable)
//		return SpecAtomic{value, (obj.value)};
//	}
//	SpecAtomic operator,(SpecAtomic& obj) volatile noexcept {											// , Comma operator (not applicable)
//		return SpecAtomic{value, (obj.value)};
//	}

	// The rest of member, pointer, allocate and function operators make no sense for this class
	////////////////////////////////////

	// Main Atomic functions
	////////////////////////////////////

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), T>
	fetch_add(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_add(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_add<R>()), T>
	fetch_add(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_add(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && has_fetch_addptr<R>()), T>
	fetch_add(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_add(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && has_fetch_addptr<R>()), T>
	fetch_add(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_add(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && !has_fetch_addptr<R>()), T>
	fetch_add(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected + arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_add<R>() && !has_fetch_addptr<R>()), T>
	fetch_add(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected + arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), T>
	fetch_sub(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_sub(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_sub<R>()), T>
	fetch_sub(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_sub(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && has_fetch_subptr<R>()), T>
	fetch_sub(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_sub(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && has_fetch_subptr<R>()), T>
	fetch_sub(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_sub(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && !has_fetch_subptr<R>()), T>
	fetch_sub(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected - arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_sub<R>() && !has_fetch_subptr<R>()), T>
	fetch_sub(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected - arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_and<R>()), T>
	fetch_and(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_and(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_and<R>()), T>
	fetch_and(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_and(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && has_fetch_andptr<R>()), T>
	fetch_and(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_and(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && has_fetch_andptr<R>()), T>
	fetch_and(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_and(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && !has_fetch_andptr<R>()), T>
	fetch_and(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected & arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_and<R>() && !has_fetch_andptr<R>()), T>
	fetch_and(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected & arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_or<R>()), T>
	fetch_or(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_or(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_or<R>()), T>
	fetch_or(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_or(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && has_fetch_orptr<R>()), T>
	fetch_or(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_or(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && has_fetch_orptr<R>()), T>
	fetch_or(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_or(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && !has_fetch_orptr<R>()), T>
	fetch_or(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected | arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_or<R>() && !has_fetch_orptr<R>()), T>
	fetch_or(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected | arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_xor<R>()), T>
	fetch_xor(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_xor(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(has_fetch_xor<R>()), T>
	fetch_xor(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_xor(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && has_fetch_xorptr<R>()), T>
	fetch_xor(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		return value.fetch_xor(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && has_fetch_xorptr<R>()), T>
	fetch_xor(typename R::difference_type arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		return value.fetch_xor(arg, memord);
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && !has_fetch_xorptr<R>()), T>
	fetch_xor(T arg, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected ^ arg, memord, memord));
		return expected;
	}

	template<typename R = std::atomic<T>>
	std::enable_if_t<(!has_fetch_xor<R>() && !has_fetch_xorptr<R>()), T>
	fetch_xor(T arg, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		T expected = value.load(memord);
		while(!value.compare_exchange_weak(expected, expected ^ arg, memord, memord));
		return expected;
	}

	bool is_lock_free() const noexcept {
		return value.is_lock_free();
	}

	bool is_lock_free() const volatile noexcept {
		return value.is_lock_free();
	}

#if __cplusplus >= 202002L
	void notify_all() noexcept {
		value.notify_all();
	}

	void notify_all() volatile noexcept {
		value.notify_all();
	}

	void notify_one() noexcept {
		value.notify_one();
	}

	void notify_one() volatile noexcept {
		value.notify_one();
	}

	void wait(T old, std::memory_order memorder = std::memory_order_seq_cst) const noexcept {
		value.wait(old, memorder);
	}

	void wait(T old, std::memory_order memorder = std::memory_order_seq_cst) const volatile noexcept {
		value.wait(old, memorder);
	}

#endif

	T exchange(T desired, std::memory_order memord = std::memory_order_seq_cst) noexcept{
		return value.exchange(desired, memord);
	}
	T exchange(T desired, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept{
		return value.exchange(desired, memord);
	}

	T load(std::memory_order memord = std::memory_order_seq_cst) const noexcept {
		return value.load(memord);
	}

	T load(std::memory_order memord = std::memory_order_seq_cst) const volatile noexcept {
		return value.load(memord);
	}

	void store(T desired, std::memory_order memord = std::memory_order_seq_cst) noexcept {
		value.store(desired, memord);
	}

	void store(T desired, std::memory_order memord = std::memory_order_seq_cst) volatile noexcept {
		value.store(desired, memord);
	}

	bool compare_exchange_weak(T& expected, T desired, std::memory_order memsuccess, std::memory_order memfailure) noexcept {
		return value.compare_exchange_weak(expected, desired, memsuccess, memfailure);
	}

	bool compare_exchange_weak(T& expected, T desired, std::memory_order memsuccess, std::memory_order memfailure) volatile noexcept {
		return value.compare_exchange_weak(expected, desired, memsuccess, memfailure);
	}

	bool compare_exchange_weak(T& expected, T desired, std::memory_order memorder = std::memory_order_seq_cst) noexcept {
		return value.compare_exchange_weak(expected, desired, memorder);
	}

	bool compare_exchange_weak(T& expected, T desired, std::memory_order memorder = std::memory_order_seq_cst) volatile noexcept {
		return value.compare_exchange_weak(expected, desired, memorder);
	}

	bool compare_exchange_strong(T& expected, T desired, std::memory_order memsuccess, std::memory_order memfailure) noexcept {
		return value.compare_exchange_strong(expected, desired, memsuccess, memfailure);
	}

	bool compare_exchange_strong(T& expected, T desired, std::memory_order memsuccess, std::memory_order memfailure) volatile noexcept {
		return value.compare_exchange_strong(expected, desired, memsuccess, memfailure);
	}

	bool compare_exchange_strong(T& expected, T desired, std::memory_order memorder = std::memory_order_seq_cst) noexcept {
		return value.compare_exchange_strong(expected, desired, memorder);
	}

	bool compare_exchange_strong(T& expected, T desired, std::memory_order memorder = std::memory_order_seq_cst) volatile noexcept {
		return value.compare_exchange_strong(expected, desired, memorder);
	}

	// Additional functions
	////////////////////////////////////

	T getValue(std::memory_order memord = std::memory_order_seq_cst) const noexcept {			// Custom function to get the internal value directly (normally it should not be necessary to use it)
		return value.load(memord);
	}
	T getValue(std::memory_order memord = std::memory_order_seq_cst) const volatile noexcept {	// Custom function to get the internal value directly (normally it should not be necessary to use it)
		return value.load(memord);
	}

	std::atomic<T>& getValueRef() noexcept {													// Custom function to get the internal value by reference directly (normally shouldn't be necessary either)
		return value;
	}
	std::atomic<T>& getValueRef() volatile noexcept {											// Custom function to get the internal value by reference directly (normally shouldn't be necessary either)
		return value;
	}

	std::atomic<T>* getValuePtr() noexcept {													// Custom function to get the pointer of the internal value directly
		return &value;
	}
	std::atomic<T>* getValuePtr() volatile noexcept {											// Custom function to get the pointer of the internal value directly
		return &value;
	}

	SpecAtomic* getObjPtr() noexcept {															// Custom function to get a pointer to the object
		return this;
	}
	SpecAtomic* getObjPtr() volatile noexcept {													// Custom function to get a pointer to the object
		return this;
	}

};

// External overload of operators seems not necesary and can creaty ambiguity
////////////////////////////////////

} //namespace SpecLib

#endif // __SPECATOMIC_H
