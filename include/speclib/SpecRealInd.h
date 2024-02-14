/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     SpecRealInd.h
/// \brief    SpecRealInd class wraper to speculate on floating point types with local margin of error
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef __SPECREALIND_H
#define __SPECREALIND_H

#include <type_traits>
#include <ostream>
#include <istream>
#include <limits>
#include <utility>
#include <cassert>
#include "speclib/SpecRealComm.h"

namespace SpecLib {

// SpecRealInd class definition
template<typename T, const specreal_eps_type_t EPS_TYPE = EPS_ABSOLUTE>
class SpecRealInd {
};

// SpecRealInd partial specialization for EPS_TYPE = EPS_ABSOLUTE
template<typename T>
class SpecRealInd<T, EPS_ABSOLUTE> {
private:
	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;
public:
	using value_type = T;
	using value_type_nv = expr_type<T>;
	using precision_type = value_type_nv;
	static constexpr specreal_eps_type_t eps_type = EPS_ABSOLUTE;

	static constexpr value_type_nv defaultEpsilonAbs = std::numeric_limits<value_type_nv>::epsilon() * static_cast<value_type_nv>(100.0);

private:
	using TNV = value_type_nv;

	T value;						// The real value is stored here
	TNV epsilonAbs;					// The absolute margin of error allowed in the comparisons (must be positive)

public:
	// Overload of cast operator (default)
	//////////////////////////////////////
	constexpr operator T() const noexcept { return value; }				// Cast operator (EPS_ABSOLUTE)

	constexpr operator T() const volatile noexcept { return value; }	// Cast operator (EPS_ABSOLUTE)

	// Constructors and destructor
	//////////////////////////////

	SpecRealInd() noexcept = default;									// Default constructor (EPS_ABSOLUTE)

	SpecRealInd(const T _value) noexcept :								// Default constructor (EPS_ABSOLUTE)
	value(_value),
	epsilonAbs(defaultEpsilonAbs)
	{ }

	template<typename P>
	SpecRealInd(const T _value, const P _epsilon) noexcept :			// Custom constructor (EPS_ABSOLUTE)
	value(_value),
	epsilonAbs(_epsilon)
	{ }

	~SpecRealInd() noexcept = default;									// Destructor (EPS_ABSOLUTE)

	SpecRealInd(const SpecRealInd& other) = default;					// Copy constructor (EPS_ABSOLUTE)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(const SpecRealInd<T, EPS_TYPE2>& other) noexcept :		// Copy constructor (other SpecReal type) (EPS_ABSOLUTE)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(const SpecRealInd& other, const P _epsilon) noexcept :	// Copy constructor (alt version) (EPS_ABSOLUTE)
	value(other.value),
	epsilonAbs(_epsilon)
	{ }

	SpecRealInd(SpecRealInd&& other) noexcept = default;				// Move constructor (EPS_ABSOLUTE)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(SpecRealInd<T, EPS_TYPE2>&& other) noexcept :			// Move constructor (EPS_ABSOLUTE) (other SpecReal type)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(SpecRealInd&& other, const P _epsilon) noexcept :		// Move constructor (alt version) (EPS_ABSOLUTE)
	value(other.value),
	epsilonAbs(_epsilon)
	{ }

	// Overload of assignment operators
	////////////////////////////////////

	SpecRealInd& operator=(const SpecRealInd& other) noexcept = default;			// Copy assignment
//	SpecRealInd& operator=(const volatile SpecRealInd& other) volatile noexcept {	// Copy assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const SpecRealInd<T, EPS_TYPE2>& other) noexcept {					// Copy assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const volatile SpecRealInd<T, EPS_TYPE2>& other) volatile noexcept {	// Copy assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

	SpecRealInd& operator=(SpecRealInd&& other) noexcept = default;					// Move assignment
//	SpecRealInd& operator=(volatile SpecRealInd&& other) volatile noexcept {		// Move assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(SpecRealInd<T, EPS_TYPE2>&& other) noexcept {						// Move assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(volatile SpecRealInd<T, EPS_TYPE2>&& other) volatile noexcept {		// Move assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

//	SpecRealInd& operator=(SpecRealInd obj) noexcept {								// Direct assigment (not necessary and creates ambiguity)
//		value = obj.value;
//		return *this;
//	}
//	template<typename P>
//	SpecRealInd& operator=(P oval) noexcept {										// Direct assigment (not necessary and creates ambiguity)
//		value = oval;
//		return *this;
//	}

	// Overload of comparison operators (EPS_ABSOLUTE)
	////////////////////////////////////////////////////////////

	constexpr bool operator==(const SpecRealInd& obj) const noexcept {				// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs);
	}
	constexpr bool operator==(const SpecRealInd& obj) const volatile noexcept {		// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs);
	}

	template<typename P>
	constexpr bool operator==(const P& oval) const noexcept {						// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs);
	}
	template<typename P>
	constexpr bool operator==(const P& oval) const volatile noexcept {				// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs);
	}

	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const noexcept {				// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs);
	}
	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const volatile noexcept {		// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs);
	}

	template<typename P>
	constexpr bool operator!=(const P& oval) const noexcept {						// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs);
	}
	template<typename P>
	constexpr bool operator!=(const P& oval) const volatile noexcept {				// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs);
	}

	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const noexcept {				// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs);
	}
	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const volatile noexcept {		// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs);
	}

	template<typename P>
	constexpr bool operator<(const P& oval) const noexcept {						// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs);
	}
	template<typename P>
	constexpr bool operator<(const P& oval) const volatile noexcept {				// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs);
	}

	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const noexcept {				// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs);
	}
	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const volatile noexcept {		// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs);
	}

	template<typename P>
	constexpr bool operator>(const P& oval) const noexcept {						// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs);
	}
	template<typename P>
	constexpr bool operator>(const P& oval) const volatile noexcept {				// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs);
	}

	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const noexcept {				// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs);
	}
	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const volatile noexcept {		// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs);
	}

	template<typename P>
	constexpr bool operator<=(const P& oval) const noexcept {						// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs);
	}
	template<typename P>
	constexpr bool operator<=(const P& oval) const volatile noexcept {				// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs);
	}

	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const noexcept {				// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs);
	}
	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const volatile noexcept {		// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs);
	}

	template<typename P>
	constexpr bool operator>=(const P& oval) const noexcept {						// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs);
	}
	template<typename P>
	constexpr bool operator>=(const P& oval) const volatile noexcept {				// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs);
	}

	// Overload of stream operators
	////////////////////////////////////

	constexpr std::ostream& operator<<(std::ostream& out) const noexcept {				// << ostream operator
		return out << value;
	}
	constexpr std::ostream& operator<<(std::ostream& out) const volatile noexcept {		// << ostream operator
		return out << value;
	}
	constexpr std::istream& operator>>(std::istream& is) noexcept {						// << istream operator
		return is >> value;
	}
	constexpr std::istream& operator>>(std::istream& is) volatile noexcept {			// << istream operator
		return is >> value;
	}

	// Overload of arithmetic operators
	////////////////////////////////////

	constexpr SpecRealInd operator+() const noexcept {									// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator+() const volatile noexcept {							// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator-() const noexcept {									// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd operator-() const volatile noexcept {							// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd& operator++() noexcept {										// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator++() volatile noexcept {								// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator++(int) const noexcept {								// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator++(int) const volatile noexcept {						// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
	}
	constexpr SpecRealInd& operator--() noexcept {										// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator--() volatile noexcept {								// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator--(int) const noexcept {								// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator--(int) const volatile noexcept {						// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
	}

	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) noexcept {				// += Addition assignment arithmetic operator
		value += obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) volatile noexcept {		// += Addition assignment arithmetic operator
		value += obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) noexcept {							// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) volatile noexcept {				// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const noexcept {			// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const volatile noexcept {	// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const noexcept {						// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const volatile noexcept {			// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) noexcept {				// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) volatile noexcept {		// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) noexcept {							// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) volatile noexcept {				// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const noexcept {			// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const volatile noexcept {	// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const noexcept {						// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const volatile noexcept {			// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) noexcept {				// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) volatile noexcept {		// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) noexcept {							// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) volatile noexcept {				// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const noexcept {			// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const volatile noexcept {	// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const noexcept {						// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const volatile noexcept {			// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) noexcept {				// /= Division assignment arithmetic operator
		value /= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) volatile noexcept {		// /= Division assignment arithmetic operator
		value /= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) noexcept {							// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) volatile noexcept {				// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const noexcept {			// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
 	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const volatile noexcept {	// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const noexcept {						// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
 	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const volatile noexcept {			// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
	}

	// Modulo and Binary operators not available on floating point numbers
	////////////////////////////////////

	// Overload of logical operators
	////////////////////////////////////

	constexpr bool operator!() const noexcept {									// ! NOT logical operator
		return !value;
	}
	constexpr bool operator!() const volatile noexcept {						// ! NOT logical operator
		return !value;
	}
	constexpr bool operator&&(const SpecRealInd& obj) const noexcept {			// && AND logical operator
		return (value && obj.value);
	}
	constexpr bool operator&&(const SpecRealInd& obj) const volatile noexcept {	// && AND logical operator
		return (value && obj.value);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const noexcept {					// && AND logical operator (other clases)
		return (value && oval);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const volatile noexcept {			// && AND logical operator (other clases)
		return (value && oval);
	}
	constexpr bool operator||(const SpecRealInd& obj) const noexcept {			// || OR logical operator
		return (value || obj.value);
	}
	constexpr bool operator||(const SpecRealInd& obj) const volatile noexcept {	// || OR logical operator
		return (value || obj.value);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const noexcept {					// || OR logical operator (other clases)
		return (value || oval);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const volatile noexcept {			// || OR logical operator (other clases)
		return (value || oval);
	}

	// Overload of other operators
	////////////////////////////////////

#ifdef SLSPECREAL_OVERLOAD_ADDRESSOF_OP
	constexpr T* operator&() noexcept {											// & Address-of operator (??)
		return &value;
	}
	constexpr T* operator&() volatile noexcept {								// & Address-of operator (??)
		return &value;
	}
#endif

	constexpr SpecRealInd operator,(SpecRealInd& obj) noexcept {				// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	constexpr SpecRealInd operator,(SpecRealInd& obj) volatile noexcept {		// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) noexcept {							// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) volatile noexcept {				// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}

	// The rest of member, pointer, bitwise, allocate and function operators make no sense for this class
	////////////////////////////////////

	// Additional functions
	////////////////////////////////////

	constexpr T getValue() const noexcept {					// Custom function to get the internal value directly
		return value;
	}

	constexpr T getValue() const volatile noexcept {		// Custom function to get the internal value directly
		return value;
	}

	constexpr T& getValueRef() noexcept {					// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr volatile T& getValueRef() volatile noexcept {	// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr T* getValuePtr() noexcept {					// Custom function to get the pointer of the internal value directly
		return &value;
	}
	
	constexpr T* getValuePtr() volatile noexcept {			// Custom function to get the pointer of the internal value directly
		return &value;
	}

	constexpr SpecRealInd* getObjPtr() noexcept {			// Custom function to get a pointer to the object
		return this;
	}

	constexpr volatile SpecRealInd* getObjPtr() volatile noexcept {				// Custom function to get a pointer to the object
		return this;
	}

	constexpr TNV getAbsPrecisionThreshold() const noexcept {					// Custom function to get the absolute precision threshold used in comparisons
		return epsilonAbs;
	}
	constexpr TNV getAbsPrecisionThreshold() const volatile noexcept {			// Custom function to get the absolute precision threshold used in comparisons
		return epsilonAbs;
	}
	constexpr TNV getPrecisionThreshold() const noexcept {						// Custom function to get the precision threshold used in comparisons
		return getAbsPrecisionThreshold();
	}
	constexpr TNV getPrecisionThreshold() const volatile noexcept {				// Custom function to get the precision threshold used in comparisons
		return getAbsPrecisionThreshold();
	}

	constexpr void setAbsPrecisionThreshold(const TNV pr) noexcept {			// Custom function to set the absolute precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonAbs = pr;
	}
	constexpr void setAbsPrecisionThreshold(const TNV pr) volatile noexcept {	// Custom function to set the absolute precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonAbs = pr;
	}
	constexpr void setPrecisionThreshold(const TNV pr) noexcept {				// Custom function to set the precision threshold used in comparisons (EPS_ABSOLUTE)
		setAbsPrecisionThreshold(pr);
	}
	constexpr void setPrecisionThreshold(const TNV pr) volatile noexcept {		// Custom function to set the precision threshold used in comparisons (EPS_ABSOLUTE)
		setAbsPrecisionThreshold(pr);
	}

private:
	// Function to get the absolute value of a number
	static constexpr T abs(const T n) noexcept {
		return (n >= static_cast<T>(0) ? n : -n);
	}

	// Comparison functions (EPS_ABSOLUTE)
	constexpr bool EqualComp(const T oval, const TNV epsAbs) const noexcept {
		return abs(value - oval) <= epsAbs;
	}
	constexpr bool EqualComp(const T oval, const TNV epsAbs) const volatile noexcept {
		return abs(value - oval) <= epsAbs;
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsAbs) const noexcept {
		return abs(value - oval) > epsAbs;
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsAbs) const volatile noexcept {
		return abs(value - oval) > epsAbs;
	}
	constexpr bool LessThanComp(const T oval, const TNV epsAbs) const noexcept {
		return value < (oval - epsAbs);
	}
	constexpr bool LessThanComp(const T oval, const TNV epsAbs) const volatile noexcept {
		return value < (oval - epsAbs);
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsAbs) const noexcept {
		return value > (oval + epsAbs);
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsAbs) const volatile noexcept {
		return value > (oval + epsAbs);
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsAbs) const noexcept {
		return value <= (oval + epsAbs);
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsAbs) const volatile noexcept {
		return value <= (oval + epsAbs);
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsAbs) const noexcept {
		return value >= (oval - epsAbs);
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsAbs) const volatile noexcept {
		return value >= (oval - epsAbs);
	}
};

// SpecRealInd partial specialization for EPS_TYPE = EPS_RELATIVE
template<typename T>
class SpecRealInd<T, EPS_RELATIVE> {
private:
	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;
public:
	using value_type = T;
	using value_type_nv = expr_type<T>;
	using precision_type = value_type_nv;
	static constexpr specreal_eps_type_t eps_type = EPS_RELATIVE;

	static constexpr value_type_nv defaultEpsilonRel = std::numeric_limits<value_type_nv>::epsilon() * static_cast<value_type_nv>(8.0);

private:
	using TNV = value_type_nv;

	T value;						// The real value is stored here
	TNV epsilonRel;					// The absolute margin of error allowed in the comparisons (must be positive)

public:
	// Overload of cast operator (default)
	//////////////////////////////////////
	constexpr operator T() const noexcept { return value; }				// Cast operator (EPS_RELATIVE)

	constexpr operator T() const volatile noexcept { return value; }	// Cast operator (EPS_RELATIVE)

	// Constructors and destructor
	//////////////////////////////

	SpecRealInd() noexcept = default;									// Default constructor (EPS_RELATIVE)

	SpecRealInd(const T _value) noexcept :								// Default constructor (EPS_RELATIVE)
	value(_value),
	epsilonRel(defaultEpsilonRel)
	{ }

	template<typename P>
	SpecRealInd(const T _value, const P _epsilon) noexcept :			// Custom constructor (EPS_RELATIVE)
	value(_value),
	epsilonRel(_epsilon)
	{ }

	~SpecRealInd() noexcept = default;									// Destructor (EPS_RELATIVE)

	SpecRealInd(const SpecRealInd& other) = default;					// Copy constructor (EPS_RELATIVE)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(const SpecRealInd<T, EPS_TYPE2>& other) noexcept :		// Copy constructor (other SpecReal type) (EPS_RELATIVE)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(const SpecRealInd& other, const P _epsilon) noexcept :	// Copy constructor (alt version) (EPS_RELATIVE)
	value(other.value),
	epsilonRel(_epsilon)
	{ }

	SpecRealInd(SpecRealInd&& other) noexcept = default;				// Move constructor (EPS_RELATIVE)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(SpecRealInd<T, EPS_TYPE2>&& other) noexcept :			// Move constructor (other SpecReal type) (EPS_RELATIVE)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(SpecRealInd&& other, const P _epsilon) noexcept :		// Move constructor (alt version) (EPS_RELATIVE)
	value(other.value),
	epsilonRel(_epsilon)
	{ }

	// Overload of assignment operators
	////////////////////////////////////

	SpecRealInd& operator=(const SpecRealInd& other) noexcept = default;			// Copy assignment
//	SpecRealInd& operator=(const volatile SpecRealInd& other) volatile noexcept {	// Copy assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const SpecRealInd<T, EPS_TYPE2>& other) noexcept {					// Copy assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const volatile SpecRealInd<T, EPS_TYPE2>& other) volatile noexcept {	// Copy assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

	SpecRealInd& operator=(SpecRealInd&& other) noexcept = default;					// Move assignment
//	SpecRealInd& operator=(volatile SpecRealInd&& other) volatile noexcept {		// Move assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(SpecRealInd<T, EPS_TYPE2>&& other) noexcept {						// Move assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(volatile SpecRealInd<T, EPS_TYPE2>&& other) volatile noexcept {		// Move assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

//	SpecRealInd& operator=(SpecRealInd obj) noexcept {								// Direct assigment (not necessary and creates ambiguity)
//		value = obj.value;
//		return *this;
//	}
//	template<typename P>
//	SpecRealInd& operator=(P oval) noexcept {										// Direct assigment (not necessary and creates ambiguity)
//		value = oval;
//		return *this;
//	}

	// Overload of comparison operators (EPS_RELATIVE)
	////////////////////////////////////////////////////////////

	constexpr bool operator==(const SpecRealInd& obj) const noexcept {				// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonRel);
	}
	constexpr bool operator==(const SpecRealInd& obj) const volatile noexcept {		// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonRel);
	}

	template<typename P>
	constexpr bool operator==(const P& oval) const noexcept {						// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonRel);
	}
	template<typename P>
	constexpr bool operator==(const P& oval) const volatile noexcept {				// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const noexcept {				// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const volatile noexcept {		// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonRel);
	}

	template<typename P>
	constexpr bool operator!=(const P& oval) const noexcept {						// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonRel);
	}
	template<typename P>
	constexpr bool operator!=(const P& oval) const volatile noexcept {				// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const noexcept {				// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const volatile noexcept {		// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonRel);
	}

	template<typename P>
	constexpr bool operator<(const P& oval) const noexcept {						// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonRel);
	}
	template<typename P>
	constexpr bool operator<(const P& oval) const volatile noexcept {				// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const noexcept {				// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const volatile noexcept {		// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonRel);
	}

	template<typename P>
	constexpr bool operator>(const P& oval) const noexcept {						// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonRel);
	}
	template<typename P>
	constexpr bool operator>(const P& oval) const volatile noexcept {				// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const noexcept {				// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const volatile noexcept {		// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonRel);
	}

	template<typename P>
	constexpr bool operator<=(const P& oval) const noexcept {						// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonRel);
	}
	template<typename P>
	constexpr bool operator<=(const P& oval) const volatile noexcept {				// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const noexcept {				// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const volatile noexcept {		// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonRel);
	}

	template<typename P>
	constexpr bool operator>=(const P& oval) const noexcept {						// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonRel);
	}
	template<typename P>
	constexpr bool operator>=(const P& oval) const volatile noexcept {				// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonRel);
	}

	// Overload of stream operators
	////////////////////////////////////

	constexpr std::ostream& operator<<(std::ostream& out) const noexcept {				// << ostream operator
		return out << value;
	}
	constexpr std::ostream& operator<<(std::ostream& out) const volatile noexcept {		// << ostream operator
		return out << value;
	}
	constexpr std::istream& operator>>(std::istream& is) noexcept {						// << istream operator
		return is >> value;
	}
	constexpr std::istream& operator>>(std::istream& is) volatile noexcept {			// << istream operator
		return is >> value;
	}

	// Overload of arithmetic operators
	////////////////////////////////////

	constexpr SpecRealInd operator+() const noexcept {									// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator+() const volatile noexcept {							// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator-() const noexcept {									// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd operator-() const volatile noexcept {							// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd& operator++() noexcept {										// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator++() volatile noexcept {								// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator++(int) const noexcept {								// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator++(int) const volatile noexcept {						// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
	}
	constexpr SpecRealInd& operator--() noexcept {										// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator--() volatile noexcept {								// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator--(int) const noexcept {								// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator--(int) const volatile noexcept {						// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
	}

	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) noexcept {				// += Addition assignment arithmetic operator
		value += obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) volatile noexcept {		// += Addition assignment arithmetic operator
		value += obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) noexcept {							// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) volatile noexcept {				// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const noexcept {			// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const volatile noexcept {	// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const noexcept {						// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const volatile noexcept {			// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) noexcept {				// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) volatile noexcept {		// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) noexcept {							// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) volatile noexcept {				// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const noexcept {			// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const volatile noexcept {	// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const noexcept {						// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const volatile noexcept {			// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) noexcept {				// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) volatile noexcept {		// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) noexcept {							// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) volatile noexcept {				// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const noexcept {			// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const volatile noexcept {	// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const noexcept {						// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const volatile noexcept {			// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) noexcept {				// /= Division assignment arithmetic operator
		value /= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) volatile noexcept {		// /= Division assignment arithmetic operator
		value /= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) noexcept {							// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) volatile noexcept {				// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const noexcept {			// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
 	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const volatile noexcept {	// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const noexcept {						// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
 	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const volatile noexcept {			// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
	}

	// Modulo and Binary operators not available on floating point numbers
	////////////////////////////////////

	// Overload of logical operators
	////////////////////////////////////

	constexpr bool operator!() const noexcept {									// ! NOT logical operator
		return !value;
	}
	constexpr bool operator!() const volatile noexcept {						// ! NOT logical operator
		return !value;
	}
	constexpr bool operator&&(const SpecRealInd& obj) const noexcept {			// && AND logical operator
		return (value && obj.value);
	}
	constexpr bool operator&&(const SpecRealInd& obj) const volatile noexcept {	// && AND logical operator
		return (value && obj.value);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const noexcept {					// && AND logical operator (other clases)
		return (value && oval);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const volatile noexcept {			// && AND logical operator (other clases)
		return (value && oval);
	}
	constexpr bool operator||(const SpecRealInd& obj) const noexcept {			// || OR logical operator
		return (value || obj.value);
	}
	constexpr bool operator||(const SpecRealInd& obj) const volatile noexcept {	// || OR logical operator
		return (value || obj.value);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const noexcept {					// || OR logical operator (other clases)
		return (value || oval);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const volatile noexcept {			// || OR logical operator (other clases)
		return (value || oval);
	}

	// Overload of other operators
	////////////////////////////////////

#ifdef SLSPECREAL_OVERLOAD_ADDRESSOF_OP
	constexpr T* operator&() noexcept {											// & Address-of operator (??)
		return &value;
	}
	constexpr T* operator&() volatile noexcept {								// & Address-of operator (??)
		return &value;
	}
#endif

	constexpr SpecRealInd operator,(SpecRealInd& obj) noexcept {				// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	constexpr SpecRealInd operator,(SpecRealInd& obj) volatile noexcept {		// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) noexcept {							// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) volatile noexcept {				// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}

	// The rest of member, pointer, bitwise, allocate and function operators make no sense for this class
	////////////////////////////////////

	// Additional functions
	////////////////////////////////////

	constexpr T getValue() const noexcept {							// Custom function to get the internal value directly
		return value;
	}

	constexpr T getValue() const volatile noexcept {				// Custom function to get the internal value directly
		return value;
	}

	constexpr T& getValueRef() noexcept {							// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr volatile T& getValueRef() volatile noexcept {			// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr T* getValuePtr() noexcept {					// Custom function to get the pointer of the internal value directly
		return &value;
	}
	
	constexpr T* getValuePtr() volatile noexcept {			// Custom function to get the pointer of the internal value directly
		return &value;
	}

	constexpr SpecRealInd* getObjPtr() noexcept {					// Custom function to get a pointer to the object
		return this;
	}

	constexpr volatile SpecRealInd* getObjPtr() volatile noexcept {	// Custom function to get a pointer to the object
		return this;
	}

	constexpr TNV getRelPrecisionThreshold() const noexcept {					// Custom function to get the relative precision threshold used in comparisons
		return epsilonRel;
	}
	constexpr TNV getRelPrecisionThreshold() const volatile noexcept {			// Custom function to get the relative precision threshold used in comparisons
		return epsilonRel;
	}
	constexpr TNV getPrecisionThreshold() const noexcept {						// Custom function to get the precision threshold used in comparisons
		return getRelPrecisionThreshold();
	}
	constexpr TNV getPrecisionThreshold() const volatile noexcept {				// Custom function to get the precision threshold used in comparisons
		return getRelPrecisionThreshold();
	}

	constexpr void setRelPrecisionThreshold(const TNV pr) noexcept {			// Custom function to set the relative precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonRel = pr;
	}
	constexpr void setRelPrecisionThreshold(const TNV pr) volatile noexcept {	// Custom function to set the relative precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonRel = pr;
	}
	constexpr void setPrecisionThreshold(const TNV pr) noexcept {				// Custom function to set the precision threshold used in comparisons
		setRelPrecisionThreshold(pr);
	}
	constexpr void setPrecisionThreshold(const TNV pr) volatile noexcept {		// Custom function to set the precision threshold used in comparisons
		setRelPrecisionThreshold(pr);
	}

private:
	// Function to get the absolute value of a number
	static constexpr T abs(T n) noexcept {
		return (n >= static_cast<T>(0) ? n : -n);
	}

	// Comparison functions (ESP_RELATIVE)
	constexpr bool EqualComp(const T oval, const TNV epsRel) const noexcept {
		return abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel);
	}
	constexpr bool EqualComp(const T oval, const TNV epsRel) const volatile noexcept {
		return abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel);
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsRel) const noexcept {
		return abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel);
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsRel) const volatile noexcept {
		return abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel);
	}
	constexpr bool LessThanComp(const T oval, const TNV epsRel) const noexcept {
		return value < (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool LessThanComp(const T oval, const TNV epsRel) const volatile noexcept {
		return value < (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsRel) const noexcept {
		return value > (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsRel) const volatile noexcept {
		return value > (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsRel) const noexcept {
		return value <= (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsRel) const volatile noexcept {
		return value <= (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsRel) const noexcept {
		return value >= (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsRel) const volatile noexcept {
		return value >= (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}
};

// SpecRealInd partial specialization for EPS_TYPE = EPS_ULP
template<typename T>
class SpecRealInd<T, EPS_ULP> {
private:
	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;
public:
	using value_type = T;
	using value_type_nv = expr_type<T>;
	using ulp_type = typename std::remove_volatile<typename internal::UIntOfSize<sizeof(T), std::is_volatile<T>::value>::value_type>::type;
	using precision_type = ulp_type;
	static constexpr specreal_eps_type_t eps_type = EPS_ULP;

	static constexpr ulp_type defaultEpsilonUlp = static_cast<ulp_type>(8);

private:
	using TNV = value_type_nv;
	using RFP = internal::RawFloatPoint<T>;

	T value;						// The real value is stored here
	ulp_type epsilonUlp;			// The amount of ULP used as margin of error in the comparisons (must be positive)

public:
	// Overload of cast operator (default)
	//////////////////////////////////////
	constexpr operator T() const noexcept { return value; }				// Cast operator (EPS_ULP)

	constexpr operator T() const volatile noexcept { return value; }	// Cast operator (EPS_ULP)

	// Constructors and destructor
	//////////////////////////////

	SpecRealInd() noexcept = default;									// Default constructor (EPS_ULP)

	SpecRealInd(const T _value) noexcept :								// Default constructor (EPS_ULP)
	value(_value),
	epsilonUlp(defaultEpsilonUlp)
	{ }

	SpecRealInd(const T _value, const ulp_type _epsilon) noexcept :		// Custom constructor (EPS_ULP)
	value(_value),
	epsilonUlp(_epsilon)
	{ }

	~SpecRealInd() noexcept = default;									// Destructor (EPS_ULP)

	SpecRealInd(const SpecRealInd& other) = default;							// Copy constructor (EPS_ULP)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(const SpecRealInd<T, EPS_TYPE2>& other) noexcept :				// Copy constructor (other SpecReal type) (EPS_ULP)
	SpecRealInd(other.getValue())
	{}

	SpecRealInd(const SpecRealInd& other, const ulp_type _epsilon) noexcept :	// Copy constructor (alt version) (EPS_ULP)
	value(other.value),
	epsilonUlp(_epsilon)
	{ }

	SpecRealInd(SpecRealInd&& other) noexcept = default;						// Move constructor (EPS_ULP)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(SpecRealInd<T, EPS_TYPE2>&& other) noexcept :					// Move constructor (other SpecReal type) (EPS_ULP)
	SpecRealInd(other.getValue())
	{}

	SpecRealInd(SpecRealInd&& other, const ulp_type _epsilon) noexcept :		// Move constructor (alt version) (EPS_ULP)
	value(other.value),
	epsilonUlp(_epsilon)
	{ }

	// Overload of assignment operators
	////////////////////////////////////

	SpecRealInd& operator=(const SpecRealInd& other) noexcept = default;			// Copy assignment
//	SpecRealInd& operator=(const volatile SpecRealInd& other) volatile noexcept {	// Copy assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const SpecRealInd<T, EPS_TYPE2>& other) noexcept {					// Copy assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const volatile SpecRealInd<T, EPS_TYPE2>& other) volatile noexcept {	// Copy assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

	SpecRealInd& operator=(SpecRealInd&& other) noexcept = default;					// Move assignment
//	SpecRealInd& operator=(volatile SpecRealInd&& other) volatile noexcept {		// Move assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(SpecRealInd<T, EPS_TYPE2>&& other) noexcept {						// Move assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(volatile SpecRealInd<T, EPS_TYPE2>&& other) volatile noexcept {		// Move assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

//	SpecRealInd& operator=(SpecRealInd obj) noexcept {								// Direct assigment (not necessary and creates ambiguity)
//		value = obj.value;
//		return *this;
//	}
//	template<typename P>
//	SpecRealInd& operator=(P oval) noexcept {										// Direct assigment (not necessary and creates ambiguity)
//		value = oval;
//		return *this;
//	}

	// Overload of comparison operators (EPS_ULP)
	////////////////////////////////////////////////////////////

	constexpr bool operator==(const SpecRealInd& obj) const noexcept {				// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonUlp);
	}
	constexpr bool operator==(const SpecRealInd& obj) const volatile noexcept {		// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator==(const P& oval) const noexcept {						// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator==(const P& oval) const volatile noexcept {				// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const noexcept {				// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const volatile noexcept {		// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator!=(const P& oval) const noexcept {						// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator!=(const P& oval) const volatile noexcept {				// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const noexcept {				// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const volatile noexcept {		// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator<(const P& oval) const noexcept {						// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator<(const P& oval) const volatile noexcept {				// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const noexcept {				// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const volatile noexcept {		// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator>(const P& oval) const noexcept {						// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator>(const P& oval) const volatile noexcept {				// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const noexcept {				// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const volatile noexcept {		// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator<=(const P& oval) const noexcept {						// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator<=(const P& oval) const volatile noexcept {				// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const noexcept {				// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const volatile noexcept {		// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator>=(const P& oval) const noexcept {						// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator>=(const P& oval) const volatile noexcept {				// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonUlp);
	}

	// Overload of stream operators
	////////////////////////////////////

	constexpr std::ostream& operator<<(std::ostream& out) const noexcept {				// << ostream operator
		return out << value;
	}
	constexpr std::ostream& operator<<(std::ostream& out) const volatile noexcept {		// << ostream operator
		return out << value;
	}
	constexpr std::istream& operator>>(std::istream& is) noexcept {						// << istream operator
		return is >> value;
	}
	constexpr std::istream& operator>>(std::istream& is) volatile noexcept {			// << istream operator
		return is >> value;
	}

	// Overload of arithmetic operators
	////////////////////////////////////

	constexpr SpecRealInd operator+() const noexcept {									// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator+() const volatile noexcept {							// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator-() const noexcept {									// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd operator-() const volatile noexcept {							// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd& operator++() noexcept {										// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator++() volatile noexcept {								// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator++(int) const noexcept {								// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator++(int) const volatile noexcept {						// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
	}
	constexpr SpecRealInd& operator--() noexcept {										// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator--() volatile noexcept {								// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator--(int) const noexcept {								// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator--(int) const volatile noexcept {						// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
	}

	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) noexcept {				// += Addition assignment arithmetic operator
		value += obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) volatile noexcept {		// += Addition assignment arithmetic operator
		value += obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) noexcept {							// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) volatile noexcept {				// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const noexcept {			// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const volatile noexcept {	// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const noexcept {						// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const volatile noexcept {			// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) noexcept {				// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) volatile noexcept {		// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) noexcept {							// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) volatile noexcept {				// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const noexcept {			// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const volatile noexcept {	// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const noexcept {						// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const volatile noexcept {			// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) noexcept {				// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) volatile noexcept {		// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) noexcept {							// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) volatile noexcept {				// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const noexcept {			// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const volatile noexcept {	// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const noexcept {						// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const volatile noexcept {			// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) noexcept {				// /= Division assignment arithmetic operator
		value /= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) volatile noexcept {		// /= Division assignment arithmetic operator
		value /= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) noexcept {							// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) volatile noexcept {				// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const noexcept {			// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
 	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const volatile noexcept {	// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const noexcept {						// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
 	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const volatile noexcept {			// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
	}

	// Modulo and Binary operators not available on floating point numbers
	////////////////////////////////////

	// Overload of logical operators
	////////////////////////////////////

	constexpr bool operator!() const noexcept {									// ! NOT logical operator
		return !value;
	}
	constexpr bool operator!() const volatile noexcept {						// ! NOT logical operator
		return !value;
	}
	constexpr bool operator&&(const SpecRealInd& obj) const noexcept {			// && AND logical operator
		return (value && obj.value);
	}
	constexpr bool operator&&(const SpecRealInd& obj) const volatile noexcept {	// && AND logical operator
		return (value && obj.value);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const noexcept {					// && AND logical operator (other clases)
		return (value && oval);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const volatile noexcept {			// && AND logical operator (other clases)
		return (value && oval);
	}
	constexpr bool operator||(const SpecRealInd& obj) const noexcept {			// || OR logical operator
		return (value || obj.value);
	}
	constexpr bool operator||(const SpecRealInd& obj) const volatile noexcept {	// || OR logical operator
		return (value || obj.value);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const noexcept {					// || OR logical operator (other clases)
		return (value || oval);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const volatile noexcept {			// || OR logical operator (other clases)
		return (value || oval);
	}

	// Overload of other operators
	////////////////////////////////////

#ifdef SLSPECREAL_OVERLOAD_ADDRESSOF_OP
	constexpr T* operator&() noexcept {											// & Address-of operator (??)
		return &value;
	}
	constexpr T* operator&() volatile noexcept {								// & Address-of operator (??)
		return &value;
	}
#endif

	constexpr SpecRealInd operator,(SpecRealInd& obj) noexcept {				// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	constexpr SpecRealInd operator,(SpecRealInd& obj) volatile noexcept {		// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) noexcept {							// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) volatile noexcept {				// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}

	// The rest of member, pointer, bitwise, allocate and function operators make no sense for this class
	////////////////////////////////////

	// Additional functions
	////////////////////////////////////

	constexpr T getValue() const noexcept {							// Custom function to get the internal value directly
		return value;
	}

	constexpr T getValue() const volatile noexcept {				// Custom function to get the internal value directly
		return value;
	}

	constexpr T& getValueRef() noexcept {							// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr volatile T& getValueRef() volatile noexcept {			// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr T* getValuePtr() noexcept {					// Custom function to get the pointer of the internal value directly
		return &value;
	}
	
	constexpr T* getValuePtr() volatile noexcept {			// Custom function to get the pointer of the internal value directly
		return &value;
	}

	constexpr SpecRealInd* getObjPtr() noexcept {					// Custom function to get a pointer to the object
		return this;
	}

	constexpr volatile SpecRealInd* getObjPtr() volatile noexcept {	// Custom function to get a pointer to the object
		return this;
	}

	constexpr ulp_type getUlpPrecisionThreshold() const noexcept {					// Custom function to get the ULP precision threshold used in comparisons
		return epsilonUlp;
	}
	constexpr ulp_type getUlpPrecisionThreshold() const volatile noexcept {			// Custom function to get the ULP precision threshold used in comparisons
		return epsilonUlp;
	}
	constexpr ulp_type getPrecisionThreshold() const noexcept {						// Custom function to get the precision threshold used in comparisons
		return getUlpPrecisionThreshold();
	}
	constexpr ulp_type getPrecisionThreshold() const volatile noexcept {			// Custom function to get the precision threshold used in comparisons
		return getUlpPrecisionThreshold();
	}

	constexpr void setUlpPrecisionThreshold(const ulp_type pr) noexcept {			// Custom function to set the ULP precision threshold used in comparisons
		assert(pr >= 0);
		epsilonUlp = pr;
	}
	constexpr void setUlpPrecisionThreshold(const ulp_type pr) volatile noexcept {	// Custom function to set the ULP precision threshold used in comparisons
		assert(pr >= 0);
		epsilonUlp = pr;
	}
	constexpr void setPrecisionThreshold(const ulp_type pr) noexcept {				// Custom function to set the precision threshold used in comparisons
		setUlpPrecisionThreshold(pr);
	}
	constexpr void setPrecisionThreshold(const ulp_type pr) volatile noexcept {		// Custom function to set the precision threshold used in comparisons
		setUlpPrecisionThreshold(pr);
	}

private:
	// Function to get the absolute value of a number
	static constexpr T abs(T n) noexcept {
		return (n >= static_cast<T>(0) ? n : -n);
	}

	// Comparison functions (EPS_ULP)
	constexpr bool EqualComp(const T oval, const ulp_type epsUlp) const noexcept {
		return RFP(value).EqualComp(RFP(oval), epsUlp);
	}
	constexpr bool EqualComp(const T oval, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).EqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	constexpr bool NotEqualComp(const T oval, const ulp_type epsUlp) const noexcept {
		return RFP(value).NotEqualComp(RFP(oval), epsUlp);
	}
	constexpr bool NotEqualComp(const T oval, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).NotEqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	constexpr bool LessThanComp(const T oval, const ulp_type epsUlp) const noexcept {
		return RFP(value).LessThanComp(RFP(oval), epsUlp);
	}
	constexpr bool LessThanComp(const T oval, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).LessThanComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	constexpr bool GreaterThanComp(const T oval, const ulp_type epsUlp) const noexcept {
		return RFP(value).GreaterThanComp(RFP(oval), epsUlp);
	}
	constexpr bool GreaterThanComp(const T oval, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).GreaterThanComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	constexpr bool LessThanOrEqualComp(const T oval, const ulp_type epsUlp) const noexcept {
		return RFP(value).LessThanOrEqualComp(RFP(oval), epsUlp);
	}
	constexpr bool LessThanOrEqualComp(const T oval, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).LessThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const ulp_type epsUlp) const noexcept {
		return RFP(value).GreaterThanOrEqualComp(RFP(oval), epsUlp);
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).GreaterThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
};

// SpecRealInd partial specialization for EPS_TYPE = EPS_ABSOLUTE_AND_RELATIVE
template<typename T>
class SpecRealInd<T, EPS_ABSOLUTE_AND_RELATIVE> {
private:
	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;
public:
	using value_type = T;
	using value_type_nv = expr_type<T>;
	using precision_type = typename std::pair<value_type_nv, value_type_nv>;
	static constexpr specreal_eps_type_t eps_type = EPS_ABSOLUTE_AND_RELATIVE;

	static constexpr value_type_nv defaultEpsilonAbs = std::numeric_limits<value_type_nv>::epsilon() * static_cast<value_type_nv>(100.0);
	static constexpr value_type_nv defaultEpsilonRel = std::numeric_limits<value_type_nv>::epsilon() * static_cast<value_type_nv>(8.0);

private:
	using TNV = value_type_nv;

	T value;						// The real value is stored here
	TNV epsilonAbs;					// The absolute margin of error used in the comparisons (must be positive)
	TNV epsilonRel;					// The relative margin of error used in the comparisons (must be positive)

public:
	// Overload of cast operator (default)
	//////////////////////////////////////
	constexpr operator T() const noexcept { return value; }				// Cast operator (EPS_ABSOLUTE_AND_RELATIVE)

	constexpr operator T() const volatile noexcept { return value; }	// Cast operator (EPS_ABSOLUTE_AND_RELATIVE)

	// Constructors and destructor
	//////////////////////////////

	SpecRealInd() noexcept = default;									// Default constructor (EPS_ABSOLUTE_AND_RELATIVE)

	SpecRealInd(const T _value) noexcept :								// Default constructor (EPS_ABSOLUTE_AND_RELATIVE)
	value(_value),
	epsilonAbs(defaultEpsilonAbs),
	epsilonRel(defaultEpsilonRel)
	{ }

	template<typename P>
	SpecRealInd(const T _value, const P _epsilonAbs, const typename std::remove_reference<P>::type _epsilonRel) noexcept :				// Custom constructor (EPS_ABSOLUTE_AND_RELATIVE)
	value(_value),
	epsilonAbs(_epsilonAbs),
	epsilonRel(_epsilonRel)
	{ }

	~SpecRealInd() noexcept = default;									// Destructor (EPS_ABSOLUTE_AND_RELATIVE)

	SpecRealInd(const SpecRealInd& other) = default;					// Copy constructor (EPS_ABSOLUTE_AND_RELATIVE)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(const SpecRealInd<T, EPS_TYPE2>& other) noexcept :		// Copy constructor (other SpecReal type) (EPS_ABSOLUTE_AND_RELATIVE)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(const SpecRealInd& other, const P _epsilonAbs, const typename std::remove_reference<P>::type _epsilonRel) noexcept :	// Copy constructor (alt version) (EPS_ABSOLUTE_AND_RELATIVE)
	value(other.value),
	epsilonAbs(_epsilonAbs),
	epsilonRel(_epsilonRel)
	{ }

	SpecRealInd(SpecRealInd&& other) noexcept = default;				// Move constructor (EPS_ABSOLUTE_AND_RELATIVE)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(SpecRealInd<T, EPS_TYPE2>&& other) noexcept :			// Move constructor (other SpecReal type) (EPS_ABSOLUTE_AND_RELATIVE)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(SpecRealInd&& other, const P _epsilonAbs, const typename std::remove_reference<P>::type _epsilonRel) noexcept :			// Move constructor (alt version) (EPS_ABSOLUTE_AND_RELATIVE)
	value(other.value),
	epsilonAbs(_epsilonAbs),
	epsilonRel(_epsilonRel)
	{ }

	// Overload of assignment operators
	////////////////////////////////////

	SpecRealInd& operator=(const SpecRealInd& other) noexcept = default;			// Copy assignment
//	SpecRealInd& operator=(const volatile SpecRealInd& other) volatile noexcept {	// Copy assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const SpecRealInd<T, EPS_TYPE2>& other) noexcept {					// Copy assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const volatile SpecRealInd<T, EPS_TYPE2>& other) volatile noexcept {	// Copy assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

	SpecRealInd& operator=(SpecRealInd&& other) noexcept = default;					// Move assignment
//	SpecRealInd& operator=(volatile SpecRealInd&& other) volatile noexcept {		// Move assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(SpecRealInd<T, EPS_TYPE2>&& other) noexcept {						// Move assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(volatile SpecRealInd<T, EPS_TYPE2>&& other) volatile noexcept {		// Move assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

//	SpecRealInd& operator=(SpecRealInd obj) noexcept {								// Direct assigment (not necessary and creates ambiguity)
//		value = obj.value;
//		return *this;
//	}
//	template<typename P>
//	SpecRealInd& operator=(P oval) noexcept {										// Direct assigment (not necessary and creates ambiguity)
//		value = oval;
//		return *this;
//	}

	// Overload of comparison operators (EPS_ABSOLUTE_AND_RELATIVE)
	////////////////////////////////////////////////////////////

	constexpr bool operator==(const SpecRealInd& obj) const noexcept {				// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs, epsilonRel);
	}
	constexpr bool operator==(const SpecRealInd& obj) const volatile noexcept {		// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs, epsilonRel);
	}

	template<typename P>
	constexpr bool operator==(const P& oval) const noexcept {						// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs, epsilonRel);
	}
	template<typename P>
	constexpr bool operator==(const P& oval) const volatile noexcept {				// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const noexcept {				// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const volatile noexcept {		// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs, epsilonRel);
	}

	template<typename P>
	constexpr bool operator!=(const P& oval) const noexcept {						// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs, epsilonRel);
	}
	template<typename P>
	constexpr bool operator!=(const P& oval) const volatile noexcept {				// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const noexcept {				// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const volatile noexcept {		// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs, epsilonRel);
	}

	template<typename P>
	constexpr bool operator<(const P& oval) const noexcept {						// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs, epsilonRel);
	}
	template<typename P>
	constexpr bool operator<(const P& oval) const volatile noexcept {				// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const noexcept {				// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const volatile noexcept {		// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs, epsilonRel);
	}

	template<typename P>
	constexpr bool operator>(const P& oval) const noexcept {						// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs, epsilonRel);
	}
	template<typename P>
	constexpr bool operator>(const P& oval) const volatile noexcept {				// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const noexcept {				// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const volatile noexcept {		// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs, epsilonRel);
	}

	template<typename P>
	constexpr bool operator<=(const P& oval) const noexcept {						// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs, epsilonRel);
	}
	template<typename P>
	constexpr bool operator<=(const P& oval) const volatile noexcept {				// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs, epsilonRel);
	}

	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const noexcept {				// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs, epsilonRel);
	}
	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const volatile noexcept {		// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs, epsilonRel);
	}

	template<typename P>
	constexpr bool operator>=(const P& oval) const noexcept {						// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs, epsilonRel);
	}
	template<typename P>
	constexpr bool operator>=(const P& oval) const volatile noexcept {				// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs, epsilonRel);
	}

	// Overload of stream operators
	////////////////////////////////////

	constexpr std::ostream& operator<<(std::ostream& out) const noexcept {				// << ostream operator
		return out << value;
	}
	constexpr std::ostream& operator<<(std::ostream& out) const volatile noexcept {		// << ostream operator
		return out << value;
	}
	constexpr std::istream& operator>>(std::istream& is) noexcept {						// << istream operator
		return is >> value;
	}
	constexpr std::istream& operator>>(std::istream& is) volatile noexcept {			// << istream operator
		return is >> value;
	}

	// Overload of arithmetic operators
	////////////////////////////////////

	constexpr SpecRealInd operator+() const noexcept {									// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator+() const volatile noexcept {							// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator-() const noexcept {									// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd operator-() const volatile noexcept {							// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd& operator++() noexcept {										// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator++() volatile noexcept {								// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator++(int) const noexcept {								// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator++(int) const volatile noexcept {						// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
	}
	constexpr SpecRealInd& operator--() noexcept {										// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator--() volatile noexcept {								// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator--(int) const noexcept {								// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator--(int) const volatile noexcept {						// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
	}

	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) noexcept {				// += Addition assignment arithmetic operator
		value += obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) volatile noexcept {		// += Addition assignment arithmetic operator
		value += obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) noexcept {							// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) volatile noexcept {				// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const noexcept {			// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const volatile noexcept {	// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const noexcept {						// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const volatile noexcept {			// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) noexcept {				// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) volatile noexcept {		// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) noexcept {							// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) volatile noexcept {				// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const noexcept {			// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const volatile noexcept {	// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const noexcept {						// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const volatile noexcept {			// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) noexcept {				// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) volatile noexcept {		// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) noexcept {							// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) volatile noexcept {				// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const noexcept {			// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const volatile noexcept {	// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const noexcept {						// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const volatile noexcept {			// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) noexcept {				// /= Division assignment arithmetic operator
		value /= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) volatile noexcept {		// /= Division assignment arithmetic operator
		value /= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) noexcept {							// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) volatile noexcept {				// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const noexcept {			// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
 	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const volatile noexcept {	// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const noexcept {						// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
 	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const volatile noexcept {			// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
	}

	// Modulo and Binary operators not available on floating point numbers
	////////////////////////////////////

	// Overload of logical operators
	////////////////////////////////////

	constexpr bool operator!() const noexcept {									// ! NOT logical operator
		return !value;
	}
	constexpr bool operator!() const volatile noexcept {						// ! NOT logical operator
		return !value;
	}
	constexpr bool operator&&(const SpecRealInd& obj) const noexcept {			// && AND logical operator
		return (value && obj.value);
	}
	constexpr bool operator&&(const SpecRealInd& obj) const volatile noexcept {	// && AND logical operator
		return (value && obj.value);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const noexcept {					// && AND logical operator (other clases)
		return (value && oval);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const volatile noexcept {			// && AND logical operator (other clases)
		return (value && oval);
	}
	constexpr bool operator||(const SpecRealInd& obj) const noexcept {			// || OR logical operator
		return (value || obj.value);
	}
	constexpr bool operator||(const SpecRealInd& obj) const volatile noexcept {	// || OR logical operator
		return (value || obj.value);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const noexcept {					// || OR logical operator (other clases)
		return (value || oval);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const volatile noexcept {			// || OR logical operator (other clases)
		return (value || oval);
	}

	// Overload of other operators
	////////////////////////////////////

#ifdef SLSPECREAL_OVERLOAD_ADDRESSOF_OP
	constexpr T* operator&() noexcept {											// & Address-of operator (??)
		return &value;
	}
	constexpr T* operator&() volatile noexcept {								// & Address-of operator (??)
		return &value;
	}
#endif

	constexpr SpecRealInd operator,(SpecRealInd& obj) noexcept {				// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	constexpr SpecRealInd operator,(SpecRealInd& obj) volatile noexcept {		// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) noexcept {							// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) volatile noexcept {				// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}

	// The rest of member, pointer, bitwise, allocate and function operators make no sense for this class
	////////////////////////////////////

	// Additional functions
	////////////////////////////////////

	constexpr T getValue() const noexcept {							// Custom function to get the internal value directly
		return value;
	}

	constexpr T getValue() const volatile noexcept {				// Custom function to get the internal value directly
		return value;
	}

	constexpr T& getValueRef() noexcept {							// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr volatile T& getValueRef() volatile noexcept {			// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr T* getValuePtr() noexcept {					// Custom function to get the pointer of the internal value directly
		return &value;
	}
	
	constexpr T* getValuePtr() volatile noexcept {			// Custom function to get the pointer of the internal value directly
		return &value;
	}

	constexpr SpecRealInd* getObjPtr() noexcept {					// Custom function to get a pointer to the object
		return this;
	}

	constexpr volatile SpecRealInd* getObjPtr() volatile noexcept {	// Custom function to get a pointer to the object
		return this;
	}

	constexpr TNV getAbsPrecisionThreshold() const noexcept {																			// Custom function to get the absolute precision threshold used in comparisons
		return epsilonAbs;
	}
	constexpr TNV getAbsPrecisionThreshold() const volatile noexcept {																	// Custom function to get the absolute precision threshold used in comparisons
		return epsilonAbs;
	}
	constexpr TNV getRelPrecisionThreshold() const noexcept {																			// Custom function to get the relative precision threshold used in comparisons
		return epsilonRel;
	}
	constexpr TNV getRelPrecisionThreshold() const volatile noexcept {																	// Custom function to get the relative precision threshold used in comparisons
		return epsilonRel;
	}
	constexpr std::pair<TNV, TNV> getPrecisionThreshold() const noexcept {																// Custom function to get the precision threshold used in comparisons
		return std::pair<TNV, TNV>(getAbsPrecisionThreshold(), getRelPrecisionThreshold());
	}
	constexpr std::pair<TNV, TNV> getPrecisionThreshold() const volatile noexcept {														// Custom function to get the precision threshold used in comparisons
		return std::pair<TNV, TNV>(getAbsPrecisionThreshold(), getRelPrecisionThreshold());
	}

	constexpr void setAbsPrecisionThreshold(const TNV pr) noexcept {																	// Custom function to set the absolute precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonAbs = pr;
	}
	constexpr void setAbsPrecisionThreshold(const TNV pr) volatile noexcept {															// Custom function to set the absolute precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonAbs = pr;
	}
	constexpr void setRelPrecisionThreshold(const TNV pr) noexcept {																	// Custom function to set the relative precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonRel = pr;
	}
	constexpr void setRelPrecisionThreshold(const TNV pr) volatile noexcept {															// Custom function to set the relative precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonRel = pr;
	}
	constexpr void setPrecisionThreshold(const TNV prAbs, const typename std::remove_reference<TNV>::type prRel) noexcept {				// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(prAbs);
		setRelPrecisionThreshold(prRel);
	}
	constexpr void setPrecisionThreshold(const TNV prAbs, const typename std::remove_reference<TNV>::type prRel) volatile noexcept {	// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(prAbs);
		setRelPrecisionThreshold(prRel);
	}
	constexpr void setPrecisionThreshold(const std::pair<TNV, TNV> pr) noexcept {														// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(pr.first);
		setRelPrecisionThreshold(pr.second);
	}
	constexpr void setPrecisionThreshold(const std::pair<TNV, TNV> pr) volatile noexcept {												// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(pr.first);
		setRelPrecisionThreshold(pr.second);
	}

private:
	// Function to get the absolute value of a number
	static constexpr T abs(T n) noexcept {
		return (n >= static_cast<T>(0) ? n : -n);
	}

	// Comparison functions (ESP_ABSOLUTE_AND_RELATIVE)
	constexpr bool EqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const noexcept {
		return (abs(value - oval) <= epsAbs) || (abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool EqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const volatile noexcept {
		return (abs(value - oval) <= epsAbs) || (abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const noexcept {
		return (abs(value - oval) > epsAbs) && (abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const volatile noexcept {
		return (abs(value - oval) > epsAbs) && (abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel));
	}
	constexpr bool LessThanComp(const T oval, const TNV epsAbs, const TNV epsRel) const noexcept {
		return (value < (oval - epsAbs)) && (value < (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}
	constexpr bool LessThanComp(const T oval, const TNV epsAbs, const TNV epsRel) const volatile noexcept {
		return (value < (oval - epsAbs)) && (value < (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsAbs, const TNV epsRel) const noexcept {
		return (value > (oval + epsAbs)) && (value > (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsAbs, const TNV epsRel) const volatile noexcept {
		return (value > (oval + epsAbs)) && (value > (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const noexcept {
		return (value <= (oval + epsAbs)) || (value <= (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const volatile noexcept {
		return (value <= (oval + epsAbs)) || (value <= (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const noexcept {
		return (value >= (oval - epsAbs)) || (value >= (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel) const volatile noexcept {
		return (value >= (oval - epsAbs)) || (value >= (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}
};

// SpecRealInd partial specialization for EPS_TYPE = EPS_ABSOLUTE_AND_ULP
template<typename T>
class SpecRealInd<T, EPS_ABSOLUTE_AND_ULP> {
private:
	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;
public:
	using value_type = T;
	using value_type_nv = expr_type<T>;
	using ulp_type = typename std::remove_volatile<typename internal::UIntOfSize<sizeof(T), std::is_volatile<T>::value>::value_type>::type;
	using precision_type = typename std::pair<value_type_nv, ulp_type>;
	static constexpr specreal_eps_type_t eps_type = EPS_ABSOLUTE_AND_ULP;

	static constexpr value_type_nv defaultEpsilonAbs = std::numeric_limits<value_type_nv>::epsilon() * static_cast<value_type_nv>(100.0);
	static constexpr ulp_type defaultEpsilonUlp = static_cast<ulp_type>(8);

private:
	using TNV = value_type_nv;
	using RFP = internal::RawFloatPoint<T>;

	T value;						// The real value is stored here
	TNV epsilonAbs;					// The absolute margin of error used in the comparisons (must be positive)
	ulp_type epsilonUlp;			// The amount of ULP used as margin of error in the comparisons (must be positive)

public:
	// Overload of cast operator (default)
	//////////////////////////////////////
	constexpr operator T() const noexcept { return value; }				// Cast operator (EPS_ABSOLUTE_AND_ULP)

	constexpr operator T() const volatile noexcept { return value; }	// Cast operator (EPS_ABSOLUTE_AND_ULP)

	// Constructors and destructor
	//////////////////////////////

	SpecRealInd() noexcept = default;									// Default constructor (EPS_ABSOLUTE_AND_ULP)

	SpecRealInd(const T _value) noexcept :								// Default constructor (EPS_ABSOLUTE_AND_ULP)
	value(_value),
	epsilonAbs(defaultEpsilonAbs),
	epsilonUlp(defaultEpsilonUlp)
	{ }

	template<typename P>
	SpecRealInd(const T _value, const P _epsilonAbs, const ulp_type _epsilonUlp) noexcept :				// Custom constructor (EPS_ABSOLUTE_AND_ULP)
	value(_value),
	epsilonAbs(_epsilonAbs),
	epsilonUlp(_epsilonUlp)
	{ }

	~SpecRealInd() noexcept = default;									// Destructor (EPS_ABSOLUTE_AND_ULP)

	SpecRealInd(const SpecRealInd& other) = default;					// Copy constructor (EPS_ABSOLUTE_AND_ULP)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(const SpecRealInd<T, EPS_TYPE2>& other) noexcept :		// Copy constructor (other SpecReal type) (EPS_ABSOLUTE_AND_ULP)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(const SpecRealInd& other, const P _epsilonAbs, const ulp_type _epsilonUlp) noexcept :	// Copy constructor (alt version) (EPS_ABSOLUTE_AND_ULP)
	value(other.value),
	epsilonAbs(_epsilonAbs),
	epsilonUlp(_epsilonUlp)
	{ }

	SpecRealInd(SpecRealInd&& other) noexcept = default;				// Move constructor (EPS_ABSOLUTE_AND_ULP)

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd(SpecRealInd<T, EPS_TYPE2>&& other) noexcept :			// Move constructor (other SpecReal type) (EPS_ABSOLUTE_AND_ULP)
	SpecRealInd(other.getValue())
	{}

	template<typename P>
	SpecRealInd(SpecRealInd&& other, const P _epsilonAbs, const ulp_type _epsilonUlp) noexcept :		// Move constructor (alt version) (EPS_ABSOLUTE_AND_ULP)
	value(other.value),
	epsilonAbs(_epsilonAbs),
	epsilonUlp(_epsilonUlp)
	{ }

	// Overload of assignment operators
	////////////////////////////////////

	SpecRealInd& operator=(const SpecRealInd& other) noexcept = default;			// Copy assignment
//	SpecRealInd& operator=(const volatile SpecRealInd& other) volatile noexcept {	// Copy assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const SpecRealInd<T, EPS_TYPE2>& other) noexcept {					// Copy assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(const volatile SpecRealInd<T, EPS_TYPE2>& other) volatile noexcept {	// Copy assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

	SpecRealInd& operator=(SpecRealInd&& other) noexcept = default;					// Move assignment
//	SpecRealInd& operator=(volatile SpecRealInd&& other) volatile noexcept {		// Move assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecRealInd&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(SpecRealInd<T, EPS_TYPE2>&& other) noexcept {						// Move assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2>
	SpecRealInd& operator=(volatile SpecRealInd<T, EPS_TYPE2>&& other) volatile noexcept {		// Move assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecRealInd&>(*this);
	}

//	SpecRealInd& operator=(SpecRealInd obj) noexcept {								// Direct assigment (not necessary and creates ambiguity)
//		value = obj.value;
//		return *this;
//	}
//	template<typename P>
//	SpecRealInd& operator=(P oval) noexcept {										// Direct assigment (not necessary and creates ambiguity)
//		value = oval;
//		return *this;
//	}

	// Overload of comparison operators (EPS_ABSOLUTE_AND_ULP)
	////////////////////////////////////////////////////////////

	constexpr bool operator==(const SpecRealInd& obj) const noexcept {				// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs, epsilonUlp);
	}
	constexpr bool operator==(const SpecRealInd& obj) const volatile noexcept {		// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator==(const P& oval) const noexcept {						// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator==(const P& oval) const volatile noexcept {				// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const noexcept {				// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator!=(const SpecRealInd& obj) const volatile noexcept {		// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator!=(const P& oval) const noexcept {						// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator!=(const P& oval) const volatile noexcept {				// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const noexcept {				// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator<(const SpecRealInd& obj) const volatile noexcept {		// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator<(const P& oval) const noexcept {						// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator<(const P& oval) const volatile noexcept {				// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const noexcept {				// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator>(const SpecRealInd& obj) const volatile noexcept {		// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator>(const P& oval) const noexcept {						// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator>(const P& oval) const volatile noexcept {				// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const noexcept {				// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator<=(const SpecRealInd& obj) const volatile noexcept {		// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator<=(const P& oval) const noexcept {						// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator<=(const P& oval) const volatile noexcept {				// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const noexcept {				// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator>=(const SpecRealInd& obj) const volatile noexcept {		// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator>=(const P& oval) const noexcept {						// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator>=(const P& oval) const volatile noexcept {				// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs, epsilonUlp);
	}

	// Overload of stream operators
	////////////////////////////////////

	constexpr std::ostream& operator<<(std::ostream& out) const noexcept {				// << ostream operator
		return out << value;
	}
	constexpr std::ostream& operator<<(std::ostream& out) const volatile noexcept {		// << ostream operator
		return out << value;
	}
	constexpr std::istream& operator>>(std::istream& is) noexcept {						// << istream operator
		return is >> value;
	}
	constexpr std::istream& operator>>(std::istream& is) volatile noexcept {			// << istream operator
		return is >> value;
	}

	// Overload of arithmetic operators
	////////////////////////////////////

	constexpr SpecRealInd operator+() const noexcept {									// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator+() const volatile noexcept {							// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecRealInd operator-() const noexcept {									// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd operator-() const volatile noexcept {							// (-) Unary minus (additive inverse) arithmetic operator
		return SpecRealInd{-value};
	}
	constexpr SpecRealInd& operator++() noexcept {										// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator++() volatile noexcept {								// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator++(int) const noexcept {								// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator++(int) const volatile noexcept {						// ++ (Postfix) Increment arithmetic operator
		return SpecRealInd{value+static_cast<T>(1.0)};
	}
	constexpr SpecRealInd& operator--() noexcept {										// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecRealInd& operator--() volatile noexcept {								// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator--(int) const noexcept {								// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
 	}
	constexpr SpecRealInd operator--(int) const volatile noexcept {						// -- (Postfix) Decrement arithmetic operator
		return SpecRealInd{value-static_cast<T>(1.0)};
	}

	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) noexcept {				// += Addition assignment arithmetic operator
		value += obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator+=(const SpecRealInd& obj) volatile noexcept {		// += Addition assignment arithmetic operator
		value += obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) noexcept {							// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator+=(const P& oval) volatile noexcept {				// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const noexcept {			// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	constexpr SpecRealInd operator+(const SpecRealInd& obj) const volatile noexcept {	// + Addition arithmetic operator
		return SpecRealInd{value+obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const noexcept {						// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator+(const P& oval) const volatile noexcept {			// + Addition arithmetic operator (other classes)
		return SpecRealInd{value+(T)oval};
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) noexcept {				// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator-=(const SpecRealInd& obj) volatile noexcept {		// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) noexcept {							// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator-=(const P& oval) volatile noexcept {				// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const noexcept {			// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	constexpr SpecRealInd operator-(const SpecRealInd& obj) const volatile noexcept {	// - Subtraction arithmetic operator
		return SpecRealInd{value-obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const noexcept {						// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator-(const P& oval) const volatile noexcept {			// - Subtraction arithmetic operator (other classes)
		return SpecRealInd{value-(T)oval};
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) noexcept {				// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator*=(const SpecRealInd& obj) volatile noexcept {		// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) noexcept {							// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator*=(const P& oval) volatile noexcept {				// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const noexcept {			// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	constexpr SpecRealInd operator*(const SpecRealInd& obj) const volatile noexcept {	// * Multiplication arithmetic operator
		return SpecRealInd{value*obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const noexcept {						// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	template<typename P>
	constexpr SpecRealInd operator*(const P& oval) const volatile noexcept {			// * Multiplication arithmetic operator (other classes)
		return SpecRealInd{value*(T)oval};
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) noexcept {				// /= Division assignment arithmetic operator
		value /= obj.value;
		return *this;
	}
	constexpr SpecRealInd& operator/=(const SpecRealInd& obj) volatile noexcept {		// /= Division assignment arithmetic operator
		value /= obj.value;
		return const_cast<SpecRealInd&>(*this);
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) noexcept {							// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecRealInd& operator/=(const P& oval) volatile noexcept {				// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return const_cast<SpecRealInd&>(*this);
	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const noexcept {			// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
 	}
	constexpr SpecRealInd operator/(const SpecRealInd& obj) const volatile noexcept {	// / Division arithmetic operator
		return SpecRealInd{value/obj.value};
	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const noexcept {						// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
 	}
	template<typename P>
	constexpr SpecRealInd operator/(const P& oval) const volatile noexcept {			// / Division arithmetic operator (other classes)
		return SpecRealInd{value/(T)oval};
	}

	// Modulo and Binary operators not available on floating point numbers
	////////////////////////////////////

	// Overload of logical operators
	////////////////////////////////////

	constexpr bool operator!() const noexcept {									// ! NOT logical operator
		return !value;
	}
	constexpr bool operator!() const volatile noexcept {						// ! NOT logical operator
		return !value;
	}
	constexpr bool operator&&(const SpecRealInd& obj) const noexcept {			// && AND logical operator
		return (value && obj.value);
	}
	constexpr bool operator&&(const SpecRealInd& obj) const volatile noexcept {	// && AND logical operator
		return (value && obj.value);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const noexcept {					// && AND logical operator (other clases)
		return (value && oval);
	}
	template<typename P>
	constexpr bool operator&&(const P& oval) const volatile noexcept {			// && AND logical operator (other clases)
		return (value && oval);
	}
	constexpr bool operator||(const SpecRealInd& obj) const noexcept {			// || OR logical operator
		return (value || obj.value);
	}
	constexpr bool operator||(const SpecRealInd& obj) const volatile noexcept {	// || OR logical operator
		return (value || obj.value);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const noexcept {					// || OR logical operator (other clases)
		return (value || oval);
	}
	template<typename P>
	constexpr bool operator||(const P& oval) const volatile noexcept {			// || OR logical operator (other clases)
		return (value || oval);
	}

	// Overload of other operators
	////////////////////////////////////

#ifdef SLSPECREAL_OVERLOAD_ADDRESSOF_OP
	constexpr T* operator&() noexcept {											// & Address-of operator (??)
		return &value;
	}
	constexpr T* operator&() volatile noexcept {								// & Address-of operator (??)
		return &value;
	}
#endif

	constexpr SpecRealInd operator,(SpecRealInd& obj) noexcept {				// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	constexpr SpecRealInd operator,(SpecRealInd& obj) volatile noexcept {		// , Comma operator
		return SpecRealInd{(value, (obj.value))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) noexcept {							// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}
	template<typename P>
	constexpr SpecRealInd operator,(P& oval) volatile noexcept {				// , Comma operator
		return SpecRealInd{(value, (T)(oval))};
	}

	// The rest of member, pointer, bitwise, allocate and function operators make no sense for this class
	////////////////////////////////////

	// Additional functions
	////////////////////////////////////

	constexpr T getValue() const noexcept {							// Custom function to get the internal value directly
		return value;
	}

	constexpr T getValue() const volatile noexcept {				// Custom function to get the internal value directly
		return value;
	}

	constexpr T& getValueRef() noexcept {							// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr volatile T& getValueRef() volatile noexcept {			// Custom function to get the internal value by reference directly
		return value;
	}

	constexpr T* getValuePtr() noexcept {					// Custom function to get the pointer of the internal value directly
		return &value;
	}
	
	constexpr T* getValuePtr() volatile noexcept {			// Custom function to get the pointer of the internal value directly
		return &value;
	}

	constexpr SpecRealInd* getObjPtr() noexcept {					// Custom function to get a pointer to the object
		return this;
	}

	constexpr volatile SpecRealInd* getObjPtr() volatile noexcept {	// Custom function to get a pointer to the object
		return this;
	}

	constexpr TNV getAbsPrecisionThreshold() const noexcept {										// Custom function to get the absolute precision threshold used in comparisons
		return epsilonAbs;
	}
	constexpr TNV getAbsPrecisionThreshold() const volatile noexcept {								// Custom function to get the absolute precision threshold used in comparisons
		return epsilonAbs;
	}
	constexpr ulp_type getUlpPrecisionThreshold() const noexcept {									// Custom function to get the ULP precision threshold used in comparisons
		return epsilonUlp;
	}
	constexpr ulp_type getUlpPrecisionThreshold() const volatile noexcept {							// Custom function to get the ULP precision threshold used in comparisons
		return epsilonUlp;
	}
	constexpr std::pair<TNV, ulp_type> getPrecisionThreshold() const noexcept {						// Custom function to get the precision threshold used in comparisons
		return std::pair<TNV, TNV>(getAbsPrecisionThreshold(), getUlpPrecisionThreshold());
	}
	constexpr std::pair<TNV, ulp_type> getPrecisionThreshold() const volatile noexcept {			// Custom function to get the precision threshold used in comparisons
		return std::pair<TNV, TNV>(getAbsPrecisionThreshold(), getUlpPrecisionThreshold());
	}

	constexpr void setAbsPrecisionThreshold(const TNV pr) noexcept {								// Custom function to set the absolute precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonAbs = pr;
	}
	constexpr void setAbsPrecisionThreshold(const TNV pr) volatile noexcept {						// Custom function to set the absolute precision threshold used in comparisons
		assert(pr >= static_cast<TNV>(0.0));
		epsilonAbs = pr;
	}
	constexpr void setUlpPrecisionThreshold(const ulp_type pr) noexcept {							// Custom function to set the ULP precision threshold used in comparisons
		assert(pr >= 0);
		epsilonUlp = pr;
	}
	constexpr void setUlpPrecisionThreshold(const ulp_type pr) volatile noexcept {					// Custom function to set the ULP precision threshold used in comparisons
		assert(pr >= 0);
		epsilonUlp = pr;
	}
	constexpr void setPrecisionThreshold(const TNV prAbs, const ulp_type prUlp) noexcept {			// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(prAbs);
		setUlpPrecisionThreshold(prUlp);
	}
	constexpr void setPrecisionThreshold(const TNV prAbs, const ulp_type prUlp) volatile noexcept {	// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(prAbs);
		setUlpPrecisionThreshold(prUlp);
	}
	constexpr void setPrecisionThreshold(const std::pair<TNV, ulp_type> pr) noexcept {				// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(pr.first);
		setUlpPrecisionThreshold(pr.second);
	}
	constexpr void setPrecisionThreshold(const std::pair<TNV, ulp_type> pr) volatile noexcept {		// Custom function to set the precision threshold used in comparisons
		setAbsPrecisionThreshold(pr.first);
		setUlpPrecisionThreshold(pr.second);
	}

private:
	// Function to get the absolute value of a number
	static constexpr T abs(T n) noexcept {
		return (n >= static_cast<T>(0) ? n : -n);
	}

	// Comparison functions (EPS_ABSOLUTE_AND_ULP)
	constexpr bool EqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const noexcept {
		return (abs(value - oval) <= epsAbs) || (RFP(value).EqualComp(RFP(oval), epsUlp));
	}
	constexpr bool EqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const volatile noexcept {
		return (abs(value - oval) <= epsAbs) || (RFP(const_cast<const T&>(value)).EqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const noexcept {
		return (abs(value - oval) > epsAbs) && (RFP(value).NotEqualComp(RFP(oval), epsUlp));
	}
	constexpr bool NotEqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const volatile noexcept {
		return (abs(value - oval) > epsAbs) && (RFP(const_cast<const T&>(value)).NotEqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	constexpr bool LessThanComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const noexcept {
		return (value < (oval - epsAbs)) && (RFP(value).LessThanComp(RFP(oval), epsUlp));
	}
	constexpr bool LessThanComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const volatile noexcept {
		return (value < (oval - epsAbs)) && (RFP(const_cast<const T&>(value)).LessThanComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const noexcept {
		return (value > (oval + epsAbs)) && (RFP(value).GreaterThanComp(RFP(oval), epsUlp));
	}
	constexpr bool GreaterThanComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const volatile noexcept {
		return (value > (oval + epsAbs)) && (RFP(const_cast<const T&>(value)).GreaterThanComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const noexcept {
		return (value <= (oval + epsAbs)) || (RFP(value).LessThanOrEqualComp(RFP(oval), epsUlp));
	}
	constexpr bool LessThanOrEqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const volatile noexcept {
		return (value <= (oval + epsAbs)) || (RFP(const_cast<const T&>(value)).LessThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const noexcept {
		return (value >= (oval - epsAbs)) || (RFP(value).GreaterThanOrEqualComp(RFP(oval), epsUlp));
	}
	constexpr bool GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const ulp_type epsUlp) const volatile noexcept {
		return (value >= (oval - epsAbs)) || (RFP(const_cast<const T&>(value)).GreaterThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
};

// External overload of operators seems not necesary and can creaty ambiguity
////////////////////////////////////

} //namespace SpecLib

#endif // __SPECREALIND_H
