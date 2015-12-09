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

#include <vector>
#include "vector.h"
#include "transform.h"
#include "scene.h"


class Geometry;
struct IntersectionInfo {
	Vector ip;
	Vector normal;
	double distance;
	double u, v;
	Geometry* geom;
	Vector rayDir;
	Vector dNdx, dNdy;
};

/**
 * @class Intersectable
 * @brief implements the interface to an intersectable primitive (geometry or node)
 */
class Intersectable {
public:
	virtual bool intersect(const Ray& ray, IntersectionInfo& info) = 0;
};

class Geometry: public Intersectable, public SceneElement {
public:
	virtual ~Geometry() {}
	ElementType getElementType() const { return ELEM_GEOMETRY; }
};

class Plane: public Geometry {
public:
	double y;
	double limit;
	Plane() { y = 0; limit = 1e99; }
	void fillProperties(ParsedBlock& pb)
	{
		pb.getDoubleProp("y", &y);
		pb.getDoubleProp("limit", &limit);
	}
	bool intersect(const Ray& ray, IntersectionInfo& info);
};

class Sphere: public Geometry {
public:
	Vector O;
	double R;
	Sphere(Vector center = Vector(0, 0, 0), double radius = 1): O(center), R(radius) {}
	void fillProperties(ParsedBlock& pb)
	{
		pb.getVectorProp("O", &O);
		pb.getDoubleProp("R", &R);
	}
	
	bool intersect(const Ray& ray, IntersectionInfo& info);
};

class Cube: public Geometry {
	bool intersectSide(double level, double start, double dir, const Ray& ray, const Vector& normal, IntersectionInfo& info);
public:
	Vector O;
	double halfSide;
	Cube(Vector O = Vector(0, 0, 0), double halfSide = 0.5): O(O), halfSide(halfSide) {}

	void fillProperties(ParsedBlock& pb)
	{
		pb.getVectorProp("O", &O);
		pb.getDoubleProp("halfSide", &halfSide);
	}

	bool intersect(const Ray& ray, IntersectionInfo& info);
};

class CsgOp: public Geometry {
	void findAllIntersections(Ray ray, Geometry* geom, std::vector<IntersectionInfo>& ips);
public:
	Geometry *left, *right;
	
	virtual bool boolOp(bool inA, bool inB) = 0;

	void fillProperties(ParsedBlock& pb)
	{
		pb.requiredProp("left");
		pb.requiredProp("right");
		pb.getGeometryProp("left", &left);
		pb.getGeometryProp("right", &right);
	}
	
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

class Shader;

struct Node: public Intersectable, public SceneElement {
	Geometry* geom;
	Shader* shader;
	Transform transform;
	Texture* bump;
	
	Node() { bump = NULL; }
	Node(Geometry* g, Shader* s) { geom = g; shader = s; bump = NULL; }
	
	// from Intersectable:
	bool intersect(const Ray& ray, IntersectionInfo& data);

	// from SceneElement:
	ElementType getElementType() const { return ELEM_NODE; }
	void fillProperties(ParsedBlock& pb)
	{
		pb.getGeometryProp("geometry", &geom);
		pb.getShaderProp("shader", &shader);
		pb.getTransformProp(transform);
		pb.getTextureProp("bump", &bump);
	}
};

#endif // __GEOMETRY_H__
