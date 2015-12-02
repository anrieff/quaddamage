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
 * @File mesh.cpp
 * @Brief Contains implementation of the Mesh class
 */

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "mesh.h"
#include "constants.h"
#include "color.h"
using std::max;
using std::vector;


void Mesh::beginRender()
{
	if (isTetraeder)
		generateTetraeder();
	else
		generateTruncatedIcosahedron();
	
	printf("Mesh loaded, %d triangles\n", int(triangles.size()));
	computeBoundingGeometry();
}

void Mesh::computeBoundingGeometry()
{
	Vector O(0, 0, 0);
	
	for (auto& v: vertices) {
		O += v;
	}
	
	O = O / vertices.size();
	
	double maxRadius = 0;
	for (auto&v : vertices) {
		double distance = (O - v).length();
		maxRadius = max(maxRadius, distance);
	}
	
	boundingGeom = new Sphere(O, maxRadius);
}

Mesh::~Mesh()
{
	if (boundingGeom) delete boundingGeom;
}

inline double det(const Vector& a, const Vector& b, const Vector& c)
{
	return (a^b) * c;
}

bool Mesh::intersectTriangle(const Ray& ray, const Triangle& t, IntersectionInfo& info)
{
	if (dot(ray.dir, t.gnormal) > 0) return false;
	Vector A = vertices[t.v[0]];
	Vector B = vertices[t.v[1]];
	Vector C = vertices[t.v[2]];
	
	Vector H = ray.start - A;
	Vector D = ray.dir;
	
	double Dcr = det(B-A, C-A, -D);
	if (fabs(Dcr) < 1e-12) return false;
	
	double lambda2 = det(H, C-A, -D) / Dcr;
	double lambda3 = det(B-A, H, -D) / Dcr;
	
	if (lambda2 < 0 || lambda3 < 0) return false;
	if (lambda2 > 1 || lambda3 > 1) return false;
	if (lambda2 + lambda3 > 1) return false;
	
	double gamma = det(B-A, C-A, H) / Dcr;
	
	if (gamma < 0) return false;
	
	info.distance = gamma;
	info.ip = ray.start + ray.dir * gamma;
	if (!faceted) {
		Vector nA = normals[t.n[0]];
		Vector nB = normals[t.n[1]];
		Vector nC = normals[t.n[2]];
		
		info.normal = nA + (nB - nA) * lambda2 + (nC - nA) * lambda3;
		info.normal.normalize();
	} else {
		info.normal = t.gnormal;
	}
			
	Vector uvA = uvs[t.t[0]];
	Vector uvB = uvs[t.t[1]];
	Vector uvC = uvs[t.t[2]];
	
	Vector uv = uvA + (uvB - uvA) * lambda2 + (uvC - uvA) * lambda3;
	info.u = uv.x;
	info.v = uv.y;
	info.geom = this;
	
	return true;
}


bool Mesh::intersect(const Ray& ray, IntersectionInfo& info)
{
	if (boundingGeom->intersect(ray, info) == false)
		return false;
	bool found = false;
	IntersectionInfo closestInfo;
	
	closestInfo.distance = INF;
	
	for (auto& T: triangles) {
		if (intersectTriangle(ray, T, info) &&
			info.distance < closestInfo.distance) {
				found = true;
				closestInfo = info;
		}
	}
	
	if (found)
		info = closestInfo;
	return found;
}
