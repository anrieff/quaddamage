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
	return true;
}
