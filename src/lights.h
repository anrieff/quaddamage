
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
#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "scene.h"
#include "transform.h"

class Light: public SceneElement {
protected:
	Color color;
	float power;
public:
	Light() { color = Color(1, 1, 1); power = 1; }
	virtual ~Light() {}
	
	ElementType getElementType() const { return ELEM_LIGHT; }
	
	virtual int getNumSamples() = 0;
	
	virtual void getNthSample(int sampleIdx, const Vector& shadePos,
							  Vector& samplePos, Color& color) = 0;

	void fillProperties(ParsedBlock& pb)
	{
		pb.getColorProp("color", &color);
		pb.getFloatProp("power", &power);
	}
};

class PointLight: public Light {
	Vector pos;
public:
	void fillProperties(ParsedBlock& pb)
	{
		Light::fillProperties(pb);
		pb.getVectorProp("pos", &pos);
	}
	
	int getNumSamples()
	{
		return 1;
	}
	
	void getNthSample(int sampleIdx, const Vector& shadePos,
							  Vector& samplePos, Color& color)
	{
		color = this->color * power;
		samplePos = pos;
	}
};

class RectLight: public Light {
	int xSubd, ySubd;
	Transform T;
public:
	void fillProperties(ParsedBlock& pb)
	{
		Light::fillProperties(pb);
		pb.getIntProp("xSubd", &xSubd, 1);
		pb.getIntProp("ySubd", &ySubd, 1);
		pb.getTransformProp(T);
	}
	
	int getNumSamples();
	
	void getNthSample(int sampleIdx, const Vector& shadePos,
							  Vector& samplePos, Color& color);
	
};

#endif // __LIGHTS_H__

