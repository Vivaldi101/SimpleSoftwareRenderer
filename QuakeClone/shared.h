#ifndef SHARED_H
#define SHARED_H

#include <cstdint>
#include <stdio.h>		// FIXME: remove at some point
#include <string.h>		// FIXME: remove at some point
#include <math.h>		// FIXME: remove at some point


// typedefs for all ints
typedef int8_t	i8;
typedef int16_t	i16;
typedef int32_t	i32;
typedef int64_t	i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef unsigned char byte;

// 32 bit bool for word alignment
typedef u32 b32;

// typedefs for single and double floating points
typedef float	r32;
typedef double	r64;


// angle indexes
enum {
	PITCH = 0,		// up / down
	YAW,			// left / right
	ROLL			// fall over
};

enum {
	PLANE_X	= 0,
	PLANE_Y,
	PLANE_Z,
	PLANE_NON_AXIAL	
};


enum {
	UVN_U = 0, 
	UVN_V,
	UVN_N
};

// angle conversions
#if 1
#define DEG2RAD(a) (((a) * M_PI) / 180.0F)
#define RAD2DEG(a) (((a) * 180.0f) / M_PI)
#else
// angle conversions
static inline r32 DEG2RAD(r32 a) {
	return (a * M_PI) / 180.0f;
}
static inline r32 RAD2DEG(r32 a) {
	return (a * 180.0f) / M_PI;
} 
#endif

// macros for sizes
#define KILOBYTES(Value) ((Value)*1024ULL)
#define MEGABYTES(Value) ((KILOBYTES(Value)*1024ULL))
#define GIGABYTES(Value) ((MEGABYTES(Value)*1024ULL))
#define TERABYTES(Value) ((GIGABYTES(Value)*1024ULL))

// rgb
#define RGB_32(A,R,G,B) (((A & 255) << 24) + ((R & 255) << 16) + ((G & 255) << 8) + ((B) & 255))
#define RGB_16_565(R,G,B) ((((R) & 31) << 11) + (((G) & 63) << 5) + ((B) & 31))

inline u16 RGB_888To565(int r, int g, int b) {
    // builds a 5.6.5 format 16 bit pixel
    // assumes input is in 8.8.8 format

    r >>= 3; 
	g >>= 2; 
	b >>= 3;

    return RGB_16_565(r, g, b);
} 

// alignment
#define ALIGNUP(Address, Alignment) \
    (((size_t)Address) + ((Alignment)-1) & (~((Alignment)-1)))

#define MISALIGNAMOUNT(Address, Alignment) \
    ((((size_t)Alignment)-1) & Address)

#define ALIGNDOWN(Address, Alignment) \
    (((Address - ((((size_t)Alignment)-1) & Address))))


// nothing outside the Cvar_*() functions should modify these fields!
//struct Cvar {
//	char *		name;
//	char *		string;
//	char *		reset_string;		// cvar_restart will reset to this value
//	char *		latched_string;		// for CVAR_LATCH vars
//	int			flags;
//	b32			modified;			// set each time the cvar is changed
//	int			modification_count;	// incremented each time the cvar is changed
//	float		value;				// atof( string )
//	int			integer;			// atoi( string )
//	Cvar *		next;
//	Cvar *		hash_next;
//};

// max values
#define	MAX_CVAR_VALUE_STRING	256
#define	MAX_ENTITIES			1023		// can't be increased without changing drawsurf bit packing
// these are sort of arbitrary limits.
// the limits apply to the sum of all scenes in a frame --
// the main view, all the 3D icons, etc
#define	MAX_POLYS		600
#define	MAX_POLYVERTS	3000

/*
==============================================================

MATHLIB

==============================================================
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#define Vector3Init(v, x, y, z)		((v)[0] = (x), (v)[1] = (y), (v)[2] = (z))
#define Vector4Init(v, x, y, z, w)	((v)[0] = (x), (v)[1] = (y), (v)[2] = (z), (v)[3] = (w))
#define Vector3Zero(v)				((v)[0] = (0), (v)[1] = (0), (v)[2] = (0))
#define Vector4Zero(v)				((v)[0] = (0), (v)[1] = (0), (v)[2] = (0), (v)[3] = (1))

#define Vector3DotProduct(x, y)		((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
#define Vector3Subtract(a, b, c)	((c)[0] = (a)[0] - (b)[0] , (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define Vector3Add(a, b, c)			((c)[0] = (a)[0] + (b)[0] , (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define Vector3Copy(a, b)			((a)[0] = (b)[0] , (a)[1] = (b)[1] , (a)[2] = (b)[2])
#define Vector4Copy(a, b)			((a)[0] = (b)[0] , (a)[1] = (b)[1] , (a)[2] = (b)[2], (a)[3] = (b)[3])
#define	Vector3Scale(v, s, o)		((o)[0] = (v)[0] * (s), (o)[1] = (v)[1] * (s), (o)[2] = (v)[2] * (s))
#define	Vector3MA(v, s, b, o)		((o)[0] = (v)[0] + (b)[0] * (s), (o)[1] = (v)[1] + (b)[1] * (s), (o)[2] = (v)[2] + (b)[2] * (s))

#define Matrix3x3SetIdentity(m)		(Vector3Init((m)[0],1,0,0), Vector3Init((m)[1],0,1,0), Vector3Init((m)[2],0,0,1))
#define Matrix4x4SetIdentity(m)		(Vector4Init((m)[0],1,0,0,0), Vector4Init((m)[1],0,1,0,0), Vector4Init((m)[2],0,0,1,0), Vector4Init((m)[3],0,0,0,1))

#define Matrix3x3Init(m)			(Vector3Init((m)[0],0,0,0), Vector3Init((m)[1],0,0,0), Vector3Init((m)[2],0,0,0))
#define Matrix4x4Init(m)			(Vector4Init((m)[0],0,0,0,0), Vector4Init((m)[1],0,0,0,0), Vector4Init((m)[2],0,0,0,0), Vector4Init((m)[3],0,0,0,0))

#define Perp(v, x, y)				{r32 t = (v)[(x)]; (v)[(x)] = -(v)[(y)], (v)[(y)] = t;}		// x and y define the plane of v
#define Square(s)					((s) * (s))

union Vec3 {
	struct {	r32 x, y, z;		} v;
	r32 data[3];

	r32			&operator[](int i)			{ return data[i]; }
	const r32	&operator[](int i) const	{ return data[i]; }
};
union Vec4 {
	struct {	r32 x, y, z, w;		} v;
	r32 data[4];

	r32			&operator[](int i)			{ return data[i]; }
	const r32	&operator[](int i) const	{ return data[i]; }
};

//inline Vec3 operator <(Vec3 a, Vec3 b) {
//}

inline Vec3 operator +(Vec3 a, Vec3 b) {
	Vec3 v = {};

	v[0] = a[0] + b[0];
	v[1] = a[1] + b[1];
	v[2] = a[2] + b[2];

	return v;
}

inline Vec3 operator -(Vec3 a) {
	Vec3 v = {};

	v[0] = -a[0]; 
	v[1] = -a[1];
	v[2] = -a[2];

	return v;
}

inline Vec3 operator -(Vec3 a, Vec3 b) {
	Vec3 v = {};

	v[0] = a[0] - b[0];
	v[1] = a[1] - b[1];
	v[2] = a[2] - b[2];

	return v;
}

inline Vec3 operator *(Vec3 a, r32 s) {
	Vec3 v = {};

	v[0] = a[0] * s;
	v[1] = a[1] * s;
	v[2] = a[2] * s;

	return v;
}

static inline Vec3 Vector3CrossProduct(Vec3 v1, Vec3 v2) {
	Vec3 cross = {};

	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];

	return cross;
}

static inline r32 Vector3Len(const Vec3 *v) {
	r32	len;

	len = Vector3DotProduct(*v, *v);
	len = sqrt(len);
		
	return len;
}

static inline Vec3 Vector3Normalize(const Vec3 *v) {
	Vec3 n = {};
	r32	len, ilen;

	len = Vector3DotProduct(*v, *v);
	len = sqrt(len);

	if (len) {
		ilen = 1 / len;
		n[0] = (*v)[0] * ilen;
		n[1] = (*v)[1] * ilen;
		n[2] = (*v)[2] * ilen;
	}
		
	return n;
}

static inline Vec3 Vector3Normalize(Vec3 v) {
	Vec3 n = {};
	r32	len, ilen;

	len = Vector3DotProduct(v, v);
	len = sqrt(len);

	if (len) {
		ilen = 1 / len;
		n[0] = v[0] * ilen;
		n[1] = v[1] * ilen;
		n[2] = v[2] * ilen;
	}
		
	return n;
}

static inline Vec3 Vector3Build(Vec3 p0, Vec3 p1) {
	Vec3 v = {};

	v[0] = p1[0] - p0[0];
	v[1] = p1[1] - p0[1];
	v[2] = p1[2] - p0[2];

	return v;
}

extern void Mat1x3Mul(Vec3 *out, const Vec3 *a, const r32 b[3][3]);
extern void Mat1x4Mul(r32 out[4], const r32 a[4], const r32 b[4][4]);
extern void Mat2x2Mul(r32 out[2][2], const r32 a[2][2], const r32 b[2][2]);
extern void Mat3x3Mul(r32 out[3][3], const r32 a[3][3], const r32 b[3][3]);
extern void Mat4x4Mul(r32 out[4][4], const r32 a[4][4], const r32 b[4][4]);

struct Orientation {
	Vec3	origin;			// in world coordinates
	Vec3	axis[3];		// orientation in world, uvn
	Vec3	dir;			// look at vector in uvn system, or euler angles
};

struct Plane {
	Vec3	unit_normal;
	r32		dist;
	byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	byte	pad[2];			// for 4 byte alignment
};

/*
==============================================================

GENERAL

==============================================================
*/

// stack based allocator

struct MemoryStack {
	byte *	base_ptr;
	size_t	max_size;
	size_t	bytes_used;
};

// NOTE: dont use the _Push_ and _Pop_ functions directly, go through the macros
extern void *_Push_(MemoryStack *ms, size_t num_bytes);
extern void _Pop_(MemoryStack *ms, size_t num_bytes);

#define PushSize(arena, type, size) ((type *)_Push_(arena, size))  
#define PushStruct(arena, type) ((type *)_Push_(arena, sizeof(type)))  
#define PopStruct(arena, type) (_Pop_(arena, sizeof(type)))  

#define PushArray(arena, count, type) ((type *)_Push_(arena, (count) * sizeof(type)))
#define PopArray(arena, count, type) (_Pop_(arena, (count) * sizeof(type)))  

// util tools
#define ArrayCount(arr) ((sizeof(arr)) / (sizeof(*(arr))))
#define Swap(a, b) do { if (a != b) {a ^= b; b ^= a; a ^= b;} } while(0)
#define AnySwap(a, b, type) { do { type temp = a; a = b; b = temp; } while(0); }
#define TypelessSwap(a, b, type)
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while(0)

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))


// windows specific
#ifdef _WIN32
#include <Windows.h>
#define InvalidCodePath do { MessageBoxA(0, "Invalid code path", 0, 0); Assert(0); } while(0)
#else
#define InvalidCodePath do { Assert(0); } while(0)
#endif	// _WIN32

#endif	// Header guard