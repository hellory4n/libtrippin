/*
 * libtrippin v2.2.0
 *
 * Most biggest most massive standard library thing of all time
 * https://github.com/hellory4n/libtrippin
 *
 * Copyright (C) 2025 by hellory4n <hellory4n@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby
 * granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef _TRIPPIN_H
#define _TRIPPIN_H
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
	// counting starts at 1 lmao
	#define TR_LOG_FUNC(FmtIdx, VarargsIdx) [[gnu::format(printf, FmtIdx, VarargsIdx)]]
#else
	#define TR_LOG_FUNC(FmtIdx, VarargsIdx)
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define TR_GCC_PRAGMA(X) _Pragma(#X)

	#define TR_GCC_IGNORE_WARNING(Warning) \
		TR_GCC_PRAGMA(GCC diagnostic push) \
		TR_GCC_PRAGMA(GCC diagnostic ignored #Warning)

	#define TR_GCC_RESTORE() TR_GCC_PRAGMA(GCC diagnostic pop)
#else
	#define TR_GCC_IGNORE_WARNING(Warning)
	#define TR_GCC_RESTORE()
#endif

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float float32;
typedef double float64;
typedef size_t usize;
typedef ptrdiff_t isize;

// it's not guaranteed lmao
static_assert(sizeof(usize) == sizeof(isize), "oh no usize and isize aren't the same size");

namespace tr {

// I sure love versions.
static constexpr const char* VERSION = "v2.1.2";

// Initializes the bloody library lmao.
void init();

// Deinitializes the bloody library lmao.
void free();

/*
* LOGGING
*/

namespace ConsoleColor {
	// TODO colored output doesn't work on windows and i can't be bothered to fix it
	#ifndef _WIN32
		constexpr const char* RESET = "\033[0m";
		constexpr const char* INFO  = "\033[0;90m";
		constexpr const char* WARN  = "\033[0;93m";
		constexpr const char* ERROR = "\033[0;91m";
	#else
		constexpr const char* RESET = "";
		constexpr const char* INFO  = "";
		constexpr const char* WARN  = "";
		constexpr const char* ERROR = "";
	#endif
}

// Sets the log file to somewhere.
void use_log_file(const char* path);

// Log.
TR_LOG_FUNC(1, 2) void log(const char* fmt, ...);

// Log. (gray edition) (this is for libraries that use libtrippin so you can filter out library logs)
TR_LOG_FUNC(1, 2) void info(const char* fmt, ...);

// Oh nose.
TR_LOG_FUNC(1, 2) void warn(const char* fmt, ...);

// Oh god oh fuck. Note this doesn't crash and die everything, `tr::panic` does.
TR_LOG_FUNC(1, 2) void error(const char* fmt, ...);

// Oh god oh fuck. Note this crashes and kills everything, `tr::error` doesn't.
TR_LOG_FUNC(1, 2) [[noreturn]] void panic(const char* fmt, ...);

// Formatted assert?????????
TR_LOG_FUNC(4, 5) void __impl_assert(const char* file, int line, bool x, const char* fmt, ...);

#define TR_ASSERT_MSG(X, ...) \
	tr::__impl_assert(__FILE__, __LINE__, X, "failed assert \"" #X "\": " __VA_ARGS__)

#define TR_ASSERT(X) \
	tr::__impl_assert(__FILE__, __LINE__, X, "failed assert: " #X)

/*
* UTILITIES
*/

// Like how the spicy modern languages handle null
template<typename T>
struct Maybe
{
private:
	union {
		uint8_t waste_of_space;
		T value;
	};
	bool has_value;

public:
	// Initializes a Maybe<T> as null
	Maybe() : waste_of_space(0), has_value(false) {};

	// Intializes a Maybe<T> with a value
	Maybe(const T& val) : value(val), has_value(true) {};

	Maybe(const Maybe& other) : has_value(other.has_value)
	{
		if (this->has_value) this->value = other.value;
		else this->has_value = 0;
	}

	Maybe& operator=(const Maybe& other)
	{
		if (this != &other) {
			if (this->has_value && other.has_value) {
				this->value = other.value;
			} else if (this->has_value && !other.has_value) {
				this->value.~T();
				this->has_value = false;
			} else if (!this->has_value && other.has_value) {
				this->value = other.value;
				this->has_value = true;
			}
		}
		return *this;
	}

	~Maybe()
	{
		if (this->has_value) {
			value.~T();
		}
	}

	// If true, the maybe is, in fact, a definitely.
	bool is_valid() const
	{
		return this->has_value;
	}

	// Gets the value or panics if it's null
	T& unwrap()
	{
		if (this->has_value) return this->value;
		else tr::panic("couldn't unwrap Maybe<T>");
	}

	// Gets the value or panics if it's null
	const T& unwrap() const
	{
		if (this->has_value) return this->value;
		else tr::panic("couldn't unwrap Maybe<T>");
	}
};

// Functional propaganda
template<typename L, typename R>
struct Either
{
private:
	union {
		L val_left;
		R val_right;
	};
	// False is left, true is right
	bool active;

public:
	Either(L left) : val_left(left), active(false) {};
	Either(R right) : val_right(right), active(true) {};

	~Either()
	{
		if (this->active) {
			this->val_left.~L();
		}
		else {
			this->val_right.~R();
		}
	}

	// If true, it's left. Else, it's right.
	bool is_left() { return this->active; }
	// If true, it's right. Else, it's left.
	bool is_right() { return !this->active; }

	// Returns the left value, or panics if it's not left
	L& left()
	{
		if (!this->active) tr::panic("Either<L, R> is right, not left");
		else return this->val_left;
	}

	// Returns the right value, or panics if it's not right
	R& right()
	{
		if (this->active) tr::panic("Either<L, R> is right, not left");
		else return this->val_right;
	}
};

// It's a pair lmao.
template<typename L, typename R>
struct Pair
{
	L left;
	R right;

	Pair(L left, R right) : left(left), right(right) {};

	~Pair()
	{
		left.~L();
		right.~R();
	}
};

/*
* MATH
*/

// Vec2 lmao
template<typename T>
struct Vec2
{
	T x;
	T y;

	constexpr Vec2() : x(0), y(0) {};
	constexpr Vec2(T x, T y) : x(x), y(y) {};

	Vec2 operator+(Vec2 r)  { return Vec2(this->x + r.x, this->y + r.y); }
	Vec2 operator-(Vec2 r)  { return Vec2(this->x - r.x, this->y - r.y); }
	Vec2 operator*(Vec2 r)  { return Vec2(this->x * r.x, this->y * r.y); }
	Vec2 operator*(T r)     { return Vec2(this->x * r,   this->y * r);   }
	Vec2 operator/(Vec2 r)  { return Vec2(this->x / r.x, this->y / r.y); }
	Vec2 operator/(T r)     { return Vec2(this->x / r,   this->y / r);   }
	Vec2 operator%(Vec2 r)  { return Vec2(this->x % r.x, this->y % r.y); }
	Vec2 operator%(T r)     { return Vec2(this->x % r,   this->y % r);   }

	bool operator==(Vec2 r) { return this->x == r.x && this->y == r.y;   }
	bool operator!=(Vec2 r) { return this->x != r.x && this->y != r.y;   }
	bool operator>(Vec2 r)  { return this->x >  r.x && this->y >  r.y;   }
	bool operator>=(Vec2 r) { return this->x >= r.x && this->y >= r.y;   }
	bool operator<(Vec2 r)  { return this->x <  r.x && this->y <  r.y;   }
	bool operator<=(Vec2 r) { return this->x <= r.x && this->y <= r.y;   }

	Vec2& operator+=(Vec2 other) { return *this = *this + other; }
	Vec2& operator-=(Vec2 other) { return *this = *this - other; }
	Vec2& operator*=(Vec2 other) { return *this = *this * other; }
	Vec2& operator/=(Vec2 other) { return *this = *this / other; }
	Vec2& operator%=(Vec2 other) { return *this = *this % other; }
	Vec2& operator*=(T other) { return *this = *this * other; }
	Vec2& operator/=(T other) { return *this = *this / other; }
	Vec2& operator%=(T other) { return *this = *this % other; }
	Vec2 operator-() { return {-this->x, -this->y}; }
	Vec2 operator+() { return {+this->x, +this->y}; }

	static constexpr usize ITEMS = 2;
	T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return this->x; break;
			case 1:  return this->y; break;
			default: tr::panic("sir this is a vec2<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	T mul_inner(Vec2<T> b)
	{
		T p = 0;
		for (usize i = 0; i < ITEMS; i++) {
			p += b[i] * (*this)[i];
		}
		return p;
	}

	float64 length() { return sqrt(this->mul_inner(*this)); }
	Vec2 normalize() { return *this * (1 / this->length()); }

	// swizzling operators
	// i know this is insane
	// i didnt write this myself im not insane
	constexpr Vec2<T> xx() const { return {this->x, this->x}; }
	constexpr Vec2<T> xy() const { return {this->x, this->y}; }
	constexpr Vec2<T> yx() const { return {this->y, this->x}; }
	constexpr Vec2<T> yy() const { return {this->y, this->y}; }
};

// c i hate you
template<> inline Vec2<float64> Vec2<float64>::operator%(Vec2<float64> r) {
	return Vec2<float64>(fmod(this->x, r.x), fmod(this->y, r.y));
}
template<> inline Vec2<float64> Vec2<float64>::operator%(float64 r) {
	return Vec2<float64>(fmod(this->x, r), fmod(this->y, r));
}
template<> inline Vec2<float32> Vec2<float32>::operator%(Vec2<float32> r) {
	return Vec2<float32>(fmod(this->x, r.x), fmod(this->y, r.y));
}
template<> inline Vec2<float32> Vec2<float32>::operator%(float32 r) {
	return Vec2<float32>(fmod(this->x, r), fmod(this->y, r));
}

// Vec3 lmao
template<typename T>
struct Vec3
{
	T x;
	T y;
	T z;

	constexpr Vec3() : x(0), y(0), z(0) {};
	constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {};

	Vec3 operator+(Vec3 r)  { return Vec3(this->x + r.x, this->y + r.y, this->z + r.z);  }
	Vec3 operator-(Vec3 r)  { return Vec3(this->x - r.x, this->y - r.y, this->z - r.z);  }
	Vec3 operator*(Vec3 r)  { return Vec3(this->x * r.x, this->y * r.y, this->z * r.z);  }
	Vec3 operator*(T r)     { return Vec3(this->x * r,   this->y * r,   this->z * r);    }
	Vec3 operator/(Vec3 r)  { return Vec3(this->x / r.x, this->y / r.y, this->z / r.z);  }
	Vec3 operator/(T r)     { return Vec3(this->x / r,   this->y / r,   this->z / r);    }
	Vec3 operator%(Vec3 r)  { return Vec3(this->x % r.x, this->y % r.y, this->z % r.z);  }
	Vec3 operator%(T r)     { return Vec3(this->x % r,   this->y % r,   this->z % r);    }

	bool operator==(Vec3 r) { return this->x == r.x && this->y == r.y && this->z == r.z; }
	bool operator!=(Vec3 r) { return this->x != r.x && this->y != r.y && this->z != r.z; }
	bool operator>(Vec3 r)  { return this->x >  r.x && this->y >  r.y && this->z >  r.z; }
	bool operator>=(Vec3 r) { return this->x >= r.x && this->y >= r.y && this->z >= r.z; }
	bool operator<(Vec3 r)  { return this->x <  r.x && this->y <  r.y && this->z <  r.z; }
	bool operator<=(Vec3 r) { return this->x <= r.x && this->y <= r.y && this->z <= r.z; }

	Vec3& operator+=(Vec3 other) { return *this = *this + other; }
	Vec3& operator-=(Vec3 other) { return *this = *this - other; }
	Vec3& operator*=(Vec3 other) { return *this = *this * other; }
	Vec3& operator/=(Vec3 other) { return *this = *this / other; }
	Vec3& operator%=(Vec3 other) { return *this = *this % other; }
	Vec3& operator*=(T other) { return *this = *this * other; }
	Vec3& operator/=(T other) { return *this = *this / other; }
	Vec3& operator%=(T other) { return *this = *this % other; }
	Vec3 operator-() { return {-this->x, -this->y}; }
	Vec3 operator+() { return {+this->x, +this->y}; }

	static constexpr usize ITEMS = 3;
	T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return this->x; break;
			case 1:  return this->y; break;
			case 2:  return this->z; break;
			default: tr::panic("sir this is a vec3<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	T mul_inner(Vec3<T> b)
	{
		T p = 0;
		for (usize i = 0; i < ITEMS; i++) {
			p += b[i] * (*this)[i];
		}
		return p;
	}

	float64 length() { return sqrt(this->mul_inner(*this)); }
	Vec3<T> normalize() { return *this * (1 / this->length()); }

	Vec3<T> reflect(Vec3<T> b)
	{
		Vec3<T> r;
		T p = 2 * this->mul_inner(b);
		for (usize i = 0; i < ITEMS; i++) {
			r[i] = (*this)[i] - p * b[i];
		}
		return r;
	}

	Vec3<T> mul_cross(Vec3<T> b)
	{
		Vec3<T> r;
		r[0] = (*this)[1] * b[2] - (*this)[2] * b[1];
		r[1] = (*this)[2] * b[0] - (*this)[0] * b[2];
		r[2] = (*this)[0] * b[1] - (*this)[1] * b[0];
		r[3] = 1.f;
		return r;
	}

	// swizzling operators
	// i know this is insane
	// i didnt write this myself im not insane
	Vec3(T x, Vec2<T> other) : Vec3(x, other.y, other.z) {}
	Vec3(Vec2<T> other, T z) : Vec3(other.x, other.y, z) {}
	constexpr Vec2<T> xx() const { return {this->x, this->x}; }
	constexpr Vec2<T> xy() const { return {this->x, this->y}; }
	constexpr Vec2<T> xz() const { return {this->x, this->z}; }
	constexpr Vec2<T> yx() const { return {this->y, this->x}; }
	constexpr Vec2<T> yy() const { return {this->y, this->y}; }
	constexpr Vec2<T> yz() const { return {this->y, this->z}; }
	constexpr Vec2<T> zx() const { return {this->z, this->x}; }
	constexpr Vec2<T> zy() const { return {this->z, this->y}; }
	constexpr Vec2<T> zz() const { return {this->z, this->z}; }
	constexpr Vec3<T> xxx() const { return {this->x, this->x, this->x}; }
	constexpr Vec3<T> xxy() const { return {this->x, this->x, this->y}; }
	constexpr Vec3<T> xxz() const { return {this->x, this->x, this->z}; }
	constexpr Vec3<T> xyx() const { return {this->x, this->y, this->x}; }
	constexpr Vec3<T> xyy() const { return {this->x, this->y, this->y}; }
	constexpr Vec3<T> xyz() const { return {this->x, this->y, this->z}; }
	constexpr Vec3<T> xzx() const { return {this->x, this->z, this->x}; }
	constexpr Vec3<T> xzy() const { return {this->x, this->z, this->y}; }
	constexpr Vec3<T> xzz() const { return {this->x, this->z, this->z}; }
	constexpr Vec3<T> yxx() const { return {this->y, this->x, this->x}; }
	constexpr Vec3<T> yxy() const { return {this->y, this->x, this->y}; }
	constexpr Vec3<T> yxz() const { return {this->y, this->x, this->z}; }
	constexpr Vec3<T> yyx() const { return {this->y, this->y, this->x}; }
	constexpr Vec3<T> yyy() const { return {this->y, this->y, this->y}; }
	constexpr Vec3<T> yyz() const { return {this->y, this->y, this->z}; }
	constexpr Vec3<T> yzx() const { return {this->y, this->z, this->x}; }
	constexpr Vec3<T> yzy() const { return {this->y, this->z, this->y}; }
	constexpr Vec3<T> yzz() const { return {this->y, this->z, this->z}; }
	constexpr Vec3<T> zxx() const { return {this->z, this->x, this->x}; }
	constexpr Vec3<T> zxy() const { return {this->z, this->x, this->y}; }
	constexpr Vec3<T> zxz() const { return {this->z, this->x, this->z}; }
	constexpr Vec3<T> zyx() const { return {this->z, this->y, this->x}; }
	constexpr Vec3<T> zyy() const { return {this->z, this->y, this->y}; }
	constexpr Vec3<T> zyz() const { return {this->z, this->y, this->z}; }
	constexpr Vec3<T> zzx() const { return {this->z, this->z, this->x}; }
	constexpr Vec3<T> zzy() const { return {this->z, this->z, this->y}; }
	constexpr Vec3<T> zzz() const { return {this->z, this->z, this->z}; }
};

// c i hate you
template<> inline Vec3<float64> Vec3<float64>::operator%(Vec3<float64> r) {
	return Vec3<float64>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z));
}
template<> inline Vec3<float64> Vec3<float64>::operator%(float64 r) {
	return Vec3<float64>(fmod(this->x, r), fmod(this->y, r), fmod(this->z, r));
}
template<> inline Vec3<float32> Vec3<float32>::operator%(Vec3<float32> r) {
	return Vec3<float32>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z));
}
template<> inline Vec3<float32> Vec3<float32>::operator%(float32 r) {
	return Vec3<float32>(fmod(this->x, r), fmod(this->y, r), fmod(this->z, r));
}

// Vec4 lmao
template<typename T>
struct Vec4
{
	T x;
	T y;
	T z;
	T w;

	constexpr Vec4() : x(0), y(0), z(0), w(0) {};
	constexpr Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {};

	Vec4 operator+(Vec4 r)  { return Vec4(this->x + r.x, this->y + r.y, this->z + r.z, this->w + r.w);  }
	Vec4 operator-(Vec4 r)  { return Vec4(this->x - r.x, this->y - r.y, this->z - r.z, this->w - r.w);  }
	Vec4 operator*(Vec4 r)  { return Vec4(this->x * r.x, this->y * r.y, this->z * r.z, this->w * r.w);  }
	Vec4 operator*(T r)     { return Vec4(this->x * r,   this->y * r,   this->z * r,   this->w * r);    }
	Vec4 operator/(Vec4 r)  { return Vec4(this->x / r.x, this->y / r.y, this->z / r.z, this->w / r.z);  }
	Vec4 operator/(T r)     { return Vec4(this->x / r,   this->y / r,   this->z / r,   this->w / r);    }
	Vec4 operator%(Vec4 r)  { return Vec4(this->x % r.x, this->y % r.y, this->z % r.z, this->w % r.z);  }
	Vec4 operator%(T r)     { return Vec4(this->x % r,   this->y % r,   this->z % r,   this->w % r);    }

	bool operator==(Vec4 r) { return this->x == r.x && this->y == r.y && this->z == r.z && this->w == r.w; }
	bool operator!=(Vec4 r) { return this->x != r.x && this->y != r.y && this->z != r.z && this->w != r.w; }
	bool operator>(Vec4 r)  { return this->x >  r.x && this->y >  r.y && this->z >  r.z && this->w >  r.w; }
	bool operator>=(Vec4 r) { return this->x >= r.x && this->y >= r.y && this->z >= r.z && this->w >= r.w; }
	bool operator<(Vec4 r)  { return this->x <  r.x && this->y <  r.y && this->z <  r.z && this->w <  r.w; }
	bool operator<=(Vec4 r) { return this->x <= r.x && this->y <= r.y && this->z <= r.z && this->w <= r.w; }

	Vec4& operator+=(Vec4 other) { return *this = *this + other; }
	Vec4& operator-=(Vec4 other) { return *this = *this - other; }
	Vec4& operator*=(Vec4 other) { return *this = *this * other; }
	Vec4& operator/=(Vec4 other) { return *this = *this / other; }
	Vec4& operator%=(Vec4 other) { return *this = *this % other; }
	Vec4& operator*=(T other) { return *this = *this * other; }
	Vec4& operator/=(T other) { return *this = *this / other; }
	Vec4& operator%=(T other) { return *this = *this % other; }
	Vec4 operator-() { return {-this->x, -this->y}; }
	Vec4 operator+() { return {+this->x, +this->y}; }

	static constexpr usize ITEMS = 4;
	T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return this->x; break;
			case 1:  return this->y; break;
			case 2:  return this->z; break;
			case 3:  return this->w; break;
			default: tr::panic("sir this is a vec4<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	T mul_inner(Vec4<T> b)
	{
		T p = 0;
		for (usize i = 0; i < ITEMS; i++) {
			p += b[i] * (*this)[i];
		}
		return p;
	}

	float64 length() { return sqrt(this->mul_inner(*this)); }
	Vec4<T> normalize() { return *this * (1 / this->length()); }

	Vec4<T> mul_cross(Vec4<T> b)
	{
		Vec4<T> r;
		r[0] = (*this)[1] * b[2] - (*this)[2] * b[1];
		r[1] = (*this)[2] * b[0] - (*this)[0] * b[2];
		r[2] = (*this)[0] * b[1] - (*this)[1] * b[0];
		r[3] = 1.f;
		return r;
	}

	Vec4<T> reflect(Vec4<T> b)
	{
		Vec4<T> r;
		T p = 2 * this->mul_inner(b);
		for (usize i = 0; i < ITEMS; i++) {
			r[i] = (*this)[i] - p * b[i];
		}
		return r;
	}

	// swizzling operators
	// i know this is insane
	// i didnt write this myself im not insane
	Vec4(T x, T y, Vec2<T> other) : Vec4(x, y, other.z, other.w) {}
	Vec4(Vec2<T> other, T z, T w) : Vec4(other.x, other.y, z, w) {}
	Vec4(T x, Vec2<T> other, T w) : Vec4(x, other.y, other.z, w) {}
	Vec4(T x, Vec3<T> other) : Vec4(x, other.y, other.z, other.w) {}
	Vec4(Vec3<T> other, T w) : Vec4(other.x, other.y, other.z, w) {}
	constexpr Vec2<T> xx() const { return {this->x, this->x}; }
	constexpr Vec2<T> xy() const { return {this->x, this->y}; }
	constexpr Vec2<T> xz() const { return {this->x, this->z}; }
	constexpr Vec2<T> xw() const { return {this->x, this->w}; }
	constexpr Vec2<T> yx() const { return {this->y, this->x}; }
	constexpr Vec2<T> yy() const { return {this->y, this->y}; }
	constexpr Vec2<T> yz() const { return {this->y, this->z}; }
	constexpr Vec2<T> yw() const { return {this->y, this->w}; }
	constexpr Vec2<T> zx() const { return {this->z, this->x}; }
	constexpr Vec2<T> zy() const { return {this->z, this->y}; }
	constexpr Vec2<T> zz() const { return {this->z, this->z}; }
	constexpr Vec2<T> zw() const { return {this->z, this->w}; }
	constexpr Vec2<T> wx() const { return {this->w, this->x}; }
	constexpr Vec2<T> wy() const { return {this->w, this->y}; }
	constexpr Vec2<T> wz() const { return {this->w, this->z}; }
	constexpr Vec2<T> ww() const { return {this->w, this->w}; }
	constexpr Vec3<T> xxx() const { return {this->x, this->x, this->x}; }
	constexpr Vec3<T> xxy() const { return {this->x, this->x, this->y}; }
	constexpr Vec3<T> xxz() const { return {this->x, this->x, this->z}; }
	constexpr Vec3<T> xxw() const { return {this->x, this->x, this->w}; }
	constexpr Vec3<T> xyx() const { return {this->x, this->y, this->x}; }
	constexpr Vec3<T> xyy() const { return {this->x, this->y, this->y}; }
	constexpr Vec3<T> xyz() const { return {this->x, this->y, this->z}; }
	constexpr Vec3<T> xyw() const { return {this->x, this->y, this->w}; }
	constexpr Vec3<T> xzx() const { return {this->x, this->z, this->x}; }
	constexpr Vec3<T> xzy() const { return {this->x, this->z, this->y}; }
	constexpr Vec3<T> xzz() const { return {this->x, this->z, this->z}; }
	constexpr Vec3<T> xzw() const { return {this->x, this->z, this->w}; }
	constexpr Vec3<T> xwx() const { return {this->x, this->w, this->x}; }
	constexpr Vec3<T> xwy() const { return {this->x, this->w, this->y}; }
	constexpr Vec3<T> xwz() const { return {this->x, this->w, this->z}; }
	constexpr Vec3<T> xww() const { return {this->x, this->w, this->w}; }
	constexpr Vec3<T> yxx() const { return {this->y, this->x, this->x}; }
	constexpr Vec3<T> yxy() const { return {this->y, this->x, this->y}; }
	constexpr Vec3<T> yxz() const { return {this->y, this->x, this->z}; }
	constexpr Vec3<T> yxw() const { return {this->y, this->x, this->w}; }
	constexpr Vec3<T> yyx() const { return {this->y, this->y, this->x}; }
	constexpr Vec3<T> yyy() const { return {this->y, this->y, this->y}; }
	constexpr Vec3<T> yyz() const { return {this->y, this->y, this->z}; }
	constexpr Vec3<T> yyw() const { return {this->y, this->y, this->w}; }
	constexpr Vec3<T> yzx() const { return {this->y, this->z, this->x}; }
	constexpr Vec3<T> yzy() const { return {this->y, this->z, this->y}; }
	constexpr Vec3<T> yzz() const { return {this->y, this->z, this->z}; }
	constexpr Vec3<T> yzw() const { return {this->y, this->z, this->w}; }
	constexpr Vec3<T> ywx() const { return {this->y, this->w, this->x}; }
	constexpr Vec3<T> ywy() const { return {this->y, this->w, this->y}; }
	constexpr Vec3<T> ywz() const { return {this->y, this->w, this->z}; }
	constexpr Vec3<T> yww() const { return {this->y, this->w, this->w}; }
	constexpr Vec3<T> zxx() const { return {this->z, this->x, this->x}; }
	constexpr Vec3<T> zxy() const { return {this->z, this->x, this->y}; }
	constexpr Vec3<T> zxz() const { return {this->z, this->x, this->z}; }
	constexpr Vec3<T> zxw() const { return {this->z, this->x, this->w}; }
	constexpr Vec3<T> zyx() const { return {this->z, this->y, this->x}; }
	constexpr Vec3<T> zyy() const { return {this->z, this->y, this->y}; }
	constexpr Vec3<T> zyz() const { return {this->z, this->y, this->z}; }
	constexpr Vec3<T> zyw() const { return {this->z, this->y, this->w}; }
	constexpr Vec3<T> zzx() const { return {this->z, this->z, this->x}; }
	constexpr Vec3<T> zzy() const { return {this->z, this->z, this->y}; }
	constexpr Vec3<T> zzz() const { return {this->z, this->z, this->z}; }
	constexpr Vec3<T> zzw() const { return {this->z, this->z, this->w}; }
	constexpr Vec3<T> zwx() const { return {this->z, this->w, this->x}; }
	constexpr Vec3<T> zwy() const { return {this->z, this->w, this->y}; }
	constexpr Vec3<T> zwz() const { return {this->z, this->w, this->z}; }
	constexpr Vec3<T> zww() const { return {this->z, this->w, this->w}; }
	constexpr Vec3<T> wxx() const { return {this->w, this->x, this->x}; }
	constexpr Vec3<T> wxy() const { return {this->w, this->x, this->y}; }
	constexpr Vec3<T> wxz() const { return {this->w, this->x, this->z}; }
	constexpr Vec3<T> wxw() const { return {this->w, this->x, this->w}; }
	constexpr Vec3<T> wyx() const { return {this->w, this->y, this->x}; }
	constexpr Vec3<T> wyy() const { return {this->w, this->y, this->y}; }
	constexpr Vec3<T> wyz() const { return {this->w, this->y, this->z}; }
	constexpr Vec3<T> wyw() const { return {this->w, this->y, this->w}; }
	constexpr Vec3<T> wzx() const { return {this->w, this->z, this->x}; }
	constexpr Vec3<T> wzy() const { return {this->w, this->z, this->y}; }
	constexpr Vec3<T> wzz() const { return {this->w, this->z, this->z}; }
	constexpr Vec3<T> wzw() const { return {this->w, this->z, this->w}; }
	constexpr Vec3<T> wwx() const { return {this->w, this->w, this->x}; }
	constexpr Vec3<T> wwy() const { return {this->w, this->w, this->y}; }
	constexpr Vec3<T> wwz() const { return {this->w, this->w, this->z}; }
	constexpr Vec3<T> www() const { return {this->w, this->w, this->w}; }
	constexpr Vec4<T> xxxx() const { return {this->x, this->x, this->x, this->x}; }
	constexpr Vec4<T> xxxy() const { return {this->x, this->x, this->x, this->y}; }
	constexpr Vec4<T> xxxz() const { return {this->x, this->x, this->x, this->z}; }
	constexpr Vec4<T> xxxw() const { return {this->x, this->x, this->x, this->w}; }
	constexpr Vec4<T> xxyx() const { return {this->x, this->x, this->y, this->x}; }
	constexpr Vec4<T> xxyy() const { return {this->x, this->x, this->y, this->y}; }
	constexpr Vec4<T> xxyz() const { return {this->x, this->x, this->y, this->z}; }
	constexpr Vec4<T> xxyw() const { return {this->x, this->x, this->y, this->w}; }
	constexpr Vec4<T> xxzx() const { return {this->x, this->x, this->z, this->x}; }
	constexpr Vec4<T> xxzy() const { return {this->x, this->x, this->z, this->y}; }
	constexpr Vec4<T> xxzz() const { return {this->x, this->x, this->z, this->z}; }
	constexpr Vec4<T> xxzw() const { return {this->x, this->x, this->z, this->w}; }
	constexpr Vec4<T> xxwx() const { return {this->x, this->x, this->w, this->x}; }
	constexpr Vec4<T> xxwy() const { return {this->x, this->x, this->w, this->y}; }
	constexpr Vec4<T> xxwz() const { return {this->x, this->x, this->w, this->z}; }
	constexpr Vec4<T> xxww() const { return {this->x, this->x, this->w, this->w}; }
	constexpr Vec4<T> xyxx() const { return {this->x, this->y, this->x, this->x}; }
	constexpr Vec4<T> xyxy() const { return {this->x, this->y, this->x, this->y}; }
	constexpr Vec4<T> xyxz() const { return {this->x, this->y, this->x, this->z}; }
	constexpr Vec4<T> xyxw() const { return {this->x, this->y, this->x, this->w}; }
	constexpr Vec4<T> xyyx() const { return {this->x, this->y, this->y, this->x}; }
	constexpr Vec4<T> xyyy() const { return {this->x, this->y, this->y, this->y}; }
	constexpr Vec4<T> xyyz() const { return {this->x, this->y, this->y, this->z}; }
	constexpr Vec4<T> xyyw() const { return {this->x, this->y, this->y, this->w}; }
	constexpr Vec4<T> xyzx() const { return {this->x, this->y, this->z, this->x}; }
	constexpr Vec4<T> xyzy() const { return {this->x, this->y, this->z, this->y}; }
	constexpr Vec4<T> xyzz() const { return {this->x, this->y, this->z, this->z}; }
	constexpr Vec4<T> xyzw() const { return {this->x, this->y, this->z, this->w}; }
	constexpr Vec4<T> xywx() const { return {this->x, this->y, this->w, this->x}; }
	constexpr Vec4<T> xywy() const { return {this->x, this->y, this->w, this->y}; }
	constexpr Vec4<T> xywz() const { return {this->x, this->y, this->w, this->z}; }
	constexpr Vec4<T> xyww() const { return {this->x, this->y, this->w, this->w}; }
	constexpr Vec4<T> xzxx() const { return {this->x, this->z, this->x, this->x}; }
	constexpr Vec4<T> xzxy() const { return {this->x, this->z, this->x, this->y}; }
	constexpr Vec4<T> xzxz() const { return {this->x, this->z, this->x, this->z}; }
	constexpr Vec4<T> xzxw() const { return {this->x, this->z, this->x, this->w}; }
	constexpr Vec4<T> xzyx() const { return {this->x, this->z, this->y, this->x}; }
	constexpr Vec4<T> xzyy() const { return {this->x, this->z, this->y, this->y}; }
	constexpr Vec4<T> xzyz() const { return {this->x, this->z, this->y, this->z}; }
	constexpr Vec4<T> xzyw() const { return {this->x, this->z, this->y, this->w}; }
	constexpr Vec4<T> xzzx() const { return {this->x, this->z, this->z, this->x}; }
	constexpr Vec4<T> xzzy() const { return {this->x, this->z, this->z, this->y}; }
	constexpr Vec4<T> xzzz() const { return {this->x, this->z, this->z, this->z}; }
	constexpr Vec4<T> xzzw() const { return {this->x, this->z, this->z, this->w}; }
	constexpr Vec4<T> xzwx() const { return {this->x, this->z, this->w, this->x}; }
	constexpr Vec4<T> xzwy() const { return {this->x, this->z, this->w, this->y}; }
	constexpr Vec4<T> xzwz() const { return {this->x, this->z, this->w, this->z}; }
	constexpr Vec4<T> xzww() const { return {this->x, this->z, this->w, this->w}; }
	constexpr Vec4<T> xwxx() const { return {this->x, this->w, this->x, this->x}; }
	constexpr Vec4<T> xwxy() const { return {this->x, this->w, this->x, this->y}; }
	constexpr Vec4<T> xwxz() const { return {this->x, this->w, this->x, this->z}; }
	constexpr Vec4<T> xwxw() const { return {this->x, this->w, this->x, this->w}; }
	constexpr Vec4<T> xwyx() const { return {this->x, this->w, this->y, this->x}; }
	constexpr Vec4<T> xwyy() const { return {this->x, this->w, this->y, this->y}; }
	constexpr Vec4<T> xwyz() const { return {this->x, this->w, this->y, this->z}; }
	constexpr Vec4<T> xwyw() const { return {this->x, this->w, this->y, this->w}; }
	constexpr Vec4<T> xwzx() const { return {this->x, this->w, this->z, this->x}; }
	constexpr Vec4<T> xwzy() const { return {this->x, this->w, this->z, this->y}; }
	constexpr Vec4<T> xwzz() const { return {this->x, this->w, this->z, this->z}; }
	constexpr Vec4<T> xwzw() const { return {this->x, this->w, this->z, this->w}; }
	constexpr Vec4<T> xwwx() const { return {this->x, this->w, this->w, this->x}; }
	constexpr Vec4<T> xwwy() const { return {this->x, this->w, this->w, this->y}; }
	constexpr Vec4<T> xwwz() const { return {this->x, this->w, this->w, this->z}; }
	constexpr Vec4<T> xwww() const { return {this->x, this->w, this->w, this->w}; }
	constexpr Vec4<T> yxxx() const { return {this->y, this->x, this->x, this->x}; }
	constexpr Vec4<T> yxxy() const { return {this->y, this->x, this->x, this->y}; }
	constexpr Vec4<T> yxxz() const { return {this->y, this->x, this->x, this->z}; }
	constexpr Vec4<T> yxxw() const { return {this->y, this->x, this->x, this->w}; }
	constexpr Vec4<T> yxyx() const { return {this->y, this->x, this->y, this->x}; }
	constexpr Vec4<T> yxyy() const { return {this->y, this->x, this->y, this->y}; }
	constexpr Vec4<T> yxyz() const { return {this->y, this->x, this->y, this->z}; }
	constexpr Vec4<T> yxyw() const { return {this->y, this->x, this->y, this->w}; }
	constexpr Vec4<T> yxzx() const { return {this->y, this->x, this->z, this->x}; }
	constexpr Vec4<T> yxzy() const { return {this->y, this->x, this->z, this->y}; }
	constexpr Vec4<T> yxzz() const { return {this->y, this->x, this->z, this->z}; }
	constexpr Vec4<T> yxzw() const { return {this->y, this->x, this->z, this->w}; }
	constexpr Vec4<T> yxwx() const { return {this->y, this->x, this->w, this->x}; }
	constexpr Vec4<T> yxwy() const { return {this->y, this->x, this->w, this->y}; }
	constexpr Vec4<T> yxwz() const { return {this->y, this->x, this->w, this->z}; }
	constexpr Vec4<T> yxww() const { return {this->y, this->x, this->w, this->w}; }
	constexpr Vec4<T> yyxx() const { return {this->y, this->y, this->x, this->x}; }
	constexpr Vec4<T> yyxy() const { return {this->y, this->y, this->x, this->y}; }
	constexpr Vec4<T> yyxz() const { return {this->y, this->y, this->x, this->z}; }
	constexpr Vec4<T> yyxw() const { return {this->y, this->y, this->x, this->w}; }
	constexpr Vec4<T> yyyx() const { return {this->y, this->y, this->y, this->x}; }
	constexpr Vec4<T> yyyy() const { return {this->y, this->y, this->y, this->y}; }
	constexpr Vec4<T> yyyz() const { return {this->y, this->y, this->y, this->z}; }
	constexpr Vec4<T> yyyw() const { return {this->y, this->y, this->y, this->w}; }
	constexpr Vec4<T> yyzx() const { return {this->y, this->y, this->z, this->x}; }
	constexpr Vec4<T> yyzy() const { return {this->y, this->y, this->z, this->y}; }
	constexpr Vec4<T> yyzz() const { return {this->y, this->y, this->z, this->z}; }
	constexpr Vec4<T> yyzw() const { return {this->y, this->y, this->z, this->w}; }
	constexpr Vec4<T> yywx() const { return {this->y, this->y, this->w, this->x}; }
	constexpr Vec4<T> yywy() const { return {this->y, this->y, this->w, this->y}; }
	constexpr Vec4<T> yywz() const { return {this->y, this->y, this->w, this->z}; }
	constexpr Vec4<T> yyww() const { return {this->y, this->y, this->w, this->w}; }
	constexpr Vec4<T> yzxx() const { return {this->y, this->z, this->x, this->x}; }
	constexpr Vec4<T> yzxy() const { return {this->y, this->z, this->x, this->y}; }
	constexpr Vec4<T> yzxz() const { return {this->y, this->z, this->x, this->z}; }
	constexpr Vec4<T> yzxw() const { return {this->y, this->z, this->x, this->w}; }
	constexpr Vec4<T> yzyx() const { return {this->y, this->z, this->y, this->x}; }
	constexpr Vec4<T> yzyy() const { return {this->y, this->z, this->y, this->y}; }
	constexpr Vec4<T> yzyz() const { return {this->y, this->z, this->y, this->z}; }
	constexpr Vec4<T> yzyw() const { return {this->y, this->z, this->y, this->w}; }
	constexpr Vec4<T> yzzx() const { return {this->y, this->z, this->z, this->x}; }
	constexpr Vec4<T> yzzy() const { return {this->y, this->z, this->z, this->y}; }
	constexpr Vec4<T> yzzz() const { return {this->y, this->z, this->z, this->z}; }
	constexpr Vec4<T> yzzw() const { return {this->y, this->z, this->z, this->w}; }
	constexpr Vec4<T> yzwx() const { return {this->y, this->z, this->w, this->x}; }
	constexpr Vec4<T> yzwy() const { return {this->y, this->z, this->w, this->y}; }
	constexpr Vec4<T> yzwz() const { return {this->y, this->z, this->w, this->z}; }
	constexpr Vec4<T> yzww() const { return {this->y, this->z, this->w, this->w}; }
	constexpr Vec4<T> ywxx() const { return {this->y, this->w, this->x, this->x}; }
	constexpr Vec4<T> ywxy() const { return {this->y, this->w, this->x, this->y}; }
	constexpr Vec4<T> ywxz() const { return {this->y, this->w, this->x, this->z}; }
	constexpr Vec4<T> ywxw() const { return {this->y, this->w, this->x, this->w}; }
	constexpr Vec4<T> ywyx() const { return {this->y, this->w, this->y, this->x}; }
	constexpr Vec4<T> ywyy() const { return {this->y, this->w, this->y, this->y}; }
	constexpr Vec4<T> ywyz() const { return {this->y, this->w, this->y, this->z}; }
	constexpr Vec4<T> ywyw() const { return {this->y, this->w, this->y, this->w}; }
	constexpr Vec4<T> ywzx() const { return {this->y, this->w, this->z, this->x}; }
	constexpr Vec4<T> ywzy() const { return {this->y, this->w, this->z, this->y}; }
	constexpr Vec4<T> ywzz() const { return {this->y, this->w, this->z, this->z}; }
	constexpr Vec4<T> ywzw() const { return {this->y, this->w, this->z, this->w}; }
	constexpr Vec4<T> ywwx() const { return {this->y, this->w, this->w, this->x}; }
	constexpr Vec4<T> ywwy() const { return {this->y, this->w, this->w, this->y}; }
	constexpr Vec4<T> ywwz() const { return {this->y, this->w, this->w, this->z}; }
	constexpr Vec4<T> ywww() const { return {this->y, this->w, this->w, this->w}; }
	constexpr Vec4<T> zxxx() const { return {this->z, this->x, this->x, this->x}; }
	constexpr Vec4<T> zxxy() const { return {this->z, this->x, this->x, this->y}; }
	constexpr Vec4<T> zxxz() const { return {this->z, this->x, this->x, this->z}; }
	constexpr Vec4<T> zxxw() const { return {this->z, this->x, this->x, this->w}; }
	constexpr Vec4<T> zxyx() const { return {this->z, this->x, this->y, this->x}; }
	constexpr Vec4<T> zxyy() const { return {this->z, this->x, this->y, this->y}; }
	constexpr Vec4<T> zxyz() const { return {this->z, this->x, this->y, this->z}; }
	constexpr Vec4<T> zxyw() const { return {this->z, this->x, this->y, this->w}; }
	constexpr Vec4<T> zxzx() const { return {this->z, this->x, this->z, this->x}; }
	constexpr Vec4<T> zxzy() const { return {this->z, this->x, this->z, this->y}; }
	constexpr Vec4<T> zxzz() const { return {this->z, this->x, this->z, this->z}; }
	constexpr Vec4<T> zxzw() const { return {this->z, this->x, this->z, this->w}; }
	constexpr Vec4<T> zxwx() const { return {this->z, this->x, this->w, this->x}; }
	constexpr Vec4<T> zxwy() const { return {this->z, this->x, this->w, this->y}; }
	constexpr Vec4<T> zxwz() const { return {this->z, this->x, this->w, this->z}; }
	constexpr Vec4<T> zxww() const { return {this->z, this->x, this->w, this->w}; }
	constexpr Vec4<T> zyxx() const { return {this->z, this->y, this->x, this->x}; }
	constexpr Vec4<T> zyxy() const { return {this->z, this->y, this->x, this->y}; }
	constexpr Vec4<T> zyxz() const { return {this->z, this->y, this->x, this->z}; }
	constexpr Vec4<T> zyxw() const { return {this->z, this->y, this->x, this->w}; }
	constexpr Vec4<T> zyyx() const { return {this->z, this->y, this->y, this->x}; }
	constexpr Vec4<T> zyyy() const { return {this->z, this->y, this->y, this->y}; }
	constexpr Vec4<T> zyyz() const { return {this->z, this->y, this->y, this->z}; }
	constexpr Vec4<T> zyyw() const { return {this->z, this->y, this->y, this->w}; }
	constexpr Vec4<T> zyzx() const { return {this->z, this->y, this->z, this->x}; }
	constexpr Vec4<T> zyzy() const { return {this->z, this->y, this->z, this->y}; }
	constexpr Vec4<T> zyzz() const { return {this->z, this->y, this->z, this->z}; }
	constexpr Vec4<T> zyzw() const { return {this->z, this->y, this->z, this->w}; }
	constexpr Vec4<T> zywx() const { return {this->z, this->y, this->w, this->x}; }
	constexpr Vec4<T> zywy() const { return {this->z, this->y, this->w, this->y}; }
	constexpr Vec4<T> zywz() const { return {this->z, this->y, this->w, this->z}; }
	constexpr Vec4<T> zyww() const { return {this->z, this->y, this->w, this->w}; }
	constexpr Vec4<T> zzxx() const { return {this->z, this->z, this->x, this->x}; }
	constexpr Vec4<T> zzxy() const { return {this->z, this->z, this->x, this->y}; }
	constexpr Vec4<T> zzxz() const { return {this->z, this->z, this->x, this->z}; }
	constexpr Vec4<T> zzxw() const { return {this->z, this->z, this->x, this->w}; }
	constexpr Vec4<T> zzyx() const { return {this->z, this->z, this->y, this->x}; }
	constexpr Vec4<T> zzyy() const { return {this->z, this->z, this->y, this->y}; }
	constexpr Vec4<T> zzyz() const { return {this->z, this->z, this->y, this->z}; }
	constexpr Vec4<T> zzyw() const { return {this->z, this->z, this->y, this->w}; }
	constexpr Vec4<T> zzzx() const { return {this->z, this->z, this->z, this->x}; }
	constexpr Vec4<T> zzzy() const { return {this->z, this->z, this->z, this->y}; }
	constexpr Vec4<T> zzzz() const { return {this->z, this->z, this->z, this->z}; }
	constexpr Vec4<T> zzzw() const { return {this->z, this->z, this->z, this->w}; }
	constexpr Vec4<T> zzwx() const { return {this->z, this->z, this->w, this->x}; }
	constexpr Vec4<T> zzwy() const { return {this->z, this->z, this->w, this->y}; }
	constexpr Vec4<T> zzwz() const { return {this->z, this->z, this->w, this->z}; }
	constexpr Vec4<T> zzww() const { return {this->z, this->z, this->w, this->w}; }
	constexpr Vec4<T> zwxx() const { return {this->z, this->w, this->x, this->x}; }
	constexpr Vec4<T> zwxy() const { return {this->z, this->w, this->x, this->y}; }
	constexpr Vec4<T> zwxz() const { return {this->z, this->w, this->x, this->z}; }
	constexpr Vec4<T> zwxw() const { return {this->z, this->w, this->x, this->w}; }
	constexpr Vec4<T> zwyx() const { return {this->z, this->w, this->y, this->x}; }
	constexpr Vec4<T> zwyy() const { return {this->z, this->w, this->y, this->y}; }
	constexpr Vec4<T> zwyz() const { return {this->z, this->w, this->y, this->z}; }
	constexpr Vec4<T> zwyw() const { return {this->z, this->w, this->y, this->w}; }
	constexpr Vec4<T> zwzx() const { return {this->z, this->w, this->z, this->x}; }
	constexpr Vec4<T> zwzy() const { return {this->z, this->w, this->z, this->y}; }
	constexpr Vec4<T> zwzz() const { return {this->z, this->w, this->z, this->z}; }
	constexpr Vec4<T> zwzw() const { return {this->z, this->w, this->z, this->w}; }
	constexpr Vec4<T> zwwx() const { return {this->z, this->w, this->w, this->x}; }
	constexpr Vec4<T> zwwy() const { return {this->z, this->w, this->w, this->y}; }
	constexpr Vec4<T> zwwz() const { return {this->z, this->w, this->w, this->z}; }
	constexpr Vec4<T> zwww() const { return {this->z, this->w, this->w, this->w}; }
	constexpr Vec4<T> wxxx() const { return {this->w, this->x, this->x, this->x}; }
	constexpr Vec4<T> wxxy() const { return {this->w, this->x, this->x, this->y}; }
	constexpr Vec4<T> wxxz() const { return {this->w, this->x, this->x, this->z}; }
	constexpr Vec4<T> wxxw() const { return {this->w, this->x, this->x, this->w}; }
	constexpr Vec4<T> wxyx() const { return {this->w, this->x, this->y, this->x}; }
	constexpr Vec4<T> wxyy() const { return {this->w, this->x, this->y, this->y}; }
	constexpr Vec4<T> wxyz() const { return {this->w, this->x, this->y, this->z}; }
	constexpr Vec4<T> wxyw() const { return {this->w, this->x, this->y, this->w}; }
	constexpr Vec4<T> wxzx() const { return {this->w, this->x, this->z, this->x}; }
	constexpr Vec4<T> wxzy() const { return {this->w, this->x, this->z, this->y}; }
	constexpr Vec4<T> wxzz() const { return {this->w, this->x, this->z, this->z}; }
	constexpr Vec4<T> wxzw() const { return {this->w, this->x, this->z, this->w}; }
	constexpr Vec4<T> wxwx() const { return {this->w, this->x, this->w, this->x}; }
	constexpr Vec4<T> wxwy() const { return {this->w, this->x, this->w, this->y}; }
	constexpr Vec4<T> wxwz() const { return {this->w, this->x, this->w, this->z}; }
	constexpr Vec4<T> wxww() const { return {this->w, this->x, this->w, this->w}; }
	constexpr Vec4<T> wyxx() const { return {this->w, this->y, this->x, this->x}; }
	constexpr Vec4<T> wyxy() const { return {this->w, this->y, this->x, this->y}; }
	constexpr Vec4<T> wyxz() const { return {this->w, this->y, this->x, this->z}; }
	constexpr Vec4<T> wyxw() const { return {this->w, this->y, this->x, this->w}; }
	constexpr Vec4<T> wyyx() const { return {this->w, this->y, this->y, this->x}; }
	constexpr Vec4<T> wyyy() const { return {this->w, this->y, this->y, this->y}; }
	constexpr Vec4<T> wyyz() const { return {this->w, this->y, this->y, this->z}; }
	constexpr Vec4<T> wyyw() const { return {this->w, this->y, this->y, this->w}; }
	constexpr Vec4<T> wyzx() const { return {this->w, this->y, this->z, this->x}; }
	constexpr Vec4<T> wyzy() const { return {this->w, this->y, this->z, this->y}; }
	constexpr Vec4<T> wyzz() const { return {this->w, this->y, this->z, this->z}; }
	constexpr Vec4<T> wyzw() const { return {this->w, this->y, this->z, this->w}; }
	constexpr Vec4<T> wywx() const { return {this->w, this->y, this->w, this->x}; }
	constexpr Vec4<T> wywy() const { return {this->w, this->y, this->w, this->y}; }
	constexpr Vec4<T> wywz() const { return {this->w, this->y, this->w, this->z}; }
	constexpr Vec4<T> wyww() const { return {this->w, this->y, this->w, this->w}; }
	constexpr Vec4<T> wzxx() const { return {this->w, this->z, this->x, this->x}; }
	constexpr Vec4<T> wzxy() const { return {this->w, this->z, this->x, this->y}; }
	constexpr Vec4<T> wzxz() const { return {this->w, this->z, this->x, this->z}; }
	constexpr Vec4<T> wzxw() const { return {this->w, this->z, this->x, this->w}; }
	constexpr Vec4<T> wzyx() const { return {this->w, this->z, this->y, this->x}; }
	constexpr Vec4<T> wzyy() const { return {this->w, this->z, this->y, this->y}; }
	constexpr Vec4<T> wzyz() const { return {this->w, this->z, this->y, this->z}; }
	constexpr Vec4<T> wzyw() const { return {this->w, this->z, this->y, this->w}; }
	constexpr Vec4<T> wzzx() const { return {this->w, this->z, this->z, this->x}; }
	constexpr Vec4<T> wzzy() const { return {this->w, this->z, this->z, this->y}; }
	constexpr Vec4<T> wzzz() const { return {this->w, this->z, this->z, this->z}; }
	constexpr Vec4<T> wzzw() const { return {this->w, this->z, this->z, this->w}; }
	constexpr Vec4<T> wzwx() const { return {this->w, this->z, this->w, this->x}; }
	constexpr Vec4<T> wzwy() const { return {this->w, this->z, this->w, this->y}; }
	constexpr Vec4<T> wzwz() const { return {this->w, this->z, this->w, this->z}; }
	constexpr Vec4<T> wzww() const { return {this->w, this->z, this->w, this->w}; }
	constexpr Vec4<T> wwxx() const { return {this->w, this->w, this->x, this->x}; }
	constexpr Vec4<T> wwxy() const { return {this->w, this->w, this->x, this->y}; }
	constexpr Vec4<T> wwxz() const { return {this->w, this->w, this->x, this->z}; }
	constexpr Vec4<T> wwxw() const { return {this->w, this->w, this->x, this->w}; }
	constexpr Vec4<T> wwyx() const { return {this->w, this->w, this->y, this->x}; }
	constexpr Vec4<T> wwyy() const { return {this->w, this->w, this->y, this->y}; }
	constexpr Vec4<T> wwyz() const { return {this->w, this->w, this->y, this->z}; }
	constexpr Vec4<T> wwyw() const { return {this->w, this->w, this->y, this->w}; }
	constexpr Vec4<T> wwzx() const { return {this->w, this->w, this->z, this->x}; }
	constexpr Vec4<T> wwzy() const { return {this->w, this->w, this->z, this->y}; }
	constexpr Vec4<T> wwzz() const { return {this->w, this->w, this->z, this->z}; }
	constexpr Vec4<T> wwzw() const { return {this->w, this->w, this->z, this->w}; }
	constexpr Vec4<T> wwwx() const { return {this->w, this->w, this->w, this->x}; }
	constexpr Vec4<T> wwwy() const { return {this->w, this->w, this->w, this->y}; }
	constexpr Vec4<T> wwwz() const { return {this->w, this->w, this->w, this->z}; }
	constexpr Vec4<T> wwww() const { return {this->w, this->w, this->w, this->w}; }
};

// c i hate you
template<> inline Vec4<float64> Vec4<float64>::operator%(Vec4<float64> r) {
	return Vec4<float64>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z), fmod(this->w, r.w));
}
template<> inline Vec4<float64> Vec4<float64>::operator%(float64 r) {
	return Vec4<float64>(fmod(this->x, r), fmod(this->y, r), fmod(this->z, r), fmod(this->w, r));
}
template<> inline Vec4<float32> Vec4<float32>::operator%(Vec4<float32> r) {
	return Vec4<float32>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z), fmod(this->w, r.w));
}
template<> inline Vec4<float32> Vec4<float32>::operator%(float32 r) {
	return Vec4<float32>(fmod(this->x, r), fmod(this->y, r), fmod(this->z, r), fmod(this->w, r));
}

// SO RANDOM LMAO HAHA implemented through xoshiro256+
struct Random
{
private:
	uint64 state[4];

public:
	// Initializes the `tr::Random` with a seed
	explicit Random(int64 seed);

	// Initializes the `tr::Random` with the current time as the seed
	Random() : Random(time(nullptr)) {}

	// Returns a value from 0 to 1
	float64 next();

	// Returns a value in a range
	template<typename T> T next(T min, T max)
	{
		return static_cast<T>((this->next() * max) + min);
	}
};

// COLOR I HARDLY KNOW 'ER HAHAHHAHA LAUGH IMMEIDATLEY
struct Color
{
	// Red
	uint8 r = 255;
	// Green
	uint8 g = 255;
	// Blue
	uint8 b = 255;
	// Alpha/transparency
	uint8 a = 255;

	constexpr Color();
	constexpr Color(uint8 r, uint8 g, uint8 b) : r(r), g(g), b(b), a(255) {}
	constexpr Color(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) {}
	constexpr Color(Vec4<float32> vec) : r(vec.x * 255), g(vec.y * 255), b(vec.z * 255), a(vec.w * 255) {}

	// Makes a color from a hex code, with a format of 0xRRGGBB
	static constexpr Color rgb(uint32 hex)
	{
		return Color((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF, 255);
	}

	// Makes a color from a hex code, with a format of 0xRRGGBBAA
	static constexpr Color rgba(uint32 hex)
	{
		return Color((hex >> 24) & 0xFF, (hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
	}

	// OpenGL uses floats for colors
	constexpr Vec4<float32> to_vec4()
	{
		return {this->r / 255.f, this->g / 255.f, this->b / 255.f, this->a / 255.f};
	}

	// TODO a bunch of operators so lerp works
};

namespace palette {
	// White lmao.
	static constexpr Color WHITE = Color::rgb(0xFFFFFF);
	// Black lmao.
	static constexpr Color BLACK = Color::rgb(0x000000);
	// Where did it go
	static constexpr Color TRANSPARENT = Color::rgba(0x00000000);

	// TODO more palettes, e.g. tr::palette::WebSafe, tr::palette::Material, tr::palette::Starry etc
}

// π
static constexpr float64 PI = 3.141592653589793238463;

// Degrees to radians
template<typename T> inline constexpr T deg2rad(T deg) {
	return deg * (static_cast<T>(PI) / static_cast<T>(180.0));
}

// Radians to degrees
template<typename T> inline constexpr T rad2deg(T rad) {
	return rad * (static_cast<T>(180.0) / static_cast<T>(PI));
}

// clamp
template<typename T> inline constexpr T clamp(T val, T min, T max)
{
	if (val < min) return min;
	else if (val > max) return max;
	else return val;
}

// Picks the smaller option
template<typename T> inline constexpr T min(T a, T b)
{
	return a < b ? a : b;
}

// Picks the bigger option
template<typename T> inline constexpr T max(T a, T b)
{
	return a > b ? a : b;
}

// lerp
template<typename T> inline constexpr T lerp(T a, T b, float64 t)
{
	return (1.0 - t) * a + t * b;
}

// a couple disasters

// lerp
template<typename T> inline constexpr Vec2<T> lerp(Vec2<T> a, Vec2<T> b, float64 t) {
	auto man = Vec2<float64>(1.0 - t, 1.0 - t) * Vec2<float64>(a.x, a.y) + Vec2<float64>(t, t) * Vec2<float64>(b.x, b.y);
	return Vec2<T>(static_cast<T>(man.x), static_cast<T>(man.y));
}
// lerp
template<typename T> inline constexpr Vec3<T> lerp(Vec3<T> a, Vec3<T> b, float64 t) {
	auto man = Vec3<float64>(1.0 - t, 1.0 - t, 1.0 - t) * Vec3<float64>(a.x, a.y, a.z) + Vec3<float64>(t, t, t) * Vec3<float64>(b.x, b.y, b.z);
	return Vec3<T>(static_cast<T>(man.x), static_cast<T>(man.y), static_cast<T>(man.z));
}
// lerp
template<typename T> inline constexpr Vec4<T> lerp(Vec4<T> a, Vec4<T> b, float64 t) {
	auto man = Vec4<float64>(1.0 - t, 1.0 - t, 1.0 - t, 1.0 - t) * Vec4<float64>(a.x, a.y, a.z, a.w) + Vec4<float64>(t, t, t, t) * Vec4<float64>(b.x, b.y, b.z, b.w);
	return Vec4<T>(static_cast<T>(man.x), static_cast<T>(man.y), static_cast<T>(man.z), static_cast<T>(man.w));
}

// Similar to `tr::lerp`, but in reverse.
template<typename T> inline constexpr T inverse_lerp(T a, T b, T v)
{
	return (v - a) / (b - a);
}

// Converts a number from one scale to another
template<typename T> inline constexpr T remap(T val, T src_min, T src_max, T dst_min, T dst_max)
{
	return tr::lerp(dst_min, dst_max, tr::inverse_lerp(src_min, src_max, val));
}

// 🤓. This entire struct is stolen from linmath.h btw lmao
struct Matrix4x4
{
	Vec4<float32> values[4];

	Vec4<float32>& operator[](usize idx);

	// Note this initializes the matrix as 0, not identity (use `Matrix4x4::identity()` for that)
	constexpr Matrix4x4()
	{
		for (usize i = 0; i < 4; i++) {
			for (usize j = 0; j < 4; j++) {
				(*this)[i][j] = 0;
			}
		}
	}

	// Initializes an identity matrix.
	static constexpr Matrix4x4 identity()
	{
		Matrix4x4 m;
		for (usize i = 0; i < 4; i++) {
			for (usize j = 0; j < 4; j++) {
				m[i][j] = i == j ? 1 : 0;
			}
		}
		return m;
	}

	// man
	static Matrix4x4 translate(float32 x, float32 y, float32 z);
	static Matrix4x4 from_vec3_mul_outer(Vec3<float32> a, Vec3<float32> b);
	static Matrix4x4 frustum(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far);
	static Matrix4x4 orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far);
	static Matrix4x4 perspective(float32 fovy, float32 aspect, float32 near, float32 far);

	Vec4<float32> row(usize idx);
	Vec4<float32> column(usize idx);
	Matrix4x4 transpose();
	Matrix4x4 operator+(Matrix4x4 b);
	Matrix4x4 operator-(Matrix4x4 b);
	Matrix4x4 operator*(float32 b);
	Matrix4x4 operator*(Matrix4x4 b);
	Vec4<float32> operator*(Vec4<float32> b);
	Matrix4x4 scale(Vec3<float32> b);
	Matrix4x4 rotate(float32 x, float32 y, float32 z, float32 radians);
	Matrix4x4 rotate_x(float32 radians);
	Matrix4x4 rotate_y(float32 radians);
	Matrix4x4 rotate_z(float32 radians);
	Matrix4x4 invert();
};

/*
 * MEMORY
*/

// Why the fuck not.
struct MemoryInfo {
	// Currently allocated by arenas, in bytes
	isize allocated = 0;
	// Like `allocated`, but cumulative
	isize cumulative_allocated = 0;
	// Amount of alive reference counted objects
	isize ref_counted_objs = 0;
	// Like `ref_counted_objs`, but cumulative
	isize cumulative_ref_counted_objs = 0;
	// Total amount of memory freed from arenas, in bytes
	isize freed_by_arenas = 0;
	// Amount of freed reference counted objects
	isize freed_ref_counted_objs = 0;
	// Total amount of alive arena pages
	isize alive_pages = 0;
	// All arena pages that have ever existed
	isize cumulative_pages = 0;
	// Total amount of freed arena pages
	isize freed_pages = 0;
};

// As the name implies, it gets the memory info. Idk why.
MemoryInfo get_memory_info();

// Implements reference counting through inheritance. Note you have to wrap your values in a `tr::Ref<T>`
// so it's not esoteric to use.
class RefCounted
{
	mutable isize count = 0;

public:
	RefCounted() : count(0) {}
	virtual ~RefCounted() {}

	void retain() const;
	void release() const;
};

// Non-esoteric wrapper around `tr::RefCounted`. It also allows null, if you don't want that to happen, use
// `tr::Ref<T>`
template<typename T>
class MaybeRef;

// Non-esoteric wrapper around `tr::RefCounted`. It also panics on null, if you don't want that to happen, use
// `tr::MaybeRef<T>`
template<typename T>
class Ref
{
	T* ptr;
	// man
	friend class MaybeRef<T>;

	RefCounted* refcounted() const { return dynamic_cast<RefCounted*>(ptr); }

public:
	// C++ can be annoying
	Ref() : ptr(nullptr) {}

	Ref(T* ptr) : ptr(ptr)
	{
		if (this->ptr == nullptr) {
			tr::panic("tr::Ref<T> can't be null, if that's intentional use tr::MaybeRef<T>");
		}
		this->refcounted()->retain();
	}

	Ref(const Ref& ref) : ptr(ref.ptr)
	{
		if (this->ptr == nullptr) {
			tr::panic("tr::Ref<T> can't be null, if that's intentional use tr::MaybeRef<T>");
		}
		this->refcounted()->retain();
	}

	Ref(const MaybeRef<T>& ref);

    Ref(Ref&& other) : ptr(other.ptr) { other.ptr = nullptr; }

	~Ref()
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->release();
	}

	Ref& operator=(T* ptr)
	{
		if (ptr == nullptr) {
			tr::panic("tr::Ref<T> can't be null, if that's intentional use tr::MaybeRef<T>");
		}
		this->refcounted()->retain();
		if (this->ptr != nullptr) this->refcounted()->release();
		this->ptr = ptr;
		return *this;
	}

	Ref& operator=(const Ref<T>& other)
	{
		if (this != &other) {
			other.refcounted()->retain();
			if (this->ptr != nullptr) {
				this->refcounted()->release();
			}
			this->ptr = other.ptr;
		}
		return *this;
	}

	Ref& operator=(Ref<T>&& other)
	{
		if (this != &other) {
			if (this->ptr != nullptr) {
				this->refcounted()->release();
			}
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}

	// Returns the crap pointer.
	T* get() const
	{
		if (this->ptr == nullptr) {
			tr::panic("tr::Ref<T> is null, if that's intentional use tr::MaybeRef<T>");
		}
		return this->ptr;
	}

	// help
	T* operator->() const                   { return this->ptr; }
	T& operator*() const                    { return *this->ptr; }
	operator T*() const                     { return this->ptr; }
	bool operator==(const MaybeRef<T>& ref) { return this->ptr == ref.ptr; }
	bool operator==(const Ref<T>& ref)      { return this->ptr == ref.ptr; }
	bool operator==(const T* ptr)           { return this->ptr == ptr; }
	bool operator!=(const MaybeRef<T>& ref) { return this->ptr != ref.ptr; }
	bool operator!=(const Ref<T>& ref)      { return this->ptr != ref.ptr; }
	bool operator!=(const T* ptr)           { return this->ptr != ptr; }
};

// Non-esoteric wrapper around `tr::RefCounted`. It also allows null, if you don't want that to happen, use
// `tr::Ref<T>`
template<typename T>
class MaybeRef
{
	T* ptr;
	// man
	friend class Ref<T>;

	RefCounted* refcounted() const { return dynamic_cast<RefCounted*>(ptr); }

public:
	MaybeRef(T* ptr = nullptr) : ptr(ptr)
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->retain();
	}

	MaybeRef(const MaybeRef& ref) : ptr(ref.ptr)
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->retain();
	}

	MaybeRef(const Ref<T>& ref) : ptr(ref.ptr)
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->retain();
	}

	MaybeRef(MaybeRef&& other) : ptr(other.ptr) { other.ptr = nullptr; }

	~MaybeRef()
	{
		if (this->ptr == nullptr) return;
		this->refcounted()->release();
	}

	MaybeRef& operator=(T* ptr)
	{
		if (ptr != nullptr) {
			this->refcounted()->retain();
		}
		if (this->ptr != nullptr) {
			this->refcounted()->release();
		}
		this->ptr = ptr;
		return *this;
	}

	MaybeRef& operator=(const MaybeRef<T>& other)
	{
		if (this != &other) {
			if (other != nullptr)     other.refcounted()->retain();
			if (this->ptr != nullptr) this->refcounted()->release();
			this->ptr = other.ptr;
		}
		return *this;
	}

	MaybeRef& operator=(MaybeRef<T>&& other)
	{
		if (this != &other) {
			if (this->ptr != nullptr) {
				this->refcounted()->release();
			}
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}

	// Returns the crap pointer.
	T* get() const
	{
		return this->ptr;
	}

	// help
	T* operator->() const                   { return this->ptr; }
	T& operator*() const                    { return *this->ptr; }
	operator T*() const                     { return this->ptr; }
	bool operator==(const MaybeRef<T>& ref) { return this->ptr == ref.ptr; }
	bool operator==(const Ref<T>& ref)      { return this->ptr == ref.ptr; }
	bool operator==(const T* ptr)           { return this->ptr == ptr; }
	bool operator!=(const MaybeRef<T>& ref) { return this->ptr != ref.ptr; }
	bool operator!=(const Ref<T>& ref)      { return this->ptr != ref.ptr; }
	bool operator!=(const T* ptr)           { return this->ptr != ptr; }
};

// man
template<typename T> Ref<T>::Ref(const MaybeRef<T>& other) : ptr(other.ptr)
{
    if (!this->ptr) {
        tr::panic("can't convert null tr::MaybeRef<T> to tr::Ref<T>");
	}
    this->refcounted()->retain();
}

// Converts kilobytes to bytes
static constexpr usize kb_to_bytes(usize x) { return x * 1024; }
// Converts megabytes to bytes
static constexpr usize mb_to_bytes(usize x) { return kb_to_bytes(x) * 1024; }
// Converts gigabytes to bytes
static constexpr usize gb_to_bytes(usize x) { return mb_to_bytes(x) * 1024; }

// Converts bytes to kilobytes
static constexpr usize bytes_to_kb(usize x) { return x / 1024; }
// Converts bytes to megabytes
static constexpr usize bytes_to_mb(usize x) { return bytes_to_kb(x) / 1024; }
// Converts bytes to gigabytes
static constexpr usize bytes_to_gb(usize x) { return bytes_to_mb(x) / 1024; }

// Arenas are made of many buffers.
class ArenaPage
{
public:
	usize size = 0;
	usize alloc_pos = 0;
	ArenaPage* prev = nullptr;
	ArenaPage* next = nullptr;
	void* buffer = nullptr;

	explicit ArenaPage(usize size);
	~ArenaPage();

	// Returns how much space left the page has
	usize available_space();
};

// Life changing allocator.
class Arena : public RefCounted
{
public:
	usize page_size = 0;
	ArenaPage* page = nullptr;

	// This is just for the compiler to shut up
	Arena() : page_size(0), page(nullptr) {}

	// Initializes the arena. `page_size` is the base size for the buffers, you can have more buffers or
	// bigger buffers.
	explicit Arena(usize page_size);

	// Frees the arena. Note this doesn't call any destructors from structs you may have allocated, as I
	// don't know how to do that.
	~Arena();

	// Allocates some crap on the arena.
	void* alloc(usize size);

	// Literally just `Arena::alloc()` but for structs and crap.
	template<typename T> T* alloc()
	{
		return reinterpret_cast<T*>(this->alloc(sizeof(T)));
	}

	// Makes sure there's enough space to fit `size`. Useful for when you're about to allocate a lot of
	// objects and don't want it to try to figure out the pages 57399593895 times.
	void prealloc(usize size);
};

// This is just for iterators
template<typename T>
struct ArrayItem
{
	usize i;
	T& val;
};

// A slice of memory, usually from an arena but can point to anywhere. Similar to a Go slice, or other
// examples.
template<typename T>
class Array
{
	MaybeRef<Arena> arena;
	T* ptr;
	usize len;
	usize cap;

public:
	// Initializes an empty array at an arena.
	explicit Array(Ref<Arena> arena, usize len) : arena(arena), len(len), cap(len)
	{
		// you may initialize with a length of 0 so you can then add crap
		if (len > 0) {
			this->ptr = reinterpret_cast<T*>(arena->alloc(sizeof(T) * len));
		}
	}

	// Initializes an array from a buffer. (the data is copied into the arena)
	explicit Array(Ref<Arena> arena, T* data, usize len) : arena(arena), len(len), cap(len)
	{
		this->ptr = reinterpret_cast<T*>(arena->alloc(sizeof(T) * len));
		memcpy(this->ptr, data, len * sizeof(T));
	}

	// Initializes an array that points to any buffer. You really should only use this for temporary arrays.
	explicit Array(T* data, usize len) : arena(nullptr), ptr(data), len(len), cap(len) {}

	// man fuck you
	Array() : arena(nullptr), ptr(nullptr), len(0), cap(0) {}

	T& operator[](usize idx) const
	{
		if (idx >= this->len) {
			tr::panic("index out of range: %zu in an array of %zu", idx, this->len);
		}
		return this->ptr[idx];
	}

	// Returns the buffer.
	T* buffer() const      { return this->ptr; }
	// Returns the length of the array.
	usize length() const   { return this->len; }
	// Returns how many items the array can hold before having to resize.
	usize capacity() const { return this->cap; }

	// fucking iterator
	class Iterator {
	public:
		Iterator(T* ptr, usize index) : idx(index), ptr(ptr) {}
		ArrayItem<T> operator*() const { return {this->idx, *this->ptr}; }
		Iterator& operator++() { this->ptr++; this->idx++; return *this; }
		bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
	private:
		usize idx;
		T* ptr;
	};

	Iterator begin() const { return Iterator(this->buffer(), 0); }
	Iterator end()   const { return Iterator(this->buffer() + this->len, this->len); }

	// Adds a new item to the array, and resizes it if necessary. This only works on arena-allocated arrays,
	// if you try to use this on an array without an arena, it will panic.
	void add(T val)
	{
		if (this->arena == nullptr) {
			tr::panic("resizing arena-less tr::Array<T> is not allowed");
		}

		// does it already fit?
		if (this->len + 1 <= this->cap) {
			(*this)[this->len++] = val;
			return;
		}

		// reallocate array
		// TODO use pages to not waste so much memory
		T* old_buffer = this->ptr;
		this->cap *= 2;
		this->ptr = reinterpret_cast<T*>(this->arena->alloc(this->cap * sizeof(T)));
		// you may initialize with a length of 0 so you can then add crap
		if (this->len > 0) {
			memcpy(this->ptr, old_buffer, this->len * sizeof(T));
		}

		(*this)[this->len++] = val;
	}

	// As the name implies, it copies the array and its items to somewhere else.
	Array<T> duplicate(Ref<Arena> arena) const
	{
		Array<T> result(arena, this->length());
		memcpy(result.buffer(), this->buffer(), this->length() * sizeof(T));
		return result;
	}
};

// Literally just a wrapper around `tr::Array`, so it works better with strings. Also all of the functions
// that return strings copy the original strings first.
class String {
	Array<char> array;

public:
	// Initializes a string from an arena and C string.
	explicit String(Ref<Arena> arena, const char* str, usize len)
	{
		this->array = Array<char>(arena, const_cast<char*>(str), len + 1);
	}

	// Initializes an empty string from an arena
	explicit String(Ref<Arena> arena, usize len)
	{
		this->array = Array<char>(arena, len + 1);
	}

	// Initializes a string from any C string. You really should only use this for temporary arrays.
	explicit String(const char* str, usize len)
	{
		this->array = Array<char>(const_cast<char*>(str), len + 1);
	}

	// Initializes a string from any C string. You really should only use this for temporary arrays.
	String(const char* str) : String(str, strlen(str)) {}

	// man
	char& operator[](usize idx) const { return this->array[idx]; }
	usize length() const { return this->array.length() - 1; }
	char* buffer() const { return this->array.buffer(); }
	operator char*() const { return this->buffer(); }
	operator const char*() const { return this->buffer(); }
	Array<char>::Iterator begin() const { return this->array.begin(); }
	// this one is different since you don't want to iterate over the null terminator
	Array<char>::Iterator end() const { return Array<char>::Iterator(const_cast<char*>(this->buffer()) + this->length() - 1, this->length() - 1); }
	String duplicate(Ref<Arena> arena) const
	{
		Array<char> arrayma = this->array.duplicate(arena);
		return String(arrayma.buffer(), arrayma.length() + 1);
	}
	// i know .add() is missing

	// special string crap
	bool operator==(const String& other);
	bool operator!=(const String& other) { return !(*this == other); }
	bool operator==(const char* other)   { return *this == String(other); }
	bool operator!=(const char* other)   { return *this != String(other); }

	// Gets a substring. The returned string doesn't include the end character.
	String substr(Ref<Arena> arena, usize start, usize end);

	// Returns an array with all of the indexes
	Array<usize> find(Ref<Arena> arena, String str, usize start = 0, usize end = 0);

	// It concatenates 2 strings lmao.
	String concat(Ref<Arena> arena, String other);

	// If true, the string starts with that other crap.
	bool starts_with(String str);

	// If true, the string ends with that other crap.
	bool ends_with(String str);

	// Gets the filename in a path, e.g. returns `file.txt` for `/path/to/file.txt`
	String file(Ref<Arena> arena);

	// Gets the directory in a path e.g. returns `/path/to` for `/path/to/file.txt`
	String directory(Ref<Arena> arena);

	// Returns the extension in a path, e.g. returns `.txt` for `/path/to/file.txt`, `.blend.1` for
	// `teapot.blend.1`, and an empty string for `.gitignore`
	String extension(Ref<Arena> arena);

	// If true, the path is absolute. Else, it's relative.
	bool is_absolute();
};

// It's just `sprintf` for `tr::String` lmao.
TR_LOG_FUNC(3, 4) String sprintf(Ref<Arena> arena, usize maxlen, const char* fmt, ...);

}

#endif
