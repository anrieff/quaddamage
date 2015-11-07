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
 * @File geometry.cpp
 * @Brief Contains implementations of geometry primitives' intersection methods.
 */
#include "geometry.h"
#include "constants.h"
#include <algorithm>
using std::vector;

bool Plane::intersect(const Ray& ray, IntersectionInfo& info)
{
	if (ray.start.y > this->y && ray.dir.y >= 0)
		return false;
	if (ray.start.y < this->y && ray.dir.y <= 0)
		return false;
	
	// this->y = 1
	// ray.start.y = 6
	// ray.dir.y = -1
	// (1 - 6) / -1 = -5 / -1 = 5
	double scaleFactor = (this->y - ray.start.y) / ray.dir.y;
	info.ip = ray.start + ray.dir * scaleFactor;
	info.distance = scaleFactor;
	info.normal = Vector(0, ray.start.y > this->y ? 1 : -1, 0);
	info.u = info.ip.x;
	info.v = info.ip.z;
	info.geom = this;
	return true;
}

bool Sphere::intersect(const Ray& ray, IntersectionInfo& info)
{
	// H = ray.start - O
	// p^2 * dir.length()^2 + p * 2 * dot(H, dir) + H.length()^2 - R^2 == 0
	Vector H = ray.start - O;
	double A = 1; // ray.lengthSqr()
	double B = 2 * dot(H, ray.dir);
	double C = H.lengthSqr() - R*R;
	
	double discr = B*B - 4*A*C;
	
	if (discr < 0) return false; // no intersection
	
	double p1, p2;
	p1 = (-B - sqrt(discr)) / (2 * A);
	p2 = (-B + sqrt(discr)) / (2 * A);
	double p;
	// p1 <= p2
	bool backNormal = false;
	if (p1 > 0) p = p1;
	else if (p2 > 0) {
		p = p2;
		backNormal = true;
	}
	else return false;
	
	info.distance = p;
	info.ip = ray.start + ray.dir * p;
	info.normal = info.ip - O;
	info.normal.normalize();
	if (backNormal) info.normal = -info.normal;
	info.u = info.v = 0;
	Vector posRelative = info.ip - O;
	info.u = atan2(posRelative.z, posRelative.x);
	info.v = asin(posRelative.y / R);
	// remap [(-PI..PI)x(-PI/2..PI/2)] -> [(0..1)x(0..1)]
	info.u = (info.u + PI) / (2*PI);
	info.v = (info.v + PI/2) / (PI);
	info.geom = this;
	return true;
}

bool Cube::intersectSide(double level, double start, double dir, const Ray& ray, const Vector& normal, IntersectionInfo& info)
{
	if (start > level && dir >= 0)
		return false;
	if (start < level && dir <= 0)
		return false;
	
	double scaleFactor = (level - start) / dir;
	Vector ip = ray.start + ray.dir * scaleFactor;
	if (ip.y > O.y + halfSide + 1e-6) return false;
	if (ip.y < O.y - halfSide - 1e-6) return false;
	
	if (ip.x > O.x + halfSide + 1e-6) return false;
	if (ip.x < O.x - halfSide - 1e-6) return false;

	if (ip.z > O.z + halfSide + 1e-6) return false;
	if (ip.z < O.z - halfSide - 1e-6) return false;
	
	double distance = scaleFactor;
	if (distance < info.distance) {
		info.ip = ip;
		info.distance = distance;
		info.normal = normal;
		info.u = info.ip.x + info.ip.z;
		info.v = info.ip.y;
		info.geom = this;
		return true;
	}
	return false;
}

bool Cube::intersect(const Ray& ray, IntersectionInfo& info)
{
	info.distance = INF;
	intersectSide(O.x - halfSide, ray.start.x, ray.dir.x, ray, Vector(-1, 0, 0), info);
	intersectSide(O.x + halfSide, ray.start.x, ray.dir.x, ray, Vector(+1, 0, 0), info);
	intersectSide(O.y - halfSide, ray.start.y, ray.dir.y, ray, Vector( 0,-1, 0), info);
	intersectSide(O.y + halfSide, ray.start.y, ray.dir.y, ray, Vector( 0,+1, 0), info);
	intersectSide(O.z - halfSide, ray.start.z, ray.dir.z, ray, Vector( 0, 0,-1), info);
	intersectSide(O.z + halfSide, ray.start.z, ray.dir.z, ray, Vector( 0, 0,+1), info);
	
	return (info.distance < INF);
}

void CsgOp::findAllIntersections(Ray ray, Geometry* geom, std::vector<IntersectionInfo>& ips)
{
	IntersectionInfo info;
	int counter = 30;
	while (geom->intersect(ray, info) && counter-- > 0) {
		ips.push_back(info);
		ray.start = info.ip + ray.dir * 1e-6;
	}
	for (int i = 1; i < (int) ips.size(); i++)
		ips[i].distance = ips[i - 1].distance + ips[i].distance + 1e-6;
}

bool CsgOp::intersect(const Ray& ray, IntersectionInfo& info)
{
	vector<IntersectionInfo> leftIPs, rightIPs;
	findAllIntersections(ray, left, leftIPs);
	findAllIntersections(ray, right, rightIPs);
	
	bool inA = leftIPs.size() % 2 ? true : false;
	bool inB = rightIPs.size() % 2 ? true : false;
	
	vector<IntersectionInfo> allIPs;
	allIPs = leftIPs;
	for (auto& ip: rightIPs) allIPs.push_back(ip);
	
	sort(allIPs.begin(), allIPs.end(), [] (const IntersectionInfo& left, const IntersectionInfo& right) { return left.distance < right.distance; });
	
	bool predicateNow = boolOp(inA, inB);
	
	for (auto& ip: allIPs) {
		if (ip.geom == left)
			inA = !inA;
		else
			inB = !inB;
			
		bool predicateNext = boolOp(inA, inB);
		
		if (predicateNext != predicateNow) {
			info = ip;
			return true;
		}
	}
	
	return false;
}
