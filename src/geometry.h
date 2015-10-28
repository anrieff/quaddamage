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

struct IntersectionInfo {
	Vector ip;
	Vector normal;
	double distance;
	double u, v;
};

class Geometry {
public:
	virtual bool intersect(const Ray& ray, IntersectionInfo& info) = 0;
	virtual ~Geometry() {}
};

class Plane: public Geometry {
public:
	double y;
	bool intersect(const Ray& ray, IntersectionInfo& info);
};

struct Shader;

struct Node {
	Geometry* geom;
	Shader* shader;
};

#endif // __GEOMETRY_H__
