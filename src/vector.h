/***************************************************************************
 *   Copyright (C) 2009-2015 by Veselin Georgiev, Slavomir Kaslev et al    *
 *   admin@raytracing-bg.net                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/**
 * @File vector.h
 * @Brief defines the Vector class (a 3D vector with the usual algebraic operations)
 */ 
#ifndef __VECTOR3D_H__
#define __VECTOR3D_H__

#include <stdio.h>
#include <math.h>

struct Vector {
	union {
		struct { double x, y, z; };
		double v[3];
	};
	
	Vector () {}
	Vector(double _x, double _y, double _z) { set(_x, _y, _z); }
	void set(double _x, double _y, double _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	void makeZero(void)
	{
		x = y = z = 0.0;
	}
	inline double length(void) const
	{
		return sqrt(x * x + y * y + z * z);
	}
	inline double lengthSqr(void) const
	{
		return (x * x + y * y + z * z);
	}
	void scale(double multiplier)
	{
		x *= multiplier;
		y *= multiplier;
		z *= multiplier;
	}
	void operator *= (double multiplier)
	{
		scale(multiplier);
	}
	void operator += (const Vector& other)
	{
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
	}
	void operator /= (double divider)
	{
		scale(1.0 / divider);
	}
	void normalize(void)
	{
		double multiplier = 1.0 / length();
		scale(multiplier);
	}
	void setLength(double newLength)
	{
		scale(newLength / length());
	}
	int maxDimension() const
	{
		double maxVal = fabs(x);
		int maxDim = 0;
		if (fabs(y) > maxVal) {
			maxDim = 1;
			maxVal = fabs(y);
		}
		if (fabs(z) > maxVal) {
			maxDim = 2;
		}
		return maxDim;
	}
	inline double& operator[](const int index) { return v[index]; }
	inline const double& operator[](const int index) const { return v[index]; }
	
	void print() const { printf("(%.9lf, %.9lf, %.9lf)", x, y, z); }
};

inline Vector operator + (const Vector& a, const Vector& b)
{
	return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vector operator - (const Vector& a, const Vector& b)
{
	return Vector(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector operator - (const Vector& a)
{
	return Vector(-a.x, -a.y, -a.z);
}

/// dot product
inline double operator * (const Vector& a, const Vector& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
/// dot product (functional form, to make it more explicit):
inline double dot(const Vector& a, const Vector& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
/// cross product
inline Vector operator ^ (const Vector& a, const Vector& b)
{
	return Vector(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

inline Vector operator * (const Vector& a, double multiplier)
{
	return Vector(a.x * multiplier, a.y * multiplier, a.z * multiplier);
}
inline Vector operator * (double multiplier, const Vector& a)
{
	return Vector(a.x * multiplier, a.y * multiplier, a.z * multiplier);
}
inline Vector operator / (const Vector& a, double divider)
{
	double multiplier = 1.0 / divider;
	return Vector(a.x * multiplier, a.y * multiplier, a.z * multiplier);
}

inline Vector normalize(const Vector& vec)
{
	double multiplier = 1.0 / vec.length();
	return vec * multiplier;
}

inline Vector reflect(Vector in, const Vector& norm)
{
	in.normalize();
	in += 2 * norm * dot(norm, -in);
	in.normalize();
	return in;
}

inline Vector faceforward(const Vector& ray, const Vector& norm)
{
	if (dot(ray, norm) < 0) return norm;
	else return -norm;
}

/// inRay must be an unit vector. Returns vectors ray1 and ray2 such that
/// inRay, ray1 and ray2 form an orthonormal system in 3D (all vectors are
/// unit, and are mutually orthogonal)
inline void orthonormalSystem(const Vector& inRay, Vector& ray1, Vector& ray2)
{
	const Vector FIXED_SAMPLES[2] = {
		{-0.267261242, 0.534522484, -0.801783726},
		{+0.483368245, 0.096673649, +0.870062840}
	};
	ray1 = fabs(dot(inRay, FIXED_SAMPLES[0])) > 0.99 ? inRay ^ FIXED_SAMPLES[1] : inRay ^ FIXED_SAMPLES[0];
	ray1.normalize();
	ray2 = inRay ^ ray1;
	ray2.normalize();
}

struct Ray {
	Vector start;
	Vector dir; //!< normed!
	int depth;
	bool debug;
	Ray()
	{
		depth = 0;
		debug = false;
	}
};

#endif // __VECTOR3D_H__
