/*
 * libtrippin: Most massive library of all time
 * https://github.com/hellory4n/libtrippin
 *
 * trippin/math.hpp
 * Vectors, matrices, RNG, and other math-y functions.
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

#ifndef _TRIPPIN_MATH_H
#define _TRIPPIN_MATH_H

#include <math.h>
#include <time.h>

#include "common.hpp"
#include "log.hpp"

namespace tr {

// TODO more constexpr
// we need to constexpr all over the place

// Vec2 lmao
template<typename T>
struct Vec2
{
	T x;
	T y;

	constexpr Vec2() : x(0), y(0) {}
	// shut up :)
	TR_GCC_IGNORE_WARNING(-Wshadow)
	constexpr Vec2(T x, T y) : x(x), y(y) {}
	TR_GCC_RESTORE()
	explicit constexpr Vec2(T v) : x(v), y(v) {}

	// TODO these could be implemented with evil macro fuckery but idk if i want to do that

	constexpr Vec2 operator+(Vec2 r)  { return Vec2(this->x + r.x, this->y + r.y); }
	constexpr Vec2 operator-(Vec2 r)  { return Vec2(this->x - r.x, this->y - r.y); }
	constexpr Vec2 operator*(Vec2 r)  { return Vec2(this->x * r.x, this->y * r.y); }
	constexpr Vec2 operator*(T r)     { return Vec2(this->x * r,   this->y * r);   }
	constexpr Vec2 operator/(Vec2 r)  { return Vec2(this->x / r.x, this->y / r.y); }
	constexpr Vec2 operator/(T r)     { return Vec2(this->x / r,   this->y / r);   }
	constexpr Vec2 operator%(Vec2 r)  { return Vec2(this->x % r.x, this->y % r.y); }
	constexpr Vec2 operator%(T r)     { return Vec2(this->x % r,   this->y % r);   }

	constexpr bool operator==(Vec2 r) { return this->x == r.x && this->y == r.y;   }
	constexpr bool operator!=(Vec2 r) { return this->x != r.x && this->y != r.y;   }
	constexpr bool operator>(Vec2 r)  { return this->x >  r.x && this->y >  r.y;   }
	constexpr bool operator>=(Vec2 r) { return this->x >= r.x && this->y >= r.y;   }
	constexpr bool operator<(Vec2 r)  { return this->x <  r.x && this->y <  r.y;   }
	constexpr bool operator<=(Vec2 r) { return this->x <= r.x && this->y <= r.y;   }

	constexpr Vec2& operator+=(Vec2 other) { return *this = *this + other; }
	constexpr Vec2& operator-=(Vec2 other) { return *this = *this - other; }
	constexpr Vec2& operator*=(Vec2 other) { return *this = *this * other; }
	constexpr Vec2& operator/=(Vec2 other) { return *this = *this / other; }
	constexpr Vec2& operator%=(Vec2 other) { return *this = *this % other; }
	constexpr Vec2& operator*=(T other) { return *this = *this * other; }
	constexpr Vec2& operator/=(T other) { return *this = *this / other; }
	constexpr Vec2& operator%=(T other) { return *this = *this % other; }
	constexpr Vec2 operator-() { return {-this->x, -this->y}; }

	static constexpr usize ITEMS = 2;
	constexpr T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return this->x; break;
			case 1:  return this->y; break;
			default: tr::panic("sir this is a vec2<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	constexpr T mul_inner(Vec2<T> b)
	{
		T p = 0;
		for (usize i = 0; i < ITEMS; i++) {
			p += b[i] * (*this)[i];
		}
		return p;
	}

	constexpr float64 length() { return sqrt(this->mul_inner(*this)); }
	constexpr Vec2 normalize() { return *this * (1 / this->length()); }

	// swizzling operators
	// i know this is insane
	// i didnt write this myself im not insane
	constexpr Vec2<T> xx() const { return {this->x, this->x}; }
	constexpr Vec2<T> xy() const { return {this->x, this->y}; }
	constexpr Vec2<T> yx() const { return {this->y, this->x}; }
	constexpr Vec2<T> yy() const { return {this->y, this->y}; }
};

// c i hate you
template<> constexpr Vec2<float64> Vec2<float64>::operator%(Vec2<float64> r) {
	return Vec2<float64>(fmod(this->x, r.x), fmod(this->y, r.y));
}
template<> constexpr Vec2<float64> Vec2<float64>::operator%(float64 r) {
	return Vec2<float64>(fmod(this->x, r), fmod(this->y, r));
}
template<> constexpr Vec2<float32> Vec2<float32>::operator%(Vec2<float32> r) {
	return Vec2<float32>(fmod(this->x, r.x), fmod(this->y, r.y));
}
template<> constexpr Vec2<float32> Vec2<float32>::operator%(float32 r) {
	return Vec2<float32>(fmod(this->x, r), fmod(this->y, r));
}

// Vec3 lmao
template<typename T>
struct Vec3
{
	T x;
	T y;
	T z;

	constexpr Vec3() : x(0), y(0), z(0) {}
	// shut up
	TR_GCC_IGNORE_WARNING(-Wshadow)
	constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
	TR_GCC_RESTORE();
	explicit constexpr Vec3(T v) : x(v), y(v), z(v) {}

	constexpr Vec3 operator+(Vec3 r)  { return Vec3(this->x + r.x, this->y + r.y, this->z + r.z);  }
	constexpr Vec3 operator-(Vec3 r)  { return Vec3(this->x - r.x, this->y - r.y, this->z - r.z);  }
	constexpr Vec3 operator*(Vec3 r)  { return Vec3(this->x * r.x, this->y * r.y, this->z * r.z);  }
	constexpr Vec3 operator*(T r)     { return Vec3(this->x * r,   this->y * r,   this->z * r);    }
	constexpr Vec3 operator/(Vec3 r)  { return Vec3(this->x / r.x, this->y / r.y, this->z / r.z);  }
	constexpr Vec3 operator/(T r)     { return Vec3(this->x / r,   this->y / r,   this->z / r);    }
	constexpr Vec3 operator%(Vec3 r)  { return Vec3(this->x % r.x, this->y % r.y, this->z % r.z);  }
	constexpr Vec3 operator%(T r)     { return Vec3(this->x % r,   this->y % r,   this->z % r);    }

	constexpr bool operator==(Vec3 r) { return this->x == r.x && this->y == r.y && this->z == r.z; }
	constexpr bool operator!=(Vec3 r) { return this->x != r.x && this->y != r.y && this->z != r.z; }
	constexpr bool operator>(Vec3 r)  { return this->x >  r.x && this->y >  r.y && this->z >  r.z; }
	constexpr bool operator>=(Vec3 r) { return this->x >= r.x && this->y >= r.y && this->z >= r.z; }
	constexpr bool operator<(Vec3 r)  { return this->x <  r.x && this->y <  r.y && this->z <  r.z; }
	constexpr bool operator<=(Vec3 r) { return this->x <= r.x && this->y <= r.y && this->z <= r.z; }

	constexpr Vec3& operator+=(Vec3 other) { return *this = *this + other; }
	constexpr Vec3& operator-=(Vec3 other) { return *this = *this - other; }
	constexpr Vec3& operator*=(Vec3 other) { return *this = *this * other; }
	constexpr Vec3& operator/=(Vec3 other) { return *this = *this / other; }
	constexpr Vec3& operator%=(Vec3 other) { return *this = *this % other; }
	constexpr Vec3& operator*=(T other) { return *this = *this * other; }
	constexpr Vec3& operator/=(T other) { return *this = *this / other; }
	constexpr Vec3& operator%=(T other) { return *this = *this % other; }
	constexpr Vec3 operator-() { return {-this->x, -this->y, -this->z}; }

	static constexpr usize ITEMS = 3;
	constexpr T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return this->x; break;
			case 1:  return this->y; break;
			case 2:  return this->z; break;
			default: tr::panic("sir this is a vec3<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	constexpr T mul_inner(Vec3<T> b)
	{
		T p = 0;
		for (usize i = 0; i < ITEMS; i++) {
			p += b[i] * (*this)[i];
		}
		return p;
	}

	constexpr float64 length() { return sqrt(this->mul_inner(*this)); }
	constexpr Vec3<T> normalize() { return *this * (1 / this->length()); }

	constexpr Vec3<T> reflect(Vec3<T> b)
	{
		Vec3<T> r;
		T p = 2 * this->mul_inner(b);
		for (usize i = 0; i < ITEMS; i++) {
			r[i] = (*this)[i] - p * b[i];
		}
		return r;
	}

	constexpr Vec3<T> mul_cross(Vec3<T> b)
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
	TR_GCC_IGNORE_WARNING(-Wshadow)
	constexpr Vec3(T x, Vec2<T> other) : Vec3(x, other.y, other.z) {}
	constexpr Vec3(Vec2<T> other, T z) : Vec3(other.x, other.y, z) {}
	TR_GCC_RESTORE()
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
template<> constexpr Vec3<float64> Vec3<float64>::operator%(Vec3<float64> r) {
	return Vec3<float64>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z));
}
template<> constexpr Vec3<float64> Vec3<float64>::operator%(float64 r) {
	return Vec3<float64>(fmod(this->x, r), fmod(this->y, r), fmod(this->z, r));
}
template<> constexpr Vec3<float32> Vec3<float32>::operator%(Vec3<float32> r) {
	return Vec3<float32>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z));
}
template<> constexpr Vec3<float32> Vec3<float32>::operator%(float32 r) {
	return Vec3<float32>(fmod(this->x, r), fmod(this->y, r), fmod(this->z, r));
}

// COLOR I HARDLY KNOW 'ER HAHAHHAHA LAUGH IMMEIDATLEY
struct Color;

// Vec4 lmao
template<typename T>
struct Vec4
{
	T x;
	T y;
	T z;
	T w;

	constexpr Vec4() : x(0), y(0), z(0), w(0) {}
	// shut up :)
	TR_GCC_IGNORE_WARNING(-Wshadow);
	constexpr Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
	TR_GCC_RESTORE();
	explicit constexpr Vec4(T v) : x(v), y(v), z(v), w(v) {}
	constexpr Vec4(Color c);

	constexpr Vec4 operator+(Vec4 r)  { return Vec4(this->x + r.x, this->y + r.y, this->z + r.z, this->w + r.w);  }
	constexpr Vec4 operator-(Vec4 r)  { return Vec4(this->x - r.x, this->y - r.y, this->z - r.z, this->w - r.w);  }
	constexpr Vec4 operator*(Vec4 r)  { return Vec4(this->x * r.x, this->y * r.y, this->z * r.z, this->w * r.w);  }
	constexpr Vec4 operator*(T r)     { return Vec4(this->x * r,   this->y * r,   this->z * r,   this->w * r);    }
	constexpr Vec4 operator/(Vec4 r)  { return Vec4(this->x / r.x, this->y / r.y, this->z / r.z, this->w / r.z);  }
	constexpr Vec4 operator/(T r)     { return Vec4(this->x / r,   this->y / r,   this->z / r,   this->w / r);    }
	constexpr Vec4 operator%(Vec4 r)  { return Vec4(this->x % r.x, this->y % r.y, this->z % r.z, this->w % r.z);  }
	constexpr Vec4 operator%(T r)     { return Vec4(this->x % r,   this->y % r,   this->z % r,   this->w % r);    }

	constexpr bool operator==(Vec4 r) { return this->x == r.x && this->y == r.y && this->z == r.z && this->w == r.w; }
	constexpr bool operator!=(Vec4 r) { return this->x != r.x && this->y != r.y && this->z != r.z && this->w != r.w; }
	constexpr bool operator>(Vec4 r)  { return this->x >  r.x && this->y >  r.y && this->z >  r.z && this->w >  r.w; }
	constexpr bool operator>=(Vec4 r) { return this->x >= r.x && this->y >= r.y && this->z >= r.z && this->w >= r.w; }
	constexpr bool operator<(Vec4 r)  { return this->x <  r.x && this->y <  r.y && this->z <  r.z && this->w <  r.w; }
	constexpr bool operator<=(Vec4 r) { return this->x <= r.x && this->y <= r.y && this->z <= r.z && this->w <= r.w; }

	constexpr Vec4& operator+=(Vec4 other) { return *this = *this + other; }
	constexpr Vec4& operator-=(Vec4 other) { return *this = *this - other; }
	constexpr Vec4& operator*=(Vec4 other) { return *this = *this * other; }
	constexpr Vec4& operator/=(Vec4 other) { return *this = *this / other; }
	constexpr Vec4& operator%=(Vec4 other) { return *this = *this % other; }
	constexpr Vec4& operator*=(T other) { return *this = *this * other; }
	constexpr Vec4& operator/=(T other) { return *this = *this / other; }
	constexpr Vec4& operator%=(T other) { return *this = *this % other; }
	constexpr Vec4 operator-() { return {-this->x, -this->y, -this->z, -this->w}; }

	static constexpr usize ITEMS = 4;
	constexpr T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return this->x; break;
			case 1:  return this->y; break;
			case 2:  return this->z; break;
			case 3:  return this->w; break;
			default: tr::panic("sir this is a vec4<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	constexpr T mul_inner(Vec4<T> b)
	{
		T p = 0;
		for (usize i = 0; i < ITEMS; i++) {
			p += b[i] * (*this)[i];
		}
		return p;
	}

	constexpr float64 length() { return sqrt(this->mul_inner(*this)); }
	constexpr Vec4<T> normalize() { return *this * (1 / this->length()); }

	constexpr Vec4<T> mul_cross(Vec4<T> b)
	{
		Vec4<T> r;
		r[0] = (*this)[1] * b[2] - (*this)[2] * b[1];
		r[1] = (*this)[2] * b[0] - (*this)[0] * b[2];
		r[2] = (*this)[0] * b[1] - (*this)[1] * b[0];
		r[3] = 1.f;
		return r;
	}

	constexpr Vec4<T> reflect(Vec4<T> b)
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
	TR_GCC_IGNORE_WARNING(-Wshadow)
	constexpr Vec4(T x, T y, Vec2<T> other) : Vec4(x, y, other.z, other.w) {}
	constexpr Vec4(Vec2<T> other, T z, T w) : Vec4(other.x, other.y, z, w) {}
	constexpr Vec4(T x, Vec2<T> other, T w) : Vec4(x, other.y, other.z, w) {}
	constexpr Vec4(T x, Vec3<T> other) : Vec4(x, other.y, other.z, other.w) {}
	constexpr Vec4(Vec3<T> other, T w) : Vec4(other.x, other.y, other.z, w) {}
	TR_GCC_RESTORE()
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
template<> constexpr Vec4<float64> Vec4<float64>::operator%(Vec4<float64> r) {
	return Vec4<float64>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z), fmod(this->w, r.w));
}
template<> constexpr Vec4<float64> Vec4<float64>::operator%(float64 r) {
	return Vec4<float64>(fmod(this->x, r), fmod(this->y, r), fmod(this->z, r), fmod(this->w, r));
}
template<> constexpr Vec4<float32> Vec4<float32>::operator%(Vec4<float32> r) {
	return Vec4<float32>(fmod(this->x, r.x), fmod(this->y, r.y), fmod(this->z, r.z), fmod(this->w, r.w));
}
template<> constexpr Vec4<float32> Vec4<float32>::operator%(float32 r) {
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
	template<typename T>
	T next(T min, T max)
	{
		return static_cast<T>((this->next() * max) + min);
	}
};

// π
static constexpr float64 PI = 3.141592653589793238463;

// Degrees to radians
template<typename T>
static constexpr T deg2rad(T deg) {
	return deg * (static_cast<T>(PI) / static_cast<T>(180.0));
}

// Radians to degrees
template<typename T>
static constexpr T rad2deg(T rad) {
	return rad * (static_cast<T>(180.0) / static_cast<T>(PI));
}

// Picks the smaller option
template<typename T>
static constexpr T min(T a, T b)
{
	return a < b ? a : b;
}

// Picks the bigger option
template<typename T>
static constexpr T max(T a, T b)
{
	return a > b ? a : b;
}

// clamp
template<typename T>
static constexpr T clamp(T val, T min, T max)
{
	return tr::min(tr::max(min, val), max);
}

// lerp
template<typename T>
static constexpr T lerp(T a, T b, float64 t)
{
	return (1.0 - t) * a + t * b;
}

// a couple disasters

// lerp
template<typename T>
static constexpr Vec2<T> lerp(Vec2<T> a, Vec2<T> b, float64 t) {
	auto man = Vec2<float64>(1.0 - t, 1.0 - t) * Vec2<float64>(a.x, a.y) + Vec2<float64>(t, t) * Vec2<float64>(b.x, b.y);
	return Vec2<T>(static_cast<T>(man.x), static_cast<T>(man.y));
}
// lerp
template<typename T>
constexpr Vec3<T> lerp(Vec3<T> a, Vec3<T> b, float64 t) {
	auto man = Vec3<float64>(1.0 - t, 1.0 - t, 1.0 - t) * Vec3<float64>(a.x, a.y, a.z) + Vec3<float64>(t, t, t) * Vec3<float64>(b.x, b.y, b.z);
	return Vec3<T>(static_cast<T>(man.x), static_cast<T>(man.y), static_cast<T>(man.z));
}
// lerp
template<typename T>
constexpr Vec4<T> lerp(Vec4<T> a, Vec4<T> b, float64 t) {
	auto man = Vec4<float64>(1.0 - t, 1.0 - t, 1.0 - t, 1.0 - t) * Vec4<float64>(a.x, a.y, a.z, a.w) + Vec4<float64>(t, t, t, t) * Vec4<float64>(b.x, b.y, b.z, b.w);
	return Vec4<T>(static_cast<T>(man.x), static_cast<T>(man.y), static_cast<T>(man.z), static_cast<T>(man.w));
}

// Similar to `tr::lerp`, but in reverse.
template<typename T>
constexpr T inverse_lerp(T a, T b, T v)
{
	return (v - a) / (b - a);
}

// Converts a number from one scale to another
template<typename T>
constexpr T remap(T val, T src_min, T src_max, T dst_min, T dst_max)
{
	return tr::lerp(dst_min, dst_max, tr::inverse_lerp(src_min, src_max, val));
}

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
	// shut up :)
	TR_GCC_IGNORE_WARNING(-Wshadow)
	constexpr Color(uint8 r, uint8 g, uint8 b) : r(r), g(g), b(b), a(255) {}
	constexpr Color(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) {}
	TR_GCC_RESTORE()
	constexpr Color(Vec4<float32> vec) :
		r(tr::clamp(vec.x, 0.f, 1.f) * 255), g(tr::clamp(vec.y, 0.f, 1.f) * 255),
		b(tr::clamp(vec.z, 0.f, 1.f) * 255), a(tr::clamp(vec.w, 0.f, 1.f) * 255) {}

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

	// im sorry... im sorry... im sorry...
	constexpr Color operator+(Color c) { return {static_cast<uint8>(clamp(r+c.r, 0, 255)), static_cast<uint8>(clamp(g+c.g, 0, 255)), static_cast<uint8>(clamp(b+c.b, 0, 255)), static_cast<uint8>(clamp(a+c.a, 0, 255))}; }
	constexpr Color operator-(Color c) { return {static_cast<uint8>(clamp(r-c.r, 0, 255)), static_cast<uint8>(clamp(g-c.g, 0, 255)), static_cast<uint8>(clamp(b-c.b, 0, 255)), static_cast<uint8>(clamp(a-c.a, 0, 255))}; }
	constexpr Color operator*(Color c) { return {static_cast<uint8>(clamp(r*c.r, 0, 255)), static_cast<uint8>(clamp(g*c.g, 0, 255)), static_cast<uint8>(clamp(b*c.b, 0, 255)), static_cast<uint8>(clamp(a*c.a, 0, 255))}; }
	constexpr Color operator*(uint8 c) { return {static_cast<uint8>(clamp(r*c, 0, 255)), static_cast<uint8>(clamp(g*c, 0, 255)), static_cast<uint8>(clamp(b*c, 0, 255)), static_cast<uint8>(clamp(a*c, 0, 255))}; }
	constexpr Color operator/(Color c) { return {static_cast<uint8>(clamp(r/c.r, 0, 255)), static_cast<uint8>(clamp(g/c.g, 0, 255)), static_cast<uint8>(clamp(b/c.b, 0, 255)), static_cast<uint8>(clamp(a/c.a, 0, 255))}; }
	constexpr Color operator/(uint8 c) { return {static_cast<uint8>(clamp(r/c, 0, 255)), static_cast<uint8>(clamp(g/c, 0, 255)), static_cast<uint8>(clamp(b/c, 0, 255)), static_cast<uint8>(clamp(a/c, 0, 255))}; }
	constexpr Color operator%(Color c) { return {static_cast<uint8>(clamp(r%c.r, 0, 255)), static_cast<uint8>(clamp(g%c.g, 0, 255)), static_cast<uint8>(clamp(b%c.b, 0, 255)), static_cast<uint8>(clamp(a%c.a, 0, 255))}; }
	constexpr Color operator%(uint8 c) { return {static_cast<uint8>(clamp(r%c, 0, 255)), static_cast<uint8>(clamp(g%c, 0, 255)), static_cast<uint8>(clamp(b%c, 0, 255)), static_cast<uint8>(clamp(a%c, 0, 255))}; }

	constexpr bool operator==(Color c) { return r == c.r && g == c.g && b == c.b && a == c.a; }
	constexpr bool operator!=(Color c) { return !(*this == c); }
	constexpr bool operator>(Color c)  { return r >  c.r && g >  c.g && b >  c.b && a >  c.a; }
	constexpr bool operator>=(Color c) { return r >= c.r && g >= c.g && b >= c.b && a >= c.a; }
	constexpr bool operator<(Color c)  { return r <  c.r && g <  c.g && b <  c.b && a <  c.a; }
	constexpr bool operator<=(Color c) { return r <= c.r && g <= c.g && b <= c.b && a <= c.a; }

	constexpr Color& operator+=(Color other) { return *this = *this + other; }
	constexpr Color& operator-=(Color other) { return *this = *this - other; }
	constexpr Color& operator*=(Color other) { return *this = *this * other; }
	constexpr Color& operator/=(Color other) { return *this = *this / other; }
	constexpr Color& operator%=(Color other) { return *this = *this % other; }
	constexpr Color& operator*=(uint8 other) { return *this = *this * other; }
	constexpr Color& operator/=(uint8 other) { return *this = *this / other; }
	constexpr Color& operator%=(uint8 other) { return *this = *this % other; }
};

template<typename T>
constexpr Vec4<T>::Vec4(Color c) {
	this->x = static_cast<T>(c.r / 255.0);
	this->y = static_cast<T>(c.g / 255.0);
	this->z = static_cast<T>(c.b / 255.0);
	this->w = static_cast<T>(c.a / 255.0);
}

namespace palette {
	// White lmao.
	constexpr Color WHITE = Color::rgb(0xffffff);
	// Black lmao.
	static constexpr Color BLACK = Color::rgb(0x000000);
	// Where did it go
	static constexpr Color TRANSPARENT = Color::rgba(0x00000000);

	// TODO more palettes, e.g. tr::palette::WebSafe, tr::palette::Material, tr::palette::Starry etc
}

// Matrix intended for use with OpenGL. This entire struct is stolen from linmath.h btw lmao
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

}

#endif
