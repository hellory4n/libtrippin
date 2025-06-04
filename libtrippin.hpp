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
constexpr const char* VERSION = "v2.0.0";

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
	T* unwrap()
	{
		if (this->has_value) {
			return &this->value;
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
	L* left()
	{
		if (this->active) tr::panic("Either<L, R> is right, not left");
		else return &this->val_left;
	}

	// Returns the right value, or panics if it's not right
	R* right()
	{
		if (this->active) tr::panic("Either<L, R> is right, not left");
		else return &this->val_right;
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

	Vec2(T x, T y) : x(x), y(y) {};

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

	Vec3(T x, T y, T z) : x(x), y(y), z(z) {};

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

	Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {};

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

// Converts kilobytes to bytes
static constexpr usize kb_to_bytes(usize x) { return x * 1024; }
// Converts megabytes to bytes
static constexpr usize mb_to_bytes(usize x) { return kb_to_bytes(x) * 1024; }
// Converts gigabytes to bytes
static constexpr usize gb_to_bytes(usize x) { return mb_to_bytes(x) * 1024; }

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

}

#endif
