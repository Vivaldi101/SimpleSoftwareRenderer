#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include <stdio.h>		// FIXME: remove at some point
#include <string.h>		// FIXME: remove at some point
#include <math.h>		// FIXME: remove at some point

// typedefs for all ints
typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;
typedef int64_t		s64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef unsigned char byte;

// 32 bit bool for dword alignment
typedef u32 b32;

// typedefs for single and double floating points
typedef float	r32;
typedef double	r64;

enum {
	PITCH,		// up / down
	YAW,		// left / right
	ROLL		// fall over
};

enum {
	PLANE_X,
	PLANE_Y,
	PLANE_Z,
	PLANE_NON_AXIAL	
};

enum {
	UVN_U, 
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
#define RGB_32(A,R,G,B) (((A & 255u) << 24u) + ((R & 255u) << 16u) + ((G & 255u) << 8u) + ((B) & 255u))
#define RGB_16_565(R,G,B) ((((R) & 31u) << 11u) + (((G) & 63u) << 5u) + ((B) & 31u))

static inline u16 RGB_888To565(int r, int g, int b) {
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


// FIXME: fine tune these
// min/max values
#define	MAX_NUM_ENTITIES	(1 << 4)
#define	MAX_NUM_POLYS		(1 << 10)
#define	MAX_NUM_POLY_VERTS	(1 << 12)
#define	MAX_NUM_LIGHTS		8
#define	MAX_RENDER_BUFFER	MEGABYTES(4)
#define	MAX_NUM_GLYPHS		(26 * 2)

#undef SINT32_MIN
#undef UINT32_MIN
#define SINT32_MIN	(-0x7fffffff - 1)
#undef SINT32_MAX
#undef UINT32_MAX
#define SINT32_MAX 0x7fffffff
#define UINT32_IMAX 0xffffffff

// util tools
#define ArrayCount(arr) ((sizeof(arr)) / (sizeof(*(arr))))
#define IntSwap(a, b) do { if (a != b) {a ^= b; b ^= a; a ^= b;} } while(0)
#define AnySwap(a, b, type) { do { type temp = a; a = b; b = temp; } while(0); }
//#define Vec3Swap(a, b) Vec3 temp = a, a = b, b = temp
#define OffsetOf(i, s) ((int)&(((s *)0)->i))
#define OffsetOfSize(i, s) sizeof((s *)0)->i
#define PointerSizeOf(v) ((sizeof(v) + sizeof(void *) - 1) & ~(sizeof(void *) - 1))
#define VaStart(va, v) ((va) = (char *)&(v) + PointerSizeOf(v))
#define VaArg(va, t) ( *(t *)(((va) += PointerSizeOf(t)) - PointerSizeOf(t)))
#define GetAnonType(value, type, prefix) (((value)->type_enum == prefix##type) ? &(value)->type : 0)

#ifdef PLATFORM_DEBUG
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while(0)
#else
#define Assert(cond)
#endif	// PLATFORM_DEBUG

/*
==============================================================

MATHLIB

==============================================================
*/

#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))

#define MIN3(a,b,c)	(MIN(MIN((a), (b)), (c)))
#define MAX3(a,b,c)	(MAX(MAX((a), (b)), (c)))

#define ABS(a) (((a) >= 0) ? (a) : -(a))

#ifndef M_PI
#define M_PI 3.14159265358979323846f	
#endif

#define Vec3Init(v, x, y, z)		((v)[0] = (x), (v)[1] = (y), (v)[2] = (z))
#define Vec4Init(v, x, y, z, w)		((v)[0] = (x), (v)[1] = (y), (v)[2] = (z), (v)[3] = (w))
#define Vec3ZeroOut(v)				((v)[0] = (0), (v)[1] = (0), (v)[2] = (0))
#define Vec4ZeroOut(v)				((v)[0] = (0), (v)[1] = (0), (v)[2] = (0), (v)[3] = (1))

//#define Dot2(x, y)					((x)[0] * (y)[0] + (x)[1] * (y)[1])
//#define Dot3(x, y)					((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
#define Vec3Copy(a, b)				((a)[0] = (b)[0] , (a)[1] = (b)[1] , (a)[2] = (b)[2])
#define Vec4Copy(a, b)				((a)[0] = (b)[0] , (a)[1] = (b)[1] , (a)[2] = (b)[2], (a)[3] = (b)[3])

#define	SnapVec3(v)					((v)[0] = (int)(v)[0], (v)[1] = (int)(v)[1], (v)[2] = (int)(v)[2])

#define Mat3x3SetIdentity(m)		(Vec3Init((m)[0],1,0,0), Vec3Init((m)[1],0,1,0), Vec3Init((m)[2],0,0,1))
#define Mat4x4SetIdentity(m)		(Vec4Init((m)[0],1,0,0,0), Vec4Init((m)[1],0,1,0,0), Vec4Init((m)[2],0,0,1,0), Vec4Init((m)[3],0,0,0,1))

#define Mat3x3Init(m)				(Vec3Init((m)[0],0,0,0), Vec3Init((m)[1],0,0,0), Vec3Init((m)[2],0,0,0))
#define Mat4x4Init(m)				(Vec4Init((m)[0],0,0,0,0), Vec4Init((m)[1],0,0,0,0), Vec4Init((m)[2],0,0,0,0), Vec4Init((m)[3],0,0,0,0))

//#define Perp(v, x, y)				{r32 t = (v)[(x)]; (v)[(x)] = -(v)[(y)], (v)[(y)] = t;}		
#define Square(s)					((s) * (s))

union Vec2 {
	struct {	r32 x, y;		} v;
	r32 data[2];

	r32			&operator[](int i)			{ return data[i]; }
	const r32	&operator[](int i) const	{ return data[i]; }
};

union Vec2i {
	struct {	s32 x, y;		} v;
	s32 data[2];

	s32			&operator[](int i)			{ return data[i]; }
	const s32	&operator[](int i) const	{ return data[i]; }
};


static inline Vec2 MakeVec2(r32 x, r32 y) {
	Vec2 v;

	v[0] = x;
	v[1] = y;

	return v;
}

static inline Vec2 MV2(r32 x, r32 y) {
	Vec2 v;

	v[0] = x;
	v[1] = y;

	return v;
}

static inline Vec2 MakeVec2(Vec2 p0, Vec2 p1) {
	Vec2 v = {};

	v[0] = p1[0] - p0[0];
	v[1] = p1[1] - p0[1];

	return v;
}

static inline Vec2 MV2(Vec2 p0, Vec2 p1) {
	Vec2 v = {};

	v[0] = p1[0] - p0[0];
	v[1] = p1[1] - p0[1];

	return v;
}

static inline Vec2i MakeVec2i(s32 x, s32 y) {
	Vec2i v;

	v[0] = x;
	v[1] = y;

	return v;
}

static inline Vec2i MV2i(s32 x, s32 y) {
	Vec2i v;

	v[0] = x;
	v[1] = y;

	return v;
}

static inline Vec2 operator +(Vec2 a, Vec2 b) {
	Vec2 v = {};

	v[0] = a[0] + b[0];
	v[1] = a[1] + b[1];

	return v;
}

static inline Vec2 operator -(Vec2 a) {
	Vec2 v = {};

	v[0] = -a[0]; 
	v[1] = -a[1];

	return v;
}

static inline Vec2 operator -(Vec2 a, Vec2 b) {
	Vec2 v = {};

	v[0] = a[0] - b[0];
	v[1] = a[1] - b[1];

	return v;
}

static inline Vec2 operator *(Vec2 a, r32 s) {
	Vec2 v = {};

	v[0] = a[0] * s;
	v[1] = a[1] * s;

	return v;
}

static inline Vec2 operator *(r32 s, Vec2 a) {
	Vec2 v = {};

	v[0] = a[0] * s;
	v[1] = a[1] * s;

	return v;
}

static inline r32 Dot2(Vec2 v1, Vec2 v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1];

	return dot;
}

static inline r32 Dot2(r32 *v1, r32 *v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1];

	return dot;
}

static inline r32 Dot2(r32 *v1, Vec2 v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1];

	return dot;
}

static inline r32 Dot2(Vec2 v1, r32 *v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1];

	return dot;
}

static inline Vec2 Vec2Norm(Vec2 v) {
	Vec2 n = {};
	r32	len, ilen;

	len = Dot2(v, v);
	len = sqrt(len);

	if (len) {
		ilen = 1.0f / len;
		n[0] = v[0] * ilen;
		n[1] = v[1] * ilen;
	}
		
	return n;
}

static inline r32 Vec2Len(Vec2 v) {
	r32	len;

	len = Dot2(v, v);
	len = sqrt(len);
		
	return len;
}

static inline Vec2 Perp(Vec2 a) {
	Vec2 v = {};

	v[0] = -a[1];
	v[1] = a[0];

	return v;
}

union Vec3 {
	struct {	r32 x, y, z;		} v;
	struct {	r32 r, g, b;		} c;
	r32 data[3];

	r32			&operator[](int i)			{ return data[i]; }
	const r32	&operator[](int i) const	{ return data[i]; }
};

union Vec3i {
	struct {	s32 x, y, z;		} v;
	struct {	s32 r, g, b;		} c;
	s32 data[3];

	s32			&operator[](int i)			{ return data[i]; }
	const s32	&operator[](int i) const	{ return data[i]; }
};


static inline Vec3 MakeVec3(r32 x, r32 y, r32 z) {
	Vec3 v;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

static inline Vec3 MV3(r32 x, r32 y, r32 z) {
	Vec3 v;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

static inline Vec3 operator +(Vec3 a, Vec3 b) {
	Vec3 v = {};

	v[0] = a[0] + b[0];
	v[1] = a[1] + b[1];
	v[2] = a[2] + b[2];

	return v;
}

static inline Vec3i operator +(Vec3i a, Vec3i b) {
	Vec3i v = {};

	v[0] = a[0] + b[0];
	v[1] = a[1] + b[1];
	v[2] = a[2] + b[2];

	return v;
}

static inline Vec3 operator -(Vec3 a) {
	Vec3 v = {};

	v[0] = -a[0]; 
	v[1] = -a[1];
	v[2] = -a[2];

	return v;
}

static inline Vec3i operator -(Vec3i a) {
	Vec3i v = {};

	v[0] = -a[0]; 
	v[1] = -a[1];
	v[2] = -a[2];

	return v;
}

static inline Vec3 operator -(Vec3 a, Vec3 b) {
	Vec3 v = {};

	v[0] = a[0] - b[0];
	v[1] = a[1] - b[1];
	v[2] = a[2] - b[2];

	return v;
}

static inline Vec3i operator -(Vec3i a, Vec3i b) {
	Vec3i v = {};

	v[0] = a[0] - b[0];
	v[1] = a[1] - b[1];
	v[2] = a[2] - b[2];

	return v;
}

static inline Vec3 operator +(Vec3 a, r32 s) {
	Vec3 v = {};

	v[0] = a[0] + s;
	v[1] = a[1] + s;
	v[2] = a[2] + s;

	return v;
}

static inline Vec3 operator -(Vec3 a, r32 s) {
	Vec3 v = {};

	v[0] = a[0] - s;
	v[1] = a[1] - s;
	v[2] = a[2] - s;

	return v;
}


static inline Vec3 operator *(Vec3 a, r32 s) {
	Vec3 v = {};

	v[0] = a[0] * s;
	v[1] = a[1] * s;
	v[2] = a[2] * s;

	return v;
}

static inline Vec3 operator *(r32 s, Vec3 a) {
	Vec3 v = {};

	v[0] = a[0] * s;
	v[1] = a[1] * s;
	v[2] = a[2] * s;

	return v;
}

static inline r32 Dot3(Vec3 v1, Vec3 v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

	return dot;
}

static inline s32 Dot3i(Vec3i v1, Vec3i v2) {
	s32 dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

	return dot;
}

static inline r32 Dot3(r32 *v1, r32 *v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

	return dot;
}

static inline r32 Dot3(r32 *v1, Vec3 v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

	return dot;
}

static inline r32 Dot3(Vec3 v1, r32 *v2) {
	r32 dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

	return dot;
}

static inline Vec3 Cross3(Vec3 v1, Vec3 v2) {
	Vec3 cross = {};

	cross[0] = (v1[1]*v2[2]) - (v1[2]*v2[1]);
	cross[1] = (v1[2]*v2[0]) - (v1[0]*v2[2]);
	cross[2] = (v1[0]*v2[1]) - (v1[1]*v2[0]);

	return cross;
}

static inline Vec3i Cross3(Vec3i v1, Vec3i v2) {
	Vec3i cross = {};

	cross[0] = (v1[1]*v2[2]) - (v1[2]*v2[1]);
	cross[1] = (v1[2]*v2[0]) - (v1[0]*v2[2]);
	cross[2] = (v1[0]*v2[1]) - (v1[1]*v2[0]);

	return cross;
}

//static inline r32 Vec3Len(const Vec3 *v) {
//	r32	len;
//
//	len = Dot3(*v, *v);
//	len = sqrt(len);
//		
//	return len;
//}


static inline r32 Vec3Len(Vec3 v) {
	r32	len;

	len = Dot3(v, v);
	len = sqrt(len);
		
	return len;
}

static inline s32 Vec3iLen(Vec3i v) {
	s32	len;

	len = Dot3i(v, v);
	len = (s32)sqrt((r32)len);
		
	return len;
}

static inline Vec3 Vec3Norm(Vec3 v) {
	Vec3 n = {};
	r32	len, ilen;

	len = Dot3(v, v);
	len = sqrt(len);

	if (len) {
		ilen = 1.0f / len;
		n[0] = v[0] * ilen;
		n[1] = v[1] * ilen;
		n[2] = v[2] * ilen;
	}
		
	return n;
}

static inline Vec3 MakeVec3(Vec3 p0, Vec3 p1) {
	Vec3 v = {};

	v[0] = p1[0] - p0[0];
	v[1] = p1[1] - p0[1];
	v[2] = p1[2] - p0[2];

	return v;
}

union Vec4 {
	struct {	r32 x, y, z, w;		} v;
	struct {	r32 r, g, b, a;		} c;
	r32 data[4];

	r32			&operator[](int i)			{ return data[i]; }
	const r32	&operator[](int i) const	{ return data[i]; }
};

static inline Vec4 MakeVec4(r32 x, r32 y, r32 z, r32 w) {
	Vec4 v;

	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = w;

	return v;
}

static inline Vec4 MV4(r32 x, r32 y, r32 z, r32 w) {
	Vec4 v;

	v[0] = x;
	v[1] = y;
	v[2] = z;
	v[3] = w;

	return v;
}

static inline Vec4 operator +(Vec4 a, Vec4 b) {
	Vec4 v = {};

	v[0] = a[0] + b[0];
	v[1] = a[1] + b[1];
	v[2] = a[2] + b[2];
	v[3] = a[3] + b[3];

	return v;
}

static inline Vec4 operator *(Vec4 a, r32 s) {
	Vec4 v = {};

	v[0] = a[0] * s;
	v[1] = a[1] * s;
	v[2] = a[2] * s;
	v[3] = a[3] * s;

	return v;
}

static inline Vec4 operator *(r32 s, Vec4 a) {
	Vec4 v = {};

	v[0] = a[0] * s;
	v[1] = a[1] * s;
	v[2] = a[2] * s;
	v[3] = a[3] * s;

	return v;
}

static inline s32 TruncateI64(s64 value) {
	Assert(value <= 0x7FFFFFFF);
	s32 result = (s32)value;
	return result;
}

static inline u32 TruncateU64(u64 value) {
	Assert(value <= 0xFFFFFFFF); 
	u32 result = (u32)value;
	return result;
}

static inline s32 RoundReal32ToS32(r32 value) {
	s32 result = (s32)(value + 0.5f);
	return result;
}

static inline u32 RoundReal32ToU32(r32 value) {
	u32 result = (u32)(value + 0.5f);
	return result;
}

extern void Mat1x3Mul(r32 out[3], const r32 a[3], const r32 b[3][3]);
extern void Mat1x4Mul(r32 out[4], const r32 a[4], const r32 b[16]);
extern void Mat2x2Mul(r32 out[4], const r32 a[4], const r32 b[4]);
extern void Mat3x3Mul(r32 out[9], const r32 a[9], const r32 b[9]);
extern void Mat4x4Mul(r32 out[16], const r32 a[16], const r32 b[16]);

extern void Mat1x4Mul(r32 out[4], const r32 a[4], const r32 b[4][4]);
extern void Mat2x2Mul(r32 out[2][2], const r32 a[2][2], const r32 b[2][2]);
extern void Mat3x3Mul(r32 out[3][3], const r32 a[3][3], const r32 b[3][3]);
extern void Mat4x4Mul(r32 out[4][4], const r32 a[4][4], const r32 b[4][4]);

extern void Mat1x3Mul(Vec3 *out, const Vec3 *a, const r32 b[9]);
extern void Mat1x3Mul(Vec3 *out, const Vec3 *a, const r32 b[3][3]);
extern void Mat1x4Mul(Vec4 *out, const Vec4 *a, const r32 b[4][4]);
extern void MatTranspose(r32 out[9], const r32 in[9]);

extern void RotatePoints(r32 rot_mat[3][3], struct PolyVert *poly_verts, int num_verts);

struct Orientation {
	Vec3	origin;			// in world coordinates
	Vec3	axis[3];		// orientation in world, uvn
	Vec3	dir;			// look at vector in uvn system
};

struct Plane {
	Vec3	unit_normal;
	r32		dist;
	//byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	//byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
};

struct Dim2d {
	int		width;          
	int		height;
};

/*
==============================================================

GENERAL

==============================================================
*/

//
// stack based allocator
//

struct MemoryStack {
	byte *	base_ptr;
	size_t	max_size;
	size_t	bytes_used;
};

//
// fixed size allocator
//

//struct FBAllocator {
//	byte *	data;
//	size_t	max_size;
//	int		num_rows;
//};

// NOTE: dont use the _Push_ and _Pop_ functions directly, go through the macros
// FIXME: pass alignment
extern void *_Push_(MemoryStack *ms, size_t num_bytes);
extern void _Pop_(MemoryStack *ms, size_t num_bytes);

#define PushSize(stack, size, type) ((type *)_Push_(stack, size))  

#define PushStruct(stack, type) ((type *)_Push_(stack, sizeof(type)))  
#define PopStruct(stack, type) (_Pop_(stack, sizeof(type)))  

#define PushArray(stack, count, type) ((type *)_Push_(stack, (count) * sizeof(type)))
#define PopArray(stack, count, type) (_Pop_(stack, (count) * sizeof(type)))  


// windows specific
#ifdef _WIN32
#include <Windows.h>
#ifdef PLATFORM_DEBUG
#define InvalidCodePath(m) do { MessageBoxA(0, "Invalid code path: " ##m, 0, 0); Com_Quit(); } while(0)
#define CheckMemory(cond) do { if (!(cond)) { MessageBoxA(0, "Out of memory in: "##__FILE__, 0, 0); __debugbreak(); } } while(0)
#define EventOverflow do { MessageBoxA(0, "Event overflow", 0, 0); Assert(0); } while(0)
#else
#define InvalidCodePath(m) 
#define CheckMemory(cond) 
#define EventOverflow 
#endif
//#define memset ZeroMemory
#else
#define InvalidCodePath do { Assert(0); } while(0)
#define OutOfMemory do { Assert(0); } while(0)
#define EventOverflow do {  Assert(0); } while(0)
#endif	// _WIN32

#endif	// Header guard