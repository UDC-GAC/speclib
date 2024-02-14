/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     SpecRealComm.h
/// \brief    Includes the types, classes, constants and definitions used in common by the SpecReal and SpecRealInd classes
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef __SPECREALCOMM_H
#define __SPECREALCOMM_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace SpecLib {

typedef enum {
	EPS_ABSOLUTE,
	EPS_RELATIVE,
	EPS_ULP,
	EPS_ABSOLUTE_AND_RELATIVE,
	EPS_ABSOLUTE_AND_ULP
} specreal_eps_type_t;

namespace internal {

#ifdef __SIZEOF_INT128__
__extension__ using __spec_uint128 = unsigned __int128;
#endif

// 'UIntOfSize' class returns an unsigned integer type of the specified size
template<const size_t BSIZE, const bool ISVOL = false>
struct UIntOfSize {
};
// 'int' and 'volatile int' are not valid types for this purpose but allow a valid type to be returned, allowing instantiation of static variables
// If you intend to use this type, you should check with a static_assertion that the returned type is a unsigned type (as 'RawFloatingPoint' class does)
template<const size_t BSIZE>
struct UIntOfSize<BSIZE, false> {
	using value_type = int;
};
template<const size_t BSIZE>
struct UIntOfSize<BSIZE, true> {
	using value_type = volatile int;
};
template<>
struct UIntOfSize<1, false> {
	using value_type = std::uint8_t;
};
template<>
struct UIntOfSize<1, true> {
	using value_type = volatile std::uint8_t;
};
template<>
struct UIntOfSize<2, false> {
	using value_type = std::uint16_t;
};
template<>
struct UIntOfSize<2, true> {
	using value_type = volatile std::uint16_t;
};
template<>
struct UIntOfSize<4, false> {
	using value_type = std::uint32_t;
};
template<>
struct UIntOfSize<4, true> {
	using value_type = volatile std::uint32_t;
};
template<>
struct UIntOfSize<8, false> {
	using value_type = std::uint64_t;
};
template<>
struct UIntOfSize<8, true> {
	using value_type = volatile std::uint64_t;
};
#ifdef __SIZEOF_INT128__
template<>
struct UIntOfSize<16, false> {
	using value_type = __spec_uint128;
};
template<>
struct UIntOfSize<16, true> {
	using value_type = volatile __spec_uint128;
};
#endif

// 'RawFloatPoint' class allows converting a floating point number (IEEE 754) to a raw format and performing comparisons (with ULP tolerance) and other operations
template<typename T>
class RawFloatPoint {
private:
	template<typename Z>
	using expr_type = std::remove_cv_t<std::remove_reference_t<Z>>;

public:
	static_assert(std::numeric_limits<T>::is_iec559, "EPS_ULP and EPS_ABSOLUTE_AND_ULP comparison types can only be used with IEC 559 (a.k.a. IEEE 754) standard floating point types");
#ifdef __SIZEOF_INT128__
	static_assert((sizeof(T) != 16) || (std::numeric_limits<T>::digits10 == 18 || std::numeric_limits<T>::digits10 == 33), "EPS_ULP and EPS_ABSOLUTE_AND_ULP comparison types cannot be used with this floating point type");
#endif

	using rawb_type = typename UIntOfSize<sizeof(T), std::is_volatile<T>::value>::value_type;
	using rawb_type_nv = typename std::remove_volatile<rawb_type>::type;

#ifdef __SIZEOF_INT128__
	static_assert(std::is_unsigned<rawb_type>::value || std::is_same<expr_type<rawb_type>, __spec_uint128>::value, "EPS_ULP and EPS_ABSOLUTE_AND_ULP comparison types cannot be used with this floating point type");
#else
	static_assert(std::is_unsigned<rawb_type>::value, "EPS_ULP and EPS_ABSOLUTE_AND_ULP comparison types cannot be used with this floating point type");
#endif
	static_assert(sizeof(rawb_type) >= sizeof(T), "EPS_ULP and EPS_ABSOLUTE_AND_ULP comparison types cannot be used because the floating point type used does not have an integer of sufficient size to represent it");

	const rawb_type rfp_value;		// The raw representation of the floating point value is stored here as unsigned integer

	// Total number of bits of the floating point number (note the exception for 'long double' which can be 80bits)
	static constexpr size_t totalNumBits = 8 * (((sizeof(T) == 16) && (std::numeric_limits<T>::digits10 == 18)) ? 10u : sizeof(T));

	// Total number of bits of the mantissa part of the floating point number
	static constexpr size_t mantissaNumBits = static_cast<size_t>(std::numeric_limits<T>::digits - 1);

	// Total number of bits of the exponent part of the floating point number
	static constexpr size_t exponentNumBits = totalNumBits - 1u - mantissaNumBits;

	// Mask of bits for the entire floating point number
	static constexpr rawb_type_nv totalMaskBits = ~static_cast<rawb_type_nv>(0) >> ((sizeof(rawb_type_nv)*8-totalNumBits));

	// Mask of bits for the sign part of the floating point number
	static constexpr rawb_type_nv signBitsMask = static_cast<rawb_type_nv>(1) << (totalNumBits - 1);

	// Mask of bits for the mansissa part of the floating point number
	static constexpr rawb_type_nv mantissaBitsMask = ~static_cast<rawb_type_nv>(0) >> (exponentNumBits + 1 + (sizeof(rawb_type_nv)*8-totalNumBits));

	// Mask of bits for the exponent part of the floating point number
	static constexpr rawb_type_nv exponentBitsMask = totalMaskBits & ~(signBitsMask | mantissaBitsMask);

	// Constructor
	RawFloatPoint(const T& x) noexcept :
	rfp_value(*reinterpret_cast<const rawb_type*>(&x) & totalMaskBits)
	{ }

	// Check if the floating point number is a NaN number
	constexpr bool is_NaN() const noexcept {
		return ((mantissaBitsMask & rfp_value) != static_cast<rawb_type>(0)) && ((exponentBitsMask & rfp_value) == exponentBitsMask);
	}

	// Convert the raw representation of the floating point number to a offset binary format
	constexpr rawb_type convertToOffsetBinary() const noexcept {
		return (signBitsMask & rfp_value) ? ((~rfp_value + static_cast<rawb_type>(1)) & totalMaskBits) : (signBitsMask | rfp_value);
	}

	// Check equality between two floating point numbers with a tolerance of 'epsUlp' ULPs
	constexpr bool EqualComp(const RawFloatPoint& other, const rawb_type epsUlp) const noexcept {
		const rawb_type x = convertToOffsetBinary();
		const rawb_type y = other.convertToOffsetBinary();
		const rawb_type diff = (x >= y) ? (x - y) : (y - x);
		return !is_NaN() && !other.is_NaN() && (diff <= epsUlp);
	}

	// Check inequality between two floating point numbers with a tolerance of 'epsUlp' ULPs
	constexpr bool NotEqualComp(const RawFloatPoint& other, const rawb_type epsUlp) const noexcept {
		const rawb_type x = convertToOffsetBinary();
		const rawb_type y = other.convertToOffsetBinary();
		const rawb_type diff = (x >= y) ? (x - y) : (y - x);
		return is_NaN() || other.is_NaN() || (diff > epsUlp);
	}

	// Check if floating point number is less than another with a tolerance of 'epsUlp' ULPs
	constexpr bool LessThanComp(const RawFloatPoint& other, const rawb_type epsUlp) const noexcept {
		const rawb_type x = convertToOffsetBinary();
		const rawb_type y = other.convertToOffsetBinary();
		return !is_NaN() && !other.is_NaN() && (x < (y - epsUlp));
	}

	// Check if floating point number is greater than another with a tolerance of 'epsUlp' ULPs
	constexpr bool GreaterThanComp(const RawFloatPoint& other, const rawb_type epsUlp) const noexcept {
		const rawb_type x = convertToOffsetBinary();
		const rawb_type y = other.convertToOffsetBinary();
		return !is_NaN() && !other.is_NaN() && (x > (y + epsUlp));
	}

	// Check if floating point number is less than or equal to another with a tolerance of 'epsUlp' ULPs
	constexpr bool LessThanOrEqualComp(const RawFloatPoint& other, const rawb_type epsUlp) const noexcept {
		const rawb_type x = convertToOffsetBinary();
		const rawb_type y = other.convertToOffsetBinary();
		return !is_NaN() && !other.is_NaN() && (x <= (y + epsUlp));
	}

	// Check if floating point number is greater than or equal to another with a tolerance of 'epsUlp' ULPs
	constexpr bool GreaterThanOrEqualComp(const RawFloatPoint& other, const rawb_type epsUlp) const noexcept {
		const rawb_type x = convertToOffsetBinary();
		const rawb_type y = other.convertToOffsetBinary();
		return !is_NaN() && !other.is_NaN() && (x >= (y - epsUlp));
	}
};

} //namespace internal

} //namespace SpecLib

#endif // __SPECREALCOMM_H
