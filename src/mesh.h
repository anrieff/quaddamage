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
 * @File mesh.h
 * @Brief Contains the Mesh class.
 */
#ifndef __MESH_H__
#define __MESH_H__

#include <vector>
#include "geometry.h"
#include "vector.h"

struct Triangle {
	int v[3], n[3], t[3];
	Vector gnormal;
};

// the C vertex of triangle with index 5 is:
// mesh.vertices[mesh.triangles[5].v[2]];

struct Mesh: public Geometry {
	std::vector<Vector> vertices;
	std::vector<Vector> normals;
	std::vector<Vector> uvs;
	std::vector<Triangle> triangles;
	
	Geometry* boundingGeom;
	bool faceted;
	
	Mesh(bool isTetraeder /* wtf? */);
	~Mesh();
	
	void setFaceted(bool faceted) { this->faceted = faceted; }
	bool intersect(const Ray& ray, IntersectionInfo& info);
	void translate(Vector amount);
	void computeBoundingGeometry();
public:
	bool intersectTriangle(const Ray& ray, const Triangle& t, IntersectionInfo& info);
	void generateTetraeder();
	void generateTruncatedIcosahedron();
};

#endif // __MESH_H__
