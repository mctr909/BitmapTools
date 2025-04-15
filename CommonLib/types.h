#ifndef __TYPES_H__
#define __TYPES_H__

#define INVALID_INDEX 0xffffffffui32

#pragma pack(push, 2)
struct direction {
	int8_t x;
	int8_t y;
};
#pragma pack(pop)

#pragma pack(push, 8)
/* 2次元座標 */
struct point {
	int32_t x;
	int32_t y;
};
#pragma pack(pop)

#pragma pack(push, 8)
/* 3次元ベクトル */
struct vec3 {
	double x;
	double y;
	double z;
	vec3() { x = 0; y = 0; z = 0; }
	vec3(double x, double y, double z) {
		this->x = x; this->y = y; this->z = z;
	}
	vec3 operator - () {
		return {-x, -y, -z};
	}
	/* ベクトル(加法) */
	vec3 operator + (vec3 const &v) {
		return {
			this->x + v.x,
			this->y + v.y,
			this->z + v.z
		};
	}
	/* ベクトル(減法) */
	vec3 operator - (vec3 const &v) {
		return {
			this->x - v.x,
			this->y - v.y,
			this->z - v.z
		};
	}
	/* ベクトル(クロス積) */
	vec3 operator * (vec3 const &v) {
		return {
			this->y * v.z - this->z * v.y,
			this->z * v.x - this->x * v.z,
			this->x * v.y - this->y * v.x,
		};
	}
	/* ベクトル(スカラー倍) */
	vec3 operator * (const double k) {
		return {x * k, y * k, z * k};
	}
	/* ベクトル(スカラー倍 1/k) */
	vec3 operator / (const double k) {
		return {x / k, y / k, z / k};
	}
	/* ベクトル(内積) */
	double operator & (vec3 const &v) {
		return this->x * v.x + this->y * v.y + this->z * v.z;
	}
	/* ベクトル(加算) */
	void operator += (vec3 const &v) {
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
	}
	/* ベクトル(減算) */
	void operator -= (vec3 const &v) {
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
	}
	/* ベクトル(スカラー倍) */
	void operator *= (const double k) {
		x *= k; y *= k; z *= k;
	}
	/* ベクトル(スカラー倍 1/k) */
	void operator /= (const double k) {
		x /= k; y /= k; z /= k;
	}
};
#pragma pack(pop)

#endif //__TYPES_H__
