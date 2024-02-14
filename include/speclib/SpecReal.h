/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     SpecReal.h
/// \brief    SpecReal class wraper to speculate on floating point types with global margin of error
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef __SPECREAL_H
#define __SPECREAL_H

#include <type_traits>
#include <ostream>
#include <istream>
#include <limits>
#include <utility>
#include <cassert>
#include "speclib/SpecRealComm.h"

namespace SpecLib {

template<typename T, const specreal_eps_type_t EPS_TYPE = EPS_ABSOLUTE, const int EPS_ID = -1>
class SpecReal {
private:
	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;
public:
	using value_type = T;
	using value_type_nv = expr_type<T>;
	using ulp_type = typename std::remove_volatile<typename internal::UIntOfSize<sizeof(T), std::is_volatile<T>::value>::value_type>::type;
	using precision_type = typename std::conditional<(EPS_TYPE == EPS_ABSOLUTE || EPS_TYPE == EPS_RELATIVE), value_type_nv,
										std::conditional<(EPS_TYPE == EPS_ULP), ulp_type,
											std::conditional<(EPS_TYPE == EPS_ABSOLUTE_AND_RELATIVE), std::pair<value_type_nv, value_type_nv>,
												std::pair<value_type_nv, ulp_type>>>>::type;
	static constexpr specreal_eps_type_t eps_type = EPS_TYPE;
	static constexpr int eps_id = EPS_ID;

	static constexpr value_type_nv defaultEpsilonAbs = std::numeric_limits<value_type_nv>::epsilon() * static_cast<value_type_nv>(100.0);
	static constexpr value_type_nv defaultEpsilonRel = std::numeric_limits<value_type_nv>::epsilon() * static_cast<value_type_nv>(8.0);
	static constexpr ulp_type defaultEpsilonUlp = static_cast<ulp_type>(8);

private:
	using TNV = value_type_nv;
	using RFP = internal::RawFloatPoint<T>;

	T value;						// The real value is stored here
	static TNV epsilonAbs;			// The absolute margin of error used in the comparisons (must be positive)
	static TNV epsilonRel;			// The relative margin of error used in the comparisons (must be positive)
	static ulp_type epsilonUlp;		// The amount of ULP used as margin of error in the comparisons (must be positive)

public:
	// Overload of cast operator (default)
	////////////////////////////////////
	constexpr operator T() const noexcept { return value; }				// Cast operator

	constexpr operator T() const volatile noexcept { return value; }	// Cast operator

	// Constructors and destructor
	////////////////////////////////////

	SpecReal() noexcept = default;										// Default constructor

	SpecReal(const T _value) noexcept :									// Default constructor
	value(_value)
	{ }

	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE || EPST == EPS_RELATIVE>>
	SpecReal(const T _value, const P _epsilon) noexcept :															// Custom constructor (EPS_ABSOLUTE and EPS_RELATIVE)
	value(_value)
	{
		setPrecisionThreshold((TNV)_epsilon);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename Z = std::enable_if_t<EPST == EPS_ULP>>
	SpecReal(const T _value, const ulp_type _epsilon) noexcept :													// Custom constructor (EPS_ULP)
	value(_value)
	{
		setPrecisionThreshold(_epsilon);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE>>
	SpecReal(const T _value, const P _epsilonAbs, const typename std::remove_reference<P>::type _epsilonRel) noexcept :	// Custom constructor (EPS_ABSOLUTE_AND_RELATIVE)
	value(_value)
	{
		setPrecisionThreshold((TNV)_epsilonAbs, (TNV)_epsilonRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP>>
	SpecReal(const T _value, const P _epsilonAbs, const ulp_type _epsilonUlp) noexcept :								// Custom constructor (EPS_ABSOLUTE_AND_ULP)
	value(_value)
	{
		setPrecisionThreshold((TNV)_epsilonAbs, _epsilonUlp);
	}

	~SpecReal() noexcept = default;										// Destructor

	SpecReal(const SpecReal& other) = default;							// Copy constructor

	template<const specreal_eps_type_t EPS_TYPE2, const int EPS_ID2>
	SpecReal(const SpecReal<T, EPS_TYPE2, EPS_ID2>& other) noexcept :	// Copy constructor (other SpecReal type)
	SpecReal(other.getValue())
	{}

	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE || EPST == EPS_RELATIVE>>
	SpecReal(const SpecReal& other, const P _epsilon) noexcept :															// Copy constructor (EPS_ABSOLUTE and EPS_RELATIVE) (alt version)
	value(other.value)
	{
		setPrecisionThreshold((TNV)_epsilon);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename Z = std::enable_if_t<EPST == EPS_ULP>>
	SpecReal(const SpecReal& other, const ulp_type _epsilon) noexcept :														// Copy constructor (EPS_ULP) (alt version)
	value(other.value)
	{
		setPrecisionThreshold(_epsilon);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE>>
	SpecReal(const SpecReal& other, const P _epsilonAbs, const typename std::remove_reference<P>::type _epsilonRel) noexcept :	// Copy constructor (alt version) (EPS_ABSOLUTE_AND_RELATIVE)
	value(other.value)
	{
		setPrecisionThreshold((TNV)_epsilonAbs, (TNV)_epsilonRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP>>
	SpecReal(const SpecReal& other, const P _epsilonAbs, const ulp_type _epsilonUlp) noexcept :									// Copy constructor (alt version) (EPS_ABSOLUTE_AND_ULP)
	value(other.value)
	{
		setPrecisionThreshold((TNV)_epsilonAbs, _epsilonUlp);
	}

	SpecReal(SpecReal&& other) noexcept = default;						// Move constructor

	template<const specreal_eps_type_t EPS_TYPE2, const int EPS_ID2>
	SpecReal(SpecReal<T, EPS_TYPE2, EPS_ID2>&& other) noexcept :		// Move constructor (other SpecReal type)
	SpecReal(other.getValue())
	{}

	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE || EPST == EPS_RELATIVE>>
	SpecReal(SpecReal&& other, const P _epsilon) noexcept :																	// Move constructor (EPS_ABSOLUTE and EPS_RELATIVE) (alt version)
	value(other.value)
	{
		setPrecisionThreshold((TNV)_epsilon);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename Z = std::enable_if_t<EPST == EPS_ULP>>
	SpecReal(SpecReal&& other, const ulp_type _epsilon) noexcept :															// Move constructor (alt version) (EPS_ULP)
	value(other.value)
	{
		setPrecisionThreshold(_epsilon);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE>>
	SpecReal(SpecReal&& other, const P _epsilonAbs, const typename std::remove_reference<P>::type _epsilonRel) noexcept :		// Move constructor (EPS_ABSOLUTE_AND_RELATIVE) (alt version)
	value(other.value)
	{
		setPrecisionThreshold((TNV)_epsilonAbs, (TNV)_epsilonRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE, typename P, typename Z = std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP>>
	SpecReal(SpecReal&& other, const P _epsilonAbs, const ulp_type _epsilonUlp) noexcept :										// Move constructor (EPS_ABSOLUTE_AND_ULP) (alt version)
	value(other.value)
	{
		setPrecisionThreshold((TNV)_epsilonAbs, _epsilonUlp);
	}

	// Overload of assignment operators
	////////////////////////////////////

	SpecReal& operator=(const SpecReal& other) noexcept = default;									// Copy assignment
//	SpecReal& operator=(const volatile SpecReal& other) volatile noexcept {							// Copy assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecReal&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2, const int EPS_ID2>
	SpecReal& operator=(const SpecReal<T, EPS_TYPE2, EPS_ID2>& other) noexcept {					// Copy assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2, const int EPS_ID2>
	SpecReal& operator=(const volatile SpecReal<T, EPS_TYPE2, EPS_ID2>& other) volatile noexcept {	// Copy assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecReal&>(*this);
	}

	SpecReal& operator=(SpecReal&& other) noexcept = default;					// Move assignment
//	SpecReal& operator=(volatile SpecReal&& other) volatile noexcept {			// Move assignment (volatile, disabled to allow class to be trivially_copyable)
//		value = other.value;
//		return const_cast<SpecReal&>(*this);
//	}

	template<const specreal_eps_type_t EPS_TYPE2, const int EPS_ID2>
	SpecReal& operator=(SpecReal<T, EPS_TYPE2, EPS_ID2>&& other) noexcept {							// Move assignment (other SpecReal type)
		value = other.getValue();
		return *this;
	}
	template<const specreal_eps_type_t EPS_TYPE2, const int EPS_ID2>
	SpecReal& operator=(volatile SpecReal<T, EPS_TYPE2, EPS_ID2>&& other) volatile noexcept {		// Move assignment (other SpecReal type)
		value = other.getValue();
		return const_cast<SpecReal&>(*this);
	}

//	SpecReal& operator=(SpecReal obj) noexcept {								// Direct assigment (not necessary and creates ambiguity)
//		value = obj.value;
//		return *this;
//	}
//	template<typename P>
//	SpecReal& operator=(P oval) noexcept {										// Direct assigment (not necessary and creates ambiguity)
//		value = oval;
//		return *this;
//	}

	// Overload of comparison operators
	////////////////////////////////////

	constexpr bool operator==(const SpecReal& obj) const noexcept {					// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}
	constexpr bool operator==(const SpecReal& obj) const volatile noexcept {		// == Equal-to comparison operator
		return EqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator==(const P& oval) const noexcept {						// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator==(const P& oval) const volatile noexcept {				// == Equal-to comparison operator (other classes)
		return EqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator!=(const SpecReal& obj) const noexcept {					// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator!=(const SpecReal& obj) const volatile noexcept {		// != Not-equal-to comparison operator
		return NotEqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator!=(const P& oval) const noexcept {						// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator!=(const P& oval) const volatile noexcept {				// != Not-equal-to comparison operator (other classes)
		return NotEqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator<(const SpecReal& obj) const noexcept {					// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator<(const SpecReal& obj) const volatile noexcept {			// < Less-than comparison operator
		return LessThanComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator<(const P& oval) const noexcept {						// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator<(const P& oval) const volatile noexcept {				// < Less-than comparison operator (other classes)
		return LessThanComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator>(const SpecReal& obj) const noexcept {					// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator>(const SpecReal& obj) const volatile noexcept {			// > Greater-than comparison operator
		return GreaterThanComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator>(const P& oval) const noexcept {						// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator>(const P& oval) const volatile noexcept {				// > Greater-than comparison operator (other classes)
		return GreaterThanComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator<=(const SpecReal& obj) const noexcept {					// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator<=(const SpecReal& obj) const volatile noexcept {		// <= Less-than-or-equal-to comparison operator
		return LessThanOrEqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator<=(const P& oval) const noexcept {						// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator<=(const P& oval) const volatile noexcept {				// <= Less-than-or-equal-to comparison operator (other classes)
		return LessThanOrEqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename R = T>
	constexpr bool operator>=(const SpecReal& obj) const noexcept {					// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename R = T>
	constexpr bool operator>=(const SpecReal& obj) const volatile noexcept {		// >= Greater-than-or-equal-to comparison operator
		return GreaterThanOrEqualComp(obj.value, epsilonAbs, epsilonRel, epsilonUlp);
	}

	template<typename P>
	constexpr bool operator>=(const P& oval) const noexcept {						// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}
	template<typename P>
	constexpr bool operator>=(const P& oval) const volatile noexcept {				// >= Greater-than-or-equal-to comparison operator (other classes)
		return GreaterThanOrEqualComp(oval, epsilonAbs, epsilonRel, epsilonUlp);
	}

	// Overload of stream operators
	////////////////////////////////////

	constexpr std::ostream& operator<<(std::ostream& out) const noexcept {			// << ostream operator
		return out << value;
	}
	constexpr std::ostream& operator<<(std::ostream& out) const volatile noexcept {	// << ostream operator
		return out << value;
	}
	constexpr std::istream& operator>>(std::istream& is) noexcept {					// << istream operator
		return is >> value;
	}
	constexpr std::istream& operator>>(std::istream& is) volatile noexcept {		// << istream operator
		return is >> value;
	}

	// Overload of arithmetic operators
	////////////////////////////////////

	constexpr SpecReal operator+() const noexcept {									// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecReal operator+() const volatile noexcept {						// (+) Unary plus (integer promotion) arithmetic operator
		return *this;
	}
	constexpr SpecReal operator-() const noexcept {									// (-) Unary minus (additive inverse) arithmetic operator
		return SpecReal{-value};
	}
	constexpr SpecReal operator-() const volatile noexcept {						// (-) Unary minus (additive inverse) arithmetic operator
		return SpecReal{-value};
	}
	constexpr SpecReal& operator++() noexcept {										// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecReal& operator++() volatile noexcept {							// ++ (Prefix) Increment arithmetic operator
		value += static_cast<T>(1.0);
		return const_cast<SpecReal&>(*this);
	}
	constexpr SpecReal operator++(int) const noexcept {								// ++ (Postfix) Increment arithmetic operator
		return SpecReal{value+static_cast<T>(1.0)};
 	}
	constexpr SpecReal operator++(int) const volatile noexcept {					// ++ (Postfix) Increment arithmetic operator
		return SpecReal{value+static_cast<T>(1.0)};
	}
	constexpr SpecReal& operator--() noexcept {										// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return *this;
 	}
	constexpr SpecReal& operator--() volatile noexcept {							// -- (Prefix) Decrement arithmetic operator
		value -= static_cast<T>(1.0);
		return const_cast<SpecReal&>(*this);
	}
	constexpr SpecReal operator--(int) const noexcept {								// -- (Postfix) Decrement arithmetic operator
		return SpecReal{value-static_cast<T>(1.0)};
 	}
	constexpr SpecReal operator--(int) const volatile noexcept {					// -- (Postfix) Decrement arithmetic operator
		return SpecReal{value-static_cast<T>(1.0)};
	}

	constexpr SpecReal& operator+=(const SpecReal& obj) noexcept {					// += Addition assignment arithmetic operator
		value += obj.value;
		return *this;
	}
	constexpr SpecReal& operator+=(const SpecReal& obj) volatile noexcept {			// += Addition assignment arithmetic operator
		value += obj.value;
		return const_cast<SpecReal&>(*this);
	}
	template<typename P>
	constexpr SpecReal& operator+=(const P& oval) noexcept {						// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return *this;
	}
	template<typename P>
	constexpr SpecReal& operator+=(const P& oval) volatile noexcept {				// += Addition assignment arithmetic operator (other classes)
		value += oval;
		return const_cast<SpecReal&>(*this);
	}
	constexpr SpecReal operator+(const SpecReal& obj) const noexcept {				// + Addition arithmetic operator
		return SpecReal{value+obj.value};
	}
	constexpr SpecReal operator+(const SpecReal& obj) const volatile noexcept {		// + Addition arithmetic operator
		return SpecReal{value+obj.value};
	}
	template<typename P>
	constexpr SpecReal operator+(const P& oval) const noexcept {					// + Addition arithmetic operator (other classes)
		return SpecReal{value+(T)oval};
	}
	template<typename P>
	constexpr SpecReal operator+(const P& oval) const volatile noexcept {			// + Addition arithmetic operator (other classes)
		return SpecReal{value+(T)oval};
	}
	constexpr SpecReal& operator-=(const SpecReal& obj) noexcept {					// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return *this;
	}
	constexpr SpecReal& operator-=(const SpecReal& obj) volatile noexcept {			// -= Subtraction assignment arithmetic operator
		value -= obj.value;
		return const_cast<SpecReal&>(*this);
	}
	template<typename P>
	constexpr SpecReal& operator-=(const P& oval) noexcept {						// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecReal& operator-=(const P& oval) volatile noexcept {				// -= Subtraction assignment arithmetic operator (other classes)
		value -= oval;
		return const_cast<SpecReal&>(*this);
	}
	constexpr SpecReal operator-(const SpecReal& obj) const noexcept {				// - Subtraction arithmetic operator
		return SpecReal{value-obj.value};
	}
	constexpr SpecReal operator-(const SpecReal& obj) const volatile noexcept {		// - Subtraction arithmetic operator
		return SpecReal{value-obj.value};
	}
	template<typename P>
	constexpr SpecReal operator-(const P& oval) const noexcept {					// - Subtraction arithmetic operator (other classes)
		return SpecReal{value-(T)oval};
	}
	template<typename P>
	constexpr SpecReal operator-(const P& oval) const volatile noexcept {			// - Subtraction arithmetic operator (other classes)
		return SpecReal{value-(T)oval};
	}
	constexpr SpecReal& operator*=(const SpecReal& obj) noexcept {					// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return *this;
	}
	constexpr SpecReal& operator*=(const SpecReal& obj) volatile noexcept {			// *= Multiplication assignment arithmetic operator
		value *= obj.value;
		return const_cast<SpecReal&>(*this);
	}
	template<typename P>
	constexpr SpecReal& operator*=(const P& oval) noexcept {						// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecReal& operator*=(const P& oval) volatile noexcept {				// *= Multiplication assignment arithmetic operator (other classes)
		value *= oval;
		return const_cast<SpecReal&>(*this);
	}
	constexpr SpecReal operator*(const SpecReal& obj) const noexcept {				// * Multiplication arithmetic operator
		return SpecReal{value*obj.value};
	}
	constexpr SpecReal operator*(const SpecReal& obj) const volatile noexcept {		// * Multiplication arithmetic operator
		return SpecReal{value*obj.value};
	}
	template<typename P>
	constexpr SpecReal operator*(const P& oval) const noexcept {					// * Multiplication arithmetic operator (other classes)
		return SpecReal{value*(T)oval};
	}
	template<typename P>
	constexpr SpecReal operator*(const P& oval) const volatile noexcept {			// * Multiplication arithmetic operator (other classes)
		return SpecReal{value*(T)oval};
	}
	constexpr SpecReal& operator/=(const SpecReal& obj) noexcept {					// /= Division assignment arithmetic operator
		value /= obj.value;
		return *this;
	}
	constexpr SpecReal& operator/=(const SpecReal& obj) volatile noexcept {			// /= Division assignment arithmetic operator
		value /= obj.value;
		return const_cast<SpecReal&>(*this);
	}
	template<typename P>
	constexpr SpecReal& operator/=(const P& oval) noexcept {						// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return *this;
	}
	template<typename P>
	constexpr SpecReal& operator/=(const P& oval) volatile noexcept {				// /= Division assignment arithmetic operator (other classes)
		value /= oval;
		return const_cast<SpecReal&>(*this);
	}
	constexpr SpecReal operator/(const SpecReal& obj) const noexcept {				// / Division arithmetic operator
		return SpecReal{value/obj.value};
 	}
	constexpr SpecReal operator/(const SpecReal& obj) const volatile noexcept {		// / Division arithmetic operator
		return SpecReal{value/obj.value};
	}
	template<typename P>
	constexpr SpecReal operator/(const P& oval) const noexcept {					// / Division arithmetic operator (other classes)
		return SpecReal{value/(T)oval};
 	}
	template<typename P>
	constexpr SpecReal operator/(const P& oval) const volatile noexcept {			// / Division arithmetic operator (other classes)
		return SpecReal{value/(T)oval};
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
	constexpr bool operator&&(const SpecReal& obj) const noexcept {				// && AND logical operator
		return (value && obj.value);
	}
	constexpr bool operator&&(const SpecReal& obj) const volatile noexcept {	// && AND logical operator
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
	constexpr bool operator||(const SpecReal& obj) const noexcept {				// || OR logical operator
		return (value || obj.value);
	}
	constexpr bool operator||(const SpecReal& obj) const volatile noexcept {	// || OR logical operator
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

	constexpr SpecReal operator,(SpecReal& obj) noexcept {						// , Comma operator
		return SpecReal{(value, (obj.value))};
	}
	constexpr SpecReal operator,(SpecReal& obj) volatile noexcept {				// , Comma operator
		return SpecReal{(value, (obj.value))};
	}
	template<typename P>
	constexpr SpecReal operator,(P& oval) noexcept {							// , Comma operator
		return SpecReal{(value, (T)(oval))};
	}
	template<typename P>
	constexpr SpecReal operator,(P& oval) volatile noexcept {					// , Comma operator
		return SpecReal{(value, (T)(oval))};
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

	constexpr SpecReal* getObjPtr() noexcept {				// Custom function to get a pointer to the object
		return this;
	}

	constexpr volatile SpecReal* getObjPtr() volatile noexcept {		// Custom function to get a pointer to the object
		return this;
	}

	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_ABSOLUTE || EPST == EPS_ABSOLUTE_AND_RELATIVE || EPST == EPS_ABSOLUTE_AND_ULP), TNV>
	getAbsPrecisionThreshold() noexcept {					// Custom function to get the absolute precision threshold used in comparisons
		return epsilonAbs;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_RELATIVE || EPST == EPS_ABSOLUTE_AND_RELATIVE), TNV>
	getRelPrecisionThreshold() noexcept {					// Custom function to get the relative precision threshold used in comparisons
		return epsilonRel;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_ULP || EPST == EPS_ABSOLUTE_AND_ULP), ulp_type>
	getUlpPrecisionThreshold() noexcept {					// Custom function to get the ULP precision threshold used in comparisons
		return epsilonUlp;
	}

	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_ABSOLUTE), TNV>
	getPrecisionThreshold() noexcept {						// Custom function to get the precision threshold used in comparisons
		return getAbsPrecisionThreshold();
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_RELATIVE), TNV>
	getPrecisionThreshold() noexcept {						// Custom function to get the precision threshold used in comparisons
		return getRelPrecisionThreshold();
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_ULP), ulp_type>
	getPrecisionThreshold() noexcept {						// Custom function to get the precision threshold used in comparisons
		return getUlpPrecisionThreshold();
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_ABSOLUTE_AND_RELATIVE), std::pair<TNV, TNV>>
	getPrecisionThreshold() noexcept {						// Custom function to get the precision threshold used in comparisons
		return std::pair<TNV, TNV>(getAbsPrecisionThreshold(), getRelPrecisionThreshold());
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	static constexpr std::enable_if_t<(EPST == EPS_ABSOLUTE_AND_ULP), std::pair<TNV, ulp_type>>
	getPrecisionThreshold() noexcept {						// Custom function to get the precision threshold used in comparisons
		return std::pair<TNV, TNV>(getAbsPrecisionThreshold(), getUlpPrecisionThreshold());
	}

	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the absolute precision threshold used in comparisons
	static constexpr void setAbsPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ABSOLUTE || EPST == EPS_ABSOLUTE_AND_RELATIVE || EPS_TYPE == EPS_ABSOLUTE_AND_ULP), TNV> pr) noexcept {
		assert(pr >= static_cast<TNV>(0.0));
		epsilonAbs = pr;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the relative precision threshold used in comparisons
	static constexpr void setRelPrecisionThreshold(const std::enable_if_t<(EPST == EPS_RELATIVE || EPST == EPS_ABSOLUTE_AND_RELATIVE), TNV> pr) noexcept {
		assert(pr >= static_cast<TNV>(0.0));
		epsilonRel = pr;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the ULP precision threshold used in comparisons
	static constexpr void setUlpPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ULP || EPST == EPS_ABSOLUTE_AND_ULP), ulp_type> pr) noexcept {
		assert(pr >= 0);
		epsilonUlp = pr;
	}

	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the precision threshold used in comparisons (EPS_ABSOLUTE)
	static constexpr void setPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ABSOLUTE), TNV> pr) noexcept {
		setAbsPrecisionThreshold(pr);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the precision threshold used in comparisons (EPS_RELATIVE)
	static constexpr void setPrecisionThreshold(const std::enable_if_t<(EPST == EPS_RELATIVE), TNV> pr) noexcept {
		setRelPrecisionThreshold(pr);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the precision threshold used in comparisons (EPS_ULP)
	static constexpr void setPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ULP), ulp_type> pr) noexcept {
		setUlpPrecisionThreshold(pr);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the precision threshold used in comparisons
	static constexpr void setPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ABSOLUTE_AND_RELATIVE), TNV> prAbs, const typename std::remove_reference<TNV>::type prRel) noexcept {
		setAbsPrecisionThreshold(prAbs);
		setRelPrecisionThreshold(prRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the precision threshold used in comparisons
	static constexpr void setPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ABSOLUTE_AND_RELATIVE), std::pair<TNV, TNV>> pr) noexcept {
		setAbsPrecisionThreshold(pr.first);
		setRelPrecisionThreshold(pr.second);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the precision threshold used in comparisons
	static constexpr void setPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ABSOLUTE_AND_ULP), TNV> prAbs, const ulp_type prUlp) noexcept {
		setAbsPrecisionThreshold(prAbs);
		setUlpPrecisionThreshold(prUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>		// Custom function to set the precision threshold used in comparisons
	static constexpr void setPrecisionThreshold(const std::enable_if_t<(EPST == EPS_ABSOLUTE_AND_ULP), std::pair<TNV, ulp_type>> pr) noexcept {
		setAbsPrecisionThreshold(pr.first);
		setUlpPrecisionThreshold(pr.second);
	}

private:
	// Function to get the absolute value of a number
	static constexpr T abs(const T n) noexcept {
		return (n >= static_cast<T>(0) ? n : -n);
	}

	// Comparison functions (EPS_ABSOLUTE)
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	EqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const noexcept {
		return abs(value - oval) <= epsAbs;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	EqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const volatile noexcept {
		return abs(value - oval) <= epsAbs;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	NotEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const noexcept {
		return abs(value - oval) > epsAbs;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	NotEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const volatile noexcept {
		return abs(value - oval) > epsAbs;
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	LessThanComp(const T oval, const TNV epsAbs, const T, const ulp_type) const noexcept {
		return value < (oval - epsAbs);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	LessThanComp(const T oval, const TNV epsAbs, const T, const ulp_type) const volatile noexcept {
		return value < (oval - epsAbs);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	GreaterThanComp(const T oval, const TNV epsAbs, const T, const ulp_type) const noexcept {
		return value > (oval + epsAbs);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	GreaterThanComp(const T oval, const TNV epsAbs, const T, const ulp_type) const volatile noexcept {
		return value > (oval + epsAbs);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	LessThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const noexcept {
		return value <= (oval + epsAbs);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	LessThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const volatile noexcept {
		return value <= (oval + epsAbs);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const noexcept {
		return value >= (oval - epsAbs);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE, bool>
	GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type) const volatile noexcept {
		return value >= (oval - epsAbs);
	}

	// Comparison functions (EPS_RELATIVE)
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	EqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const noexcept {
		return abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	EqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const volatile noexcept {
		return abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	NotEqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const noexcept {
		return abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	NotEqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const volatile noexcept {
		return abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	LessThanComp(const T oval, const T, const TNV epsRel, const ulp_type) const noexcept {
		return value < (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	LessThanComp(const T oval, const T, const TNV epsRel, const ulp_type) const volatile noexcept {
		return value < (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	GreaterThanComp(const T oval, const T, const TNV epsRel, const ulp_type) const noexcept {
		return value > (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	GreaterThanComp(const T oval, const T, const TNV epsRel, const ulp_type) const volatile noexcept {
		return value > (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	LessThanOrEqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const noexcept {
		return value <= (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	LessThanOrEqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const volatile noexcept {
		return value <= (oval + (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	GreaterThanOrEqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const noexcept {
		return value >= (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_RELATIVE, bool>
	GreaterThanOrEqualComp(const T oval, const T, const TNV epsRel, const ulp_type) const volatile noexcept {
		return value >= (oval - (std::max(abs(value), abs(oval)) * epsRel));
	}

	// Comparison functions (EPS_ULP)
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	EqualComp(const T oval, const T, const T, const ulp_type epsUlp) const noexcept {
		return RFP(value).EqualComp(RFP(oval), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	EqualComp(const T oval, const T, const T, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).EqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	NotEqualComp(const T oval, const T, const T, const ulp_type epsUlp) const noexcept {
		return RFP(value).NotEqualComp(RFP(oval), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	NotEqualComp(const T oval, const T, const T, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).NotEqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	LessThanComp(const T oval, const T, const T, const ulp_type epsUlp) const noexcept {
		return RFP(value).LessThanComp(RFP(oval), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	LessThanComp(const T oval, const T, const T, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).LessThanComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	GreaterThanComp(const T oval, const T, const T, const ulp_type epsUlp) const noexcept {
		return RFP(value).GreaterThanComp(RFP(oval), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	GreaterThanComp(const T oval, const T, const T, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).GreaterThanComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	LessThanOrEqualComp(const T oval, const T, const T, const ulp_type epsUlp) const noexcept {
		return RFP(value).LessThanOrEqualComp(RFP(oval), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	LessThanOrEqualComp(const T oval, const T, const T, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).LessThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	GreaterThanOrEqualComp(const T oval, const T, const T, const ulp_type epsUlp) const noexcept {
		return RFP(value).GreaterThanOrEqualComp(RFP(oval), epsUlp);
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ULP, bool>
	GreaterThanOrEqualComp(const T oval, const T, const T, const ulp_type epsUlp) const volatile noexcept {
		return RFP(const_cast<const T&>(value)).GreaterThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp);
	}

	// Comparison functions (EPS_ABSOLUTE_AND_RELATIVE)
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	EqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const noexcept {
		return (abs(value - oval) <= epsAbs) || (abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	EqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const volatile noexcept {
		return (abs(value - oval) <= epsAbs) || (abs(value - oval) <= (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	NotEqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const noexcept {
		return (abs(value - oval) > epsAbs) && (abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	NotEqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const volatile noexcept {
		return (abs(value - oval) > epsAbs) && (abs(value - oval) > (std::max(abs(value), abs(oval)) * epsRel));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	LessThanComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const noexcept {
		return (value < (oval - epsAbs)) && (value < (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	LessThanComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const volatile noexcept {
		return (value < (oval - epsAbs)) && (value < (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	GreaterThanComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const noexcept {
		return (value > (oval + epsAbs)) && (value > (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	GreaterThanComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const volatile noexcept {
		return (value > (oval + epsAbs)) && (value > (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	LessThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const noexcept {
		return (value <= (oval + epsAbs)) || (value <= (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	LessThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const volatile noexcept {
		return (value <= (oval + epsAbs)) || (value <= (oval + (std::max(abs(value), abs(oval)) * epsRel)));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const noexcept {
		return (value >= (oval - epsAbs)) || (value >= (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_RELATIVE, bool>
	GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const TNV epsRel, const ulp_type) const volatile noexcept {
		return (value >= (oval - epsAbs)) || (value >= (oval - (std::max(abs(value), abs(oval)) * epsRel)));
	}

	// Comparison functions (EPS_ABSOLUTE_AND_ULP)
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	EqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const noexcept {
		return (abs(value - oval) <= epsAbs) || (RFP(value).EqualComp(RFP(oval), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	EqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const volatile noexcept {
		return (abs(value - oval) <= epsAbs) || (RFP(const_cast<const T&>(value)).EqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	NotEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const noexcept {
		return (abs(value - oval) > epsAbs) && (RFP(value).NotEqualComp(RFP(oval), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	NotEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const volatile noexcept {
		return (abs(value - oval) > epsAbs) && (RFP(const_cast<const T&>(value)).NotEqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	LessThanComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const noexcept {
		return (value < (oval - epsAbs)) && (RFP(value).LessThanComp(RFP(oval), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	LessThanComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const volatile noexcept {
		return (value < (oval - epsAbs)) && (RFP(const_cast<const T&>(value)).LessThanComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	GreaterThanComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const noexcept {
		return (value > (oval + epsAbs)) && (RFP(value).GreaterThanComp(RFP(oval), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	GreaterThanComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const volatile noexcept {
		return (value > (oval + epsAbs)) && (RFP(const_cast<const T&>(value)).GreaterThanComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	LessThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const noexcept {
		return (value <= (oval + epsAbs)) || (RFP(value).LessThanOrEqualComp(RFP(oval), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	LessThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const volatile noexcept {
		return (value <= (oval + epsAbs)) || (RFP(const_cast<const T&>(value)).LessThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const noexcept {
		return (value >= (oval - epsAbs)) || (RFP(value).GreaterThanOrEqualComp(RFP(oval), epsUlp));
	}
	template<const specreal_eps_type_t EPST = EPS_TYPE>
	constexpr std::enable_if_t<EPST == EPS_ABSOLUTE_AND_ULP, bool>
	GreaterThanOrEqualComp(const T oval, const TNV epsAbs, const T, const ulp_type epsUlp) const volatile noexcept {
		return (value >= (oval - epsAbs)) || (RFP(const_cast<const T&>(value)).GreaterThanOrEqualComp(RFP(const_cast<const T&>(oval)), epsUlp));
	}

};

// External overload of operators seems not necesary and can creaty ambiguity
////////////////////////////////////

template<typename T, const specreal_eps_type_t EPS_TYPE, const int EPS_ID>
typename SpecReal<T, EPS_TYPE, EPS_ID>::TNV SpecReal<T, EPS_TYPE, EPS_ID>::epsilonAbs = SpecReal<T, EPS_TYPE, EPS_ID>::defaultEpsilonAbs;		// Definition and initialization to its default value of the absolute epsilon variable
template<typename T, const specreal_eps_type_t EPS_TYPE, const int EPS_ID>
typename SpecReal<T, EPS_TYPE, EPS_ID>::TNV SpecReal<T, EPS_TYPE, EPS_ID>::epsilonRel = SpecReal<T, EPS_TYPE, EPS_ID>::defaultEpsilonRel;		// Definition and initialization to its default value of the relative epsilon variable
template<typename T, const specreal_eps_type_t EPS_TYPE, const int EPS_ID>
typename SpecReal<T, EPS_TYPE, EPS_ID>::ulp_type SpecReal<T, EPS_TYPE, EPS_ID>::epsilonUlp = SpecReal<T, EPS_TYPE, EPS_ID>::defaultEpsilonUlp;	// Definition and initialization to its default value of the ULP epsilon variable

} //namespace SpecLib

#endif // __SPECREAL_H
