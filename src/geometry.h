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
 * @File geometry.h
 * @Brief Contains declarations of geometry primitives.
 */
#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#include "vector.h"
#include <vector>


class Geometry;
struct IntersectionInfo {
	Vector ip;
	Vector normal;
	double distance;
	double u, v;
	Geometry* geom;
	Vector rayDir;
};

class Geometry {
public:
	virtual bool intersect(const Ray& ray, IntersectionInfo& info) = 0;
	virtual ~Geometry() {}
};

class Plane: public Geometry {
public:
	double y;
	double limit;
	Plane() { y = 0; limit = 1e99; }
	bool intersect(const Ray& ray, IntersectionInfo& info);
};

class Sphere: public Geometry {
public:
	Vector O;
	double R;
	
	bool intersect(const Ray& ray, IntersectionInfo& info);
};

class Cube: public Geometry {
	bool intersectSide(double level, double start, double dir, const Ray& ray, const Vector& normal, IntersectionInfo& info);
public:
	Vector O;
	double halfSide;

	bool intersect(const Ray& ray, IntersectionInfo& info);
};

class CsgOp: public Geometry {
	void findAllIntersections(Ray ray, Geometry* geom, std::vector<IntersectionInfo>& ips);
public:
	Geometry *left, *right;
	
	virtual bool boolOp(bool inA, bool inB) = 0;
	
	bool intersect(const Ray& ray, IntersectionInfo& info);
};

class CsgAnd: public CsgOp {
public:
	bool boolOp(bool inA, bool inB) { return inA && inB; }
};

class CsgPlus: public CsgOp {
public:
	bool boolOp(bool inA, bool inB) { return inA || inB; }
};

class CsgMinus: public CsgOp {
public:
	bool boolOp(bool inA, bool inB) { return inA && !inB; }
};

struct Shader;

struct Node {
	Geometry* geom;
	Shader* shader;
};

#endif // __GEOMETRY_H__
