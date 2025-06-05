/*
 * libtrippin v2.0.0
 *
 * Most biggest most massive standard library thing for C of all time
 * More information at https://github.com/hellory4n/libtrippin
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
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <ctime>

#if (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
// counting starts at 1 lmao
#define TR_LOG_FUNC(fmt_idx, varargs_idx) [[gnu::format(printf, fmt_idx, varargs_idx)]]
#else
#define TR_LOG_FUNC(fmt_idx, varargs_idx)
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

namespace tr {

// I sure love versions.
static constexpr const char* VERSION = "v2.0.0";

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
	constexpr const char* RESET    = "\033[0m";
	constexpr const char* LIB_INFO = "\033[0;90m";
	constexpr const char* WARN     = "\033[0;93m";
	constexpr const char* ERROR    = "\033[0;91m";
	#else
	constexpr const char* RESET    = "";
	constexpr const char* LIB_INFO = "";
	constexpr const char* WARN     = "";
	constexpr const char* ERROR    = "";
	#endif
}

// Sets the log file to somewhere.
void use_log_file(const char* path);

// Log.
TR_LOG_FUNC(1, 2) void log(const char* fmt, ...);

// Log. (gray edition) (this is for libraries that use libtrippin so you can filter out library logs)
TR_LOG_FUNC(1, 2) void liblog(const char* fmt, ...);

// Oh nose.
TR_LOG_FUNC(1, 2) void warn(const char* fmt, ...);

// Oh god oh fuck. Note this doesn't crash and die everything, `tr::panic` does.
TR_LOG_FUNC(1, 2) void error(const char* fmt, ...);

// Oh god oh fuck. Note this crashes and kills everything, `tr::error` doesn't.
TR_LOG_FUNC(1, 2) [[noreturn]] void panic(const char* fmt, ...);

// Formatted assert?????????
TR_LOG_FUNC(2, 3) void assert(bool x, const char* fmt, ...);

/*
 * UTILITIES
 */

// Like how the spicy modern languages handle null
template<typename T>
struct Maybe
{
private:
	bool has_value;
	union {
		uint8_t waste_of_space;
		T value;
	};

public:
	// Initializes a Maybe<T> as null
	Maybe() : has_value(false), waste_of_space(0) {};

	// Intializes a Maybe<T> with a value
	Maybe(T val) : has_value(true), value(val) {};

	// If true, the maybe is, in fact, a definitely.
	bool is_valid()
	{
		return this->has_value;
	}

	// Gets the value or panics if it's null
	T& unwrap()
	{
		if (this->has_value) {
			return this->value;
		}
		else {
			tr::panic("couldn't unwrap Maybe<T>");
		}
	}

	// i love c++
	Maybe& operator=(T val)
	{
		if (val == nullptr) {
			this->has_value = false;
			this->value = nullptr;
		}
		else {
			this->has_value = true;
			this->value = val;
		}
		return *this;
	}
};

// Functional propaganda
template<typename L, typename R>
struct Either
{
private:
	// False is left, true is right
	bool active;
	union {
		L val_left;
		R val_right;
	};

public:
	Either(L left) : active(false), val_left(left) {};
	Either(R right) : active(true), val_right(right) {};

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
	bool is_left() { return !this->active; }
	// If true, it's right. Else, it's left.
	bool is_right() { return this->active; }

	// Returns the left value, or panics if it's not left
	L& left()
	{
		if (this->active) tr::panic("Either<L, R> is right, not left");
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

	static constexpr usize LENGTH = 2;
	T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return &this->x; break;
			case 1:  return &this->y; break;
			default: tr::panic("sir this is a vec2<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	T mul_inner(Vec2<T> b)
	{
		T p = 0;
		for (usize i = 0; i < LENGTH; i++) {
			p += b[i] * this[i];
		}
		return p;
	}

	float64 len() { return sqrt(this->mul_inner(this)); }
	Vec2<T> normalize() { return this * (1 / this->len()); }
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

	static constexpr usize LENGTH = 3;
	T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return &this->x; break;
			case 1:  return &this->y; break;
			case 2:  return &this->z; break;
			default: tr::panic("sir this is a vec3<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	T mul_inner(Vec3<T> b)
	{
		T p = 0;
		for (usize i = 0; i < LENGTH; i++) {
			p += b[i] * this[i];
		}
		return p;
	}

	float64 len() { return sqrt(this->mul_inner(this)); }
	Vec3<T> normalize() { return this * (1 / this->len()); }

	Vec3<T> reflect(Vec3<T> b)
	{
		Vec3<T> r;
		T p = 2 * this->mul_inner(b);
		for (usize i = 0; i < LENGTH; i++) {
			r[i] = this[i] - p * b[i];
		}
		return r;
	}
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

	static constexpr usize LENGTH = 4;
	T& operator[](usize idx)
	{
		switch (idx) {
			case 0:  return &this->x; break;
			case 1:  return &this->y; break;
			case 2:  return &this->z; break;
			case 3:  return &this->w; break;
			default: tr::panic("sir this is a vec4<T> not a vec%zu<T>", idx + 1); break;
		}
	}

	T mul_inner(Vec4<T> b)
	{
		T p = 0;
		for (usize i = 0; i < LENGTH; i++) {
			p += b[i] * this[i];
		}
		return p;
	}

	float64 len() { return sqrt(this->mul_inner(this)); }
	Vec4<T> normalize() { return this * (1 / this->len()); }

	Vec4<T> mul_cross(Vec4<T> b)
	{
		Vec4<T> r;
		r[0] = this[1] * b[2] - this[2] * b[1];
		r[1] = this[2] * b[0] - this[0] * b[2];
		r[2] = this[0] * b[1] - this[1] * b[0];
		r[3] = 1.f;
		return r;
	}

	Vec4<T> reflect(Vec4<T> b)
	{
		Vec4<T> r;
		T p = 2 * this->mul_inner(b);
		for (usize i = 0; i < LENGTH; i++) {
			r[i] = this[i] - p * b[i];
		}
		return r;
	}
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
	uint64_t state[4];

public:
	// Initializes the `tr::Random` with a seed
	Random(int64 seed);

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
template<typename T>
struct Matrix4x4
{
	Vec4<T> values[4];

	T& operator[](usize idx) { return this->values[idx]; }

	// Initializes an identity matrix
	static Matrix4x4<T> identity()
	{
		Matrix4x4<T> m;
		for (usize i = 0; i < 4; i++) {
			for (usize j = 0; j < 4; j++) {
				m[i][j] = i == j ? 1 : 0;
			}
		}
		return m;
	}

	Vec4<T> row(usize idx)
	{
		Vec4<T> r;
		for (usize k = 0; k < 4; k++) {
			r[k] = this[k][idx];
		}
		return r;
	}

	Vec4<T> column(usize idx)
	{
		Vec4<T> r;
		for (usize k = 0; k < 4; k++) {
			r[k] = this[idx][k];
		}
		return r;
	}

	Matrix4x4<T> transpose()
	{
		Matrix4x4<T> m;
		for (usize j = 0; j < 4; j++) {
			for (usize i = 0; i < 4; i++) {
				m[i][j] = this[j][i];
			}
		}
		return m;
	}

	Matrix4x4<T> operator+(Matrix4x4<T> b)
	{
		Matrix4x4<T> m;
		for (usize i = 0; i < 4; i++) {
			m[i] = this[i] + b[i];
		}
		return m;
	}

	Matrix4x4<T> operator-(Matrix4x4<T> b)
	{
		Matrix4x4<T> m;
		for (usize i = 0; i < 4; i++) {
			m[i] = this[i] - b[i];
		}
		return m;
	}

	Matrix4x4<T> operator*(T b)
	{
		Matrix4x4<T> m;
		for (usize i = 0; i < 4; i++) {
			m[i] = this[i] * b;
		}
		return m;
	}

	Matrix4x4<T> operator*(Vec3<T> b)
	{
		Matrix4x4<T> m;
		m[0] = this[0] * b.x;
		m[1] = this[1] * b.y;
		m[2] = this[2] * b.z;
		m[3] = this[3];
		return m;
	}

	Matrix4x4<T> operator*(Matrix4x4<T> b)
	{
		Matrix4x4<T> m, temp;
		for (usize c = 0; c < 4; c++) {
			for (usize r = 0; r < 4; r++) {
				temp[c][r] = 0;
				for (usize k = 0; k < 4; k++) {
					temp[c][r] += this[k][r] * b[c][k];
				}
			}
		}
		return m;
	}

	Matrix4x4<T> operator*(Vec4<T> b)
	{
		Vec4<T> r;
		for (usize j = 0; j < 4; j++) {
			r[j] = 0;
			for (usize i = 0; i < 4; i++) {
				r[j] += this[i][j] * b[i];
			}
		}
		return r;
	}

	Matrix4x4<T> translate(T x, T y, T z)
	{
		Matrix4x4<T> m = Matrix4x4<T>::identity();
		m[3][0] = x;
		m[3][1] = y;
		m[3][2] = z;
		return m;
	}

	Matrix4x4<T> translate_in_place(T x, T y, T z)
	{
		Matrix4x4<T> m = Matrix4x4<T>::identity();
		Vec4<T> t = Vec4<T>(x, y, z, 0);
		Vec4<T> r;
		for (usize i = 0; i < 4; i++) {
			Vec4<T> r = m.row(i);
			m[3][i] += r.mul_inner(t);
		}
		return m;
	}

	static Matrix4x4<T> from_vec3_mul_outer(Vec3<T> a, Vec3<T> b)
	{
		Matrix4x4<T> m;
		for(usize i = 0; i < 4; i++) {
			for(usize j = 0; j < 4; j++) {
				m[i][j] = i < 3 && j < 3 ? a[i] * b[j] : 0;
			}
		}
		return m;
	}

	Matrix4x4<T> rotate(T x, T y, T z, float64 radians)
	{
		float64 s = sin(radians);
		float64 c = cos(radians);
		Vec3<T> u = Vec3<T>(x, y, z);
		Matrix4x4<T> R;

		if (u.len() > 1e-4) {
			u = u.normalize();
			Matrix4x4<T> t = Matrix4x4<T>::from_vec3_mul_outer(u, u);

			Matrix4x4<T> S = {
				{    0,  u[2], -u[1], 0},
				{-u[2],     0,  u[0], 0},
				{ u[1], -u[0],     0, 0},
				{    0,     0,     0, 0}
			};
			S *= s;

			Matrix4x4<T> C = Matrix4x4<T>::identity();
			C -= t;
			C *= c;

			t += C;
			t += S;

			t[3][3] = 1;
			R = this * t;
		} else {
			R = this;
		}
		return R;
	}

	Matrix4x4<T> rotate_x(float64 radians)
	{
		float64 s = sin(radians);
		float64 c = cos(radians);
		Matrix4x4<T> R = {
			{1, 0, 0, 0},
			{0, c, s, 0},
			{0, -s, c, 0},
			{0, 0, 0, 1}
		};
		return this * R;
	}

	Matrix4x4<T> rotate_y(float64 radians)
	{
		float64 s = sin(radians);
		float64 c = cos(radians);
		Matrix4x4<T> R = {
			{c, 0, -s, 0},
			{0, 1, 0, 0},
			{s, 0, c, 0},
			{0, 0, 0, 1}
		};
		return this * R;
	}

	Matrix4x4<T> rotate_z(float64 radians)
	{
		float64 s = sin(radians);
		float64 c = cos(radians);
		Matrix4x4<T> R = {
			{c, s, 0, 0},
			{-s, c, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		};
		return this * R;
	}

	Matrix4x4<T> invert()
	{
		Matrix4x4<T> m;
		T s[6];
		T c[6];
		s[0] = this[0][0]*this[1][1] - this[1][0]*this[0][1];
		s[1] = this[0][0]*this[1][2] - this[1][0]*this[0][2];
		s[2] = this[0][0]*this[1][3] - this[1][0]*this[0][3];
		s[3] = this[0][1]*this[1][2] - this[1][1]*this[0][2];
		s[4] = this[0][1]*this[1][3] - this[1][1]*this[0][3];
		s[5] = this[0][2]*this[1][3] - this[1][2]*this[0][3];

		c[0] = this[2][0]*this[3][1] - this[3][0]*this[2][1];
		c[1] = this[2][0]*this[3][2] - this[3][0]*this[2][2];
		c[2] = this[2][0]*this[3][3] - this[3][0]*this[2][3];
		c[3] = this[2][1]*this[3][2] - this[3][1]*this[2][2];
		c[4] = this[2][1]*this[3][3] - this[3][1]*this[2][3];
		c[5] = this[2][2]*this[3][3] - this[3][2]*this[2][3];

		float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

		m[0][0] = ( this[1][1] * c[5] - this[1][2] * c[4] + this[1][3] * c[3]) * idet;
		m[0][1] = (-this[0][1] * c[5] + this[0][2] * c[4] - this[0][3] * c[3]) * idet;
		m[0][2] = ( this[3][1] * s[5] - this[3][2] * s[4] + this[3][3] * s[3]) * idet;
		m[0][3] = (-this[2][1] * s[5] + this[2][2] * s[4] - this[2][3] * s[3]) * idet;

		m[1][0] = (-this[1][0] * c[5] + this[1][2] * c[2] - this[1][3] * c[1]) * idet;
		m[1][1] = ( this[0][0] * c[5] - this[0][2] * c[2] + this[0][3] * c[1]) * idet;
		m[1][2] = (-this[3][0] * s[5] + this[3][2] * s[2] - this[3][3] * s[1]) * idet;
		m[1][3] = ( this[2][0] * s[5] - this[2][2] * s[2] + this[2][3] * s[1]) * idet;

		m[2][0] = ( this[1][0] * c[4] - this[1][1] * c[2] + this[1][3] * c[0]) * idet;
		m[2][1] = (-this[0][0] * c[4] + this[0][1] * c[2] - this[0][3] * c[0]) * idet;
		m[2][2] = ( this[3][0] * s[4] - this[3][1] * s[2] + this[3][3] * s[0]) * idet;
		m[2][3] = (-this[2][0] * s[4] + this[2][1] * s[2] - this[2][3] * s[0]) * idet;

		m[3][0] = (-this[1][0] * c[3] + this[1][1] * c[1] - this[1][2] * c[0]) * idet;
		m[3][1] = ( this[0][0] * c[3] - this[0][1] * c[1] + this[0][2] * c[0]) * idet;
		m[3][2] = (-this[3][0] * s[3] + this[3][1] * s[1] - this[3][2] * s[0]) * idet;
		m[3][3] = ( this[2][0] * s[3] - this[2][1] * s[1] + this[2][2] * s[0]) * idet;

		return m;
	}

	Matrix4x4<T> orthonormalize()
	{
		Matrix4x4<T> R = this;
		T s = 1;
		Vec3<T> h;

		R[2] = Vec4<T>(Vec3<T>(R[2].x, R[2].y, R[2].z).normalize(), R[2].w);

		s = Vec3<T>(R[1].x, R[1].y, R[1].z).mul_inner(Vec3<T>(R[2].x, R[2].y, R[3]));
		h = Vec3<T>(R[2].x, R[2].y, R[2].z) * s;
		R[1] = Vec4<T>(Vec3<T>(R[1].x, R[1].y, R[1].z) - h, R[1].w);
		R[1] = Vec4<T>(Vec3<T>(R[1].x, R[1].y, R[1].z).normalize(), R[1].w);

		s = Vec3<T>(R[0].x, R[0].y, R[0].z).mul_inner(Vec3<T>(R[2].x, R[2].y, R[2].z));
		h = R[2] * s;
		R[0] = Vec3<T>(R[0].x, R[0].y, R[0].z) - h;

		s = Vec3<T>(R[0].x, R[0].y, R[0].z).mul_inner(Vec3<T>(R[1].x, R[1].y, R[1].z));
		h = Vec3<T>(R[1].x, R[1].y, R[1].z) * s;
		R[0] = Vec4<T>(Vec3<T>(R[0].x, R[0].y, R[0].z) - h, R[0].w);
		R[0] = Vec4<T>(Vec3<T>(R[0].x, R[0].y, R[0].z).normalize(), R[0].w);

		return R;
	}

	static Matrix4x4<T> frustum(T left, T right, T bottom, T top, T near, T far)
	{
		Matrix4x4<T> m;
		m[0][0] = 2.f*near/(right-left);
		m[0][1] = m[0][2] = m[0][3] = 0.f;

		m[1][1] = 2.f*near/(top-bottom);
		m[1][0] = m[1][2] = m[1][3] = 0.f;

		m[2][0] = (right+left)/(right-left);
		m[2][1] = (top+bottom)/(top-bottom);
		m[2][2] = -(far+near)/(far-near);
		m[2][3] = -1.f;

		m[3][2] = -2.f*(far*near)/(far-near);
		m[3][0] = m[3][1] = m[3][3] = 0.f;
		return m;
	}

	static Matrix4x4<T> ortho(T left, T right, T bottom, T top, T near, T far)
	{
		Matrix4x4<T> m;
		m[0][0] = 2.f/(right-left);
		m[0][1] = m[0][2] = m[0][3] = 0.f;

		m[1][1] = 2.f/(top-bottom);
		m[1][0] = m[1][2] = m[1][3] = 0.f;

		m[2][2] = -2.f/(far-near);
		m[2][0] = m[2][1] = m[2][3] = 0.f;

		m[3][0] = -(right+left)/(right-left);
		m[3][1] = -(top+bottom)/(top-bottom);
		m[3][2] = -(far+near)/(far-near);
		m[3][3] = 1.f;
		return m;
	}

	static Matrix4x4<T> perspective(float64 fovy, float64 aspect, float64 near, float64 far)
	{
		Matrix4x4<T> m;
		const float64 a = 1.f / tanf(fovy / 2.f);

		m[0][0] = a / aspect;
		m[0][1] = 0.f;
		m[0][2] = 0.f;
		m[0][3] = 0.f;

		m[1][0] = 0.f;
		m[1][1] = a;
		m[1][2] = 0.f;
		m[1][3] = 0.f;

		m[2][0] = 0.f;
		m[2][1] = 0.f;
		m[2][2] = -((far + near) / (far - near));
		m[2][3] = -1.f;

		m[3][0] = 0.f;
		m[3][1] = 0.f;
		m[3][2] = -((2.f * far * near) / (far - near));
		m[3][3] = 0.f;
	}
};

// Converts kilobytes to bytes
inline constexpr usize kb_to_bytes(usize x) { return x * 1024; }
// Converts megabytes to bytes
inline constexpr usize mb_to_bytes(usize x) { return kb_to_bytes(x) * 1024; }
// Converts gigabytes to bytes
inline constexpr usize gb_to_bytes(usize x) { return mb_to_bytes(x) * 1024; }

// Arenas are made of many buffers.
struct ArenaPage
{
	usize size;
	usize alloc_pos;
	Maybe<ArenaPage*> prev;
	Maybe<ArenaPage*> next;
	void* buffer;

	ArenaPage(usize size);
	~ArenaPage();

	// Returns how much space left the page has
	usize available_space();

	// TODO resize(usize new_size); (it's for vectors and hashmaps and whatever the fuck)
};

// Life changing allocator.
struct Arena
{
	usize page_size;
	ArenaPage* page;

	// Initializes the arena. `page_size` is the base size for the buffers, you can have more buffers or
	// bigger buffers.
	Arena(usize page_size);

	// Frees the arena. Note this doesn't call any destructors from structs you may have allocated, as I
	// don't know how to do that.
	~Arena();

	// Allocates some crap on the arena.
	void* alloc(usize size);

	// Literally just `Arena::alloc()` but for structs and crap.
	template<typename T> T* alloc()
	{
		return (T*)this->alloc(sizeof(T));
	}

	// Makes sure there's enough space to fit `size`. Useful for when you're about to allocate a lot of
	// objects and don't want it to try to figure out the pages 57399593895 times.
	void prealloc(usize size);
};

}

#endif
