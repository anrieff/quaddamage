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
 * @File heightfield.h
 * @Brief Contains the Heightfield geometry class.
 */
#ifndef __HEIGHTFIELD_H__
#define __HEIGHTFIELD_H__

#include "geometry.h"
#include "bbox.h"

class Heightfield: public Geometry {
	float* heights, *maxH;
	Vector* normals;
	BBox bbox;
	int W, H;
	float getHeight(int x, int y) const;
	float getHighest(int x, int y, int k) const;
	Vector getNormal(float x, float y) const;
	
	bool useOptimization;
	struct HighStruct {
		float h[16];
	};
	HighStruct* highMap;
	int maxK;
	
	void buildHighMap();
	
public:
	Heightfield();
	~Heightfield();
	void beginRender();
	bool intersect(const Ray& ray, IntersectionInfo& info);
	bool isInside(const Vector& p ) const { return false; }
	void fillProperties(ParsedBlock& pb);
};

#endif // __HEIGHTFIELD_H__

