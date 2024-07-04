#pragma once
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <optional>
#include <cassert>
#include <array>

// namespace crabbyfeet
namespace cf
{
	/*
		A complete 2D geometric vector structure, with a variety
		of useful utility functions and operator overloads
	*/
	template<class T = int>
	struct vec_2d
	{
		static_assert(std::is_arithmetic<T>::value, "cf::v_2d<type> must be numeric");

		// x-axis component
		T x = 0;
		// y-axis component
		T y = 0;

		// Default constructor
		inline constexpr vec_2d() = default;

		// Specific constructor
		inline constexpr vec_2d(T _x, T _y) : x(_x), y(_y)
		{}

		// Copy constructor
		inline constexpr vec_2d(const vec_2d& v) = default;

		// Assignment operator
		inline constexpr vec_2d& operator=(const vec_2d& v) = default;


		// Returns rectangular area of vector
		inline constexpr auto area() const
		{
			return x * y;
		}

		// Returns magnitude of vector
		inline auto mag() const
		{
			return std::sqrt(x * x + y * y);
		}

		// Returns magnitude squared of vector (useful for fast comparisons)
		inline constexpr T mag2() const
		{
			return x * x + y * y;
		}

		// Returns normalised version of vector
		inline vec_2d norm() const
		{
			auto r = 1 / mag();
			return vec_2d(x * r, y * r);
		}

		// Returns vector at 90 degrees to this one
		inline constexpr vec_2d perp() const
		{
			return vec_2d(-y, x);
		}

		// Rounds both components down
		inline constexpr vec_2d floor() const
		{
			return vec_2d(std::floor(x), std::floor(y));
		}

		// Rounds both components up
		inline constexpr vec_2d ceil() const
		{
			return vec_2d(std::ceil(x), std::ceil(y));
		}

		// Returns 'element-wise' max of this and another vector
		inline constexpr vec_2d max(const vec_2d& v) const
		{
			return vec_2d(std::max(x, v.x), std::max(y, v.y));
		}

		// Returns 'element-wise' min of this and another vector
		inline constexpr vec_2d min(const vec_2d& v) const
		{
			return vec_2d(std::min(x, v.x), std::min(y, v.y));
		}

		// Calculates scalar dot product between this and another vector
		inline constexpr auto dot(const vec_2d& rhs) const
		{
			return this->x * rhs.x + this->y * rhs.y;
		}

		// Calculates 'scalar' cross product between this and another vector (useful for winding orders)
		inline constexpr auto cross(const vec_2d& rhs) const
		{
			return this->x * rhs.y - this->y * rhs.x;
		}

		// Treat this as polar coordinate (R, Theta), return cartesian equivalent (X, Y)
		inline constexpr vec_2d cart() const
		{
			return vec_2d(std::cos(y) * x, std::sin(y) * x);
		}

		// Treat this as cartesian coordinate (X, Y), return polar equivalent (R, Theta)
		inline constexpr vec_2d polar() const
		{
			return vec_2d(mag(), std::atan2(y, x));
		}

		// Clamp the components of this vector in between the 'element-wise' minimum and maximum of 2 other vectors
		inline constexpr vec_2d clamp(const vec_2d& v1, const vec_2d& v2) const
		{
			return this->max(v1).min(v2);
		}

		// Linearly interpolate between this vector, and another vector, given normalised parameter 't'
		inline constexpr vec_2d lerp(const vec_2d& v1, const double t) const
		{
			return (*this) * (T(1.0 - t)) + (v1 * T(t));
		}

		// Compare if this vector is numerically equal to another
		inline constexpr bool operator == (const vec_2d& rhs) const
		{
			return (this->x == rhs.x && this->y == rhs.y);
		}

		// Compare if this vector is not numerically equal to another
		inline constexpr bool operator != (const vec_2d& rhs) const
		{
			return (this->x != rhs.x || this->y != rhs.y);
		}

		// Return this vector as a std::string, of the form "(x,y)"
		inline std::string str() const
		{
			return std::string("(") + std::to_string(this->x) + "," + std::to_string(this->y) + ")";
		}

		// Assuming this vector is incident, given a normal, return the reflection
		inline constexpr vec_2d reflect(const vec_2d& n) const
		{
			return (*this) - 2.0 * (this->dot(n) * n);
		}

		// Allow 'casting' from other v_2d types
		template<class F>
		inline constexpr operator vec_2d<F>() const
		{
			return { static_cast<F>(this->x), static_cast<F>(this->y) };
		}
	};

	// Multiplication operator overloads between vectors and scalars, and vectors and vectors
	template<class TL, class TR>
	inline constexpr auto operator * (const TL& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs * rhs.x, lhs * rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator * (const vec_2d<TL>& lhs, const TR& rhs)
	{
		return vec_2d(lhs.x * rhs, lhs.y * rhs);
	}

	template<class TL, class TR>
	inline constexpr auto operator * (const vec_2d<TL>& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs.x * rhs.x, lhs.y * rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator *= (vec_2d<TL>& lhs, const TR& rhs)
	{
		lhs = lhs * rhs;
		return lhs;
	}

	// Division operator overloads between vectors and scalars, and vectors and vectors
	template<class TL, class TR>
	inline constexpr auto operator / (const TL& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs / rhs.x, lhs / rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator / (const vec_2d<TL>& lhs, const TR& rhs)
	{
		return vec_2d(lhs.x / rhs, lhs.y / rhs);
	}

	template<class TL, class TR>
	inline constexpr auto operator / (const vec_2d<TL>& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs.x / rhs.x, lhs.y / rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator /= (vec_2d<TL>& lhs, const TR& rhs)
	{
		lhs = lhs / rhs;
		return lhs;
	}

	// Unary Addition operator (pointless but i like the platinum trophies)
	template<class T>
	inline constexpr auto operator + (const vec_2d<T>& lhs)
	{
		return vec_2d(+lhs.x, +lhs.y);
	}

	// Addition operator overloads between vectors and scalars, and vectors and vectors
	template<class TL, class TR>
	inline constexpr auto operator + (const TL& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs + rhs.x, lhs + rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator + (const vec_2d<TL>& lhs, const TR& rhs)
	{
		return vec_2d(lhs.x + rhs, lhs.y + rhs);
	}

	template<class TL, class TR>
	inline constexpr auto operator + (const vec_2d<TL>& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator += (vec_2d<TL>& lhs, const TR& rhs)
	{
		lhs = lhs + rhs;
		return lhs;
	}

	template<class TL, class TR>
	inline constexpr auto operator += (vec_2d<TL>& lhs, const vec_2d<TR>& rhs)
	{
		lhs = lhs + rhs;
		return lhs;
	}

	// Unary negation operator overoad for inverting a vector
	template<class T>
	inline constexpr auto operator - (const vec_2d<T>& lhs)
	{
		return vec_2d(-lhs.x, -lhs.y);
	}

	// Subtraction operator overloads between vectors and scalars, and vectors and vectors
	template<class TL, class TR>
	inline constexpr auto operator - (const TL& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs - rhs.x, lhs - rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator - (const vec_2d<TL>& lhs, const TR& rhs)
	{
		return vec_2d(lhs.x - rhs, lhs.y - rhs);
	}

	template<class TL, class TR>
	inline constexpr auto operator - (const vec_2d<TL>& lhs, const vec_2d<TR>& rhs)
	{
		return vec_2d(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	template<class TL, class TR>
	inline constexpr auto operator -= (vec_2d<TL>& lhs, const TR& rhs)
	{
		lhs = lhs - rhs;
		return lhs;
	}

	// Greater/Less-Than Operator overloads - mathematically useless, but handy for "sorted" container storage
	template<class TL, class TR>
	inline constexpr bool operator < (const vec_2d<TL>& lhs, const vec_2d<TR>& rhs)
	{
		return (lhs.y < rhs.y) || (lhs.y == rhs.y && lhs.x < rhs.x);
	}

	template<class TL, class TR>
	inline constexpr bool operator > (const vec_2d<TL>& lhs, const vec_2d<TR>& rhs)
	{
		return (lhs.y > rhs.y) || (lhs.y == rhs.y && lhs.x > rhs.x);
	}

	// Allow olc::v_2d to play nicely with std::cout
	template<class T>
	inline constexpr std::ostream& operator << (std::ostream& os, const vec_2d<T>& rhs)
	{
		os << rhs.str();
		return os;
	}

	// Convenient types ready-to-go
	typedef vec_2d<int32_t> vi2d;
	typedef vec_2d<uint32_t> vu2d;
	typedef vec_2d<float> vf2d;
	typedef vec_2d<double> vd2d;
}
