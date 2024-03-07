#pragma once
#include "Vector4.h"
#include <corecrt_math.h>
#include "Vector2D.h"
struct FVector
{
	float X, Y, Z;

	FVector() : X(0.f), Y(0.f), Z(0.f) {}

	FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

	FVector(float InF) : X(InF), Y(InF), Z(InF) { }

	FVector(const FVector4& V) : X(V.X), Y(V.Y), Z(V.Z) {};

	float Size() const { return sqrtf(X * X + Y * Y + Z * Z); }

	float Sum() const { return X + Y + Z; }

	float Size2D() const { return sqrtf(X * X + Y * Y); }

	float SizeSquared() const { return X * X + Y * Y + Z * Z; }

	float DistTo(const FVector& V) const { return (*this - V).Size(); }

	FVector unit() const {
		float mult = (1.f / Size()); 
		return FVector(X * mult, Y * mult, Z * mult);
	}

	FVector operator+(const FVector& other) const { return FVector(X + other.X, Y + other.Y, Z + other.Z); }

	FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }

	FVector operator*(const FVector& V) const { return FVector(X * V.X, Y * V.Y, Z * V.Z); }

	FVector operator/(const FVector& V) const { return FVector(X / V.X, Y / V.Y, Z / V.Z); }

	bool operator==(const FVector& V) const { return X == V.X && Y == V.Y && Z == V.Z; }

	bool operator!=(const FVector& V) const { return X != V.X || Y != V.Y || Z != V.Z; }

	FVector operator-() const { return FVector(-X, -Y, -Z); }

	FVector operator+(float Bias) const { return FVector(X + Bias, Y + Bias, Z + Bias); }

	FVector operator-(float Bias) const { return FVector(X - Bias, Y - Bias, Z - Bias); }

	FVector operator*(float Scale) const { return FVector(X * Scale, Y * Scale, Z * Scale); } const

	FVector operator/(float Scale) const { const float RScale = 1.f / Scale; return FVector(X * RScale, Y * RScale, Z * RScale); }

	FVector operator=(const FVector& V) { X = V.X; Y = V.Y; Z = V.Z; return *this; }

	FVector operator+=(const FVector& V) { X += V.X; Y += V.Y; Z += V.Z; return *this; }

	FVector operator-=(const FVector& V) { X -= V.X; Y -= V.Y; Z -= V.Z; return *this; }

	FVector operator*=(const FVector& V) { X *= V.X; Y *= V.Y; Z *= V.Z; return *this; }

	FVector operator/=(const FVector& V) { X /= V.X; Y /= V.Y; Z /= V.Z; return *this; }

	FVector operator*=(float Scale) { X *= Scale; Y *= Scale; Z *= Scale; return *this; }

	FVector operator/=(float V) { const float RV = 1.f / V; X *= RV; Y *= RV; Z *= RV; return *this; }

	float operator|(const FVector& V) const { return X * V.X + Y * V.Y + Z * V.Z; }

	FVector operator^(const FVector& V) const { return FVector(Y * V.Z - Z * V.Y,Z * V.X - X * V.Z,X * V.Y - Y * V.X); }

	static const FVector ZeroVector;

	static const FVector OneVector;
};


class Vector2
{
public:
	Vector2(void)
	{
		x = y = 0.0f;
	}

	Vector2(FVector2D& v)
	{
		x = v.X; y = v.Y;
	}

	Vector2(float X, float Y)
	{
		x = X; y = Y;
	}

	Vector2(float* v)
	{
		x = v[0]; y = v[1];
	}

	Vector2(const float* v)
	{
		x = v[0]; y = v[1];
	}

	Vector2(const Vector2& v)
	{
		x = v.x; y = v.y;
	}

	Vector2& operator=(const Vector2& v)
	{
		x = v.x; y = v.y; return *this;
	}

	float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vector2& operator+=(const Vector2& v)
	{
		x += v.x; y += v.y; return *this;
	}

	Vector2& operator-=(const Vector2& v)
	{
		x -= v.x; y -= v.y; return *this;
	}

	Vector2& operator*=(const Vector2& v)
	{
		x *= v.x; y *= v.y; return *this;
	}

	Vector2& operator/=(const Vector2& v)
	{
		x /= v.x; y /= v.y; return *this;
	}

	Vector2& operator+=(float v)
	{
		x += v; y += v; return *this;
	}

	Vector2& operator-=(float v)
	{
		x -= v; y -= v; return *this;
	}

	Vector2& operator*=(float v)
	{
		x *= v; y *= v; return *this;
	}

	Vector2& operator/=(float v)
	{
		x /= v; y /= v; return *this;
	}

	Vector2 operator+(const Vector2& v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	Vector2 operator-(const Vector2& v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	Vector2 operator*(const Vector2& v) const
	{
		return Vector2(x * v.x, y * v.y);
	}

	Vector2 operator/(const Vector2& v) const
	{
		return Vector2(x / v.x, y / v.y);
	}

	Vector2 operator+(float v) const
	{
		return Vector2(x + v, y + v);
	}

	Vector2 operator-(float v) const
	{
		return Vector2(x - v, y - v);
	}

	Vector2 operator*(float v) const
	{
		return Vector2(x * v, y * v);
	}

	Vector2 operator/(float v) const
	{
		return Vector2(x / v, y / v);
	}

	void Set(float X = 0.0f, float Y = 0.0f)
	{
		x = X; y = Y;
	}

	float Length(void) const
	{
		return sqrtf(x * x + y * y);
	}

	float LengthSqr(void) const
	{
		return (x * x + y * y);
	}


	float DistTo(const Vector2& v) const
	{
		return (*this - v).Length();
	}

	float DistToSqr(const Vector2& v) const
	{
		return (*this - v).LengthSqr();
	}

	float Dot(const Vector2& v) const
	{
		return (x * v.x + y * v.y);
	}

	bool IsZero(void) const
	{
		return (x > -0.01f && x < 0.01f &&
			y > -0.01f && y < 0.01f);
	}

public:
	float x, y;
};

class Vector3
{
public:

	Vector3(void)
	{
		x = y = z = 0.0f;
	}

	Vector3(float X, float Y, float Z)
	{
		x = X; y = Y; z = Z;
	}

	Vector3(float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vector3(const float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vector3(const Vector3& v)
	{
		x = v.x; y = v.y; z = v.z;
	}

	Vector3(const Vector2& v)
	{
		x = v.x; y = v.y; z = 0.0f;
	}

	Vector3& operator=(const Vector3& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}



	Vector3& operator=(const Vector2& v)
	{
		x = v.x; y = v.y; z = 0.0f; return *this;
	}

	float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	Vector3& operator*=(const Vector3& v)
	{
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}

	Vector3& operator/=(const Vector3& v)
	{
		x /= v.x; y /= v.y; z /= v.z; return *this;
	}

	Vector3& operator==(const Vector3& v)
	{

	}

	Vector3& operator+=(float v)
	{
		x += v; y += v; z += v; return *this;
	}

	Vector3& operator-=(float v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	Vector3& operator*=(float v)
	{
		x *= v; y *= v; z *= v; return *this;
	}

	Vector3& operator/=(float v)
	{
		x /= v; y /= v; z /= v; return *this;
	}



	Vector3 operator+(const Vector3& v) const
	{
		if (this->x == 0 && this->y == 0 && this->z == 0)
			return Vector3(0.0f, 0.0f, 0.0f);

		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(const Vector3& v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(const Vector3& v) const
	{
		return Vector3(x * v.x, y * v.y, z * v.z);
	}

	Vector3 operator/(const Vector3& v) const
	{
		return Vector3(x / v.x, y / v.y, z / v.z);
	}

	Vector3 operator+(float v) const
	{
		return Vector3(x + v, y + v, z + v);
	}

	Vector3 operator-(float v) const
	{
		return Vector3(x - v, y - v, z - v);
	}

	Vector3 operator*(float v) const
	{
		return Vector3(x * v, y * v, z * v);
	}

	Vector3 operator/(float v) const
	{
		return Vector3(x / v, y / v, z / v);
	}

	void Set(float X = 0.0f, float Y = 0.0f, float Z = 0.0f)
	{
		x = X; y = Y; z = Z;
	}

	float Length(void) const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float LengthSqr(void) const
	{
		return (x * x + y * y + z * z);
	}

	float Length2d(void) const
	{
		return sqrtf(x * x + y * y);
	}

	float Length2dSqr(void) const
	{
		return (x * x + y * y);
	}

	float DistTo(const Vector3& v) const
	{
		return (*this - v).Length();
	}

	float DistToSqr(const Vector3& v) const
	{
		return (*this - v).LengthSqr();
	}

	float Dot(const Vector3& v) const
	{
		if (v.x == 0 && v.y == 0 && v.z == 0)
			return 0.0f;

		return (x * v.x + y * v.y + z * v.z);


	}

	inline void Rotate2D(const float& f)
	{
#define DEG2RAD(x)((float)(x) * (float)((float)(3.14159265358979323846f) / 180.0f))

		float _x, _y;

		float s, c;

		s = sin(DEG2RAD(f));
		c = cos(DEG2RAD(f));

		_x = x;
		_y = y;

		x = (_x * c) - (_y * s);
		y = (_x * s) + (_y * c);
	}

	Vector3 Cross(const Vector3& v) const
	{
		return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	bool IsZero(void) const
	{
		return (x > -0.01f && x < 0.01f &&
			y > -0.01f && y < 0.01f &&
			z > -0.01f && z < 0.01f);
	}



public:
	float x, y, z;
};