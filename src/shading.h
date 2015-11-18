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
 * @File shading.h
 * @Brief Contains declarations of shader classes
 */
#ifndef __SHADING_H__
#define __SHADING_H__

#include "geometry.h"
#include "color.h"

class Shader {
public:
	virtual Color shade(const Ray& ray, const IntersectionInfo& info) = 0;
	virtual ~Shader() {}
};

class Texture {
public:
	virtual Color sample(const IntersectionInfo& info) = 0;
	virtual ~Texture() {}
};

class CheckerTexture: public Texture {
public:
	Color color1, color2;
	double scaling;
	CheckerTexture() { color1.makeZero(); color2.makeZero(); scaling = 1; }
	virtual Color sample(const IntersectionInfo& info);
};

class Bitmap;
class BitmapTexture: public Texture {
	Bitmap* bitmap;
	double scaling;
public:
	BitmapTexture(const char* filename, double scaling = 1.0);
	~BitmapTexture();
	Color sample(const IntersectionInfo& info);
};

class Lambert: public Shader {
public:
	Color color;
	Texture* texture;
	Lambert() { color.makeZero(); texture = NULL; }
	Color shade(const Ray& ray, const IntersectionInfo& info);	
};

class Phong: public Shader {
public:
	Color color;
	double specularExponent;
	double specularMultiplier;
	Texture* texture;
	Phong(Color color = Color(0.5f, 0.5f, 0.5f), double specularExponent = 10, double specularMultiplier = 0.4, Texture* texture = NULL):
		color(color),
		specularExponent(specularExponent),
		specularMultiplier(specularMultiplier),
		texture(texture) {}
	Color shade(const Ray& ray, const IntersectionInfo& info);	
};

class Refl: public Shader {
public:
	double multiplier;
	double glossiness;
	int numSamples;
	Refl(double mult = 0.99, double glossiness = 1.0, int numSamples = 32): 
			multiplier(mult), glossiness(glossiness), numSamples(numSamples) {}
	Color shade(const Ray& ray, const IntersectionInfo& info);	
	
};

class Refr: public Shader {
public:
	double ior_ratio;
	double multiplier;
	Refr(double ior, double mult = 0.99): ior_ratio(ior), multiplier(mult) {}
	Color shade(const Ray& ray, const IntersectionInfo& info);	
};

class Layered: public Shader {
	struct Layer {
		Shader* shader;
		Color blend;
		Texture* tex;
	};
	Layer layers[32];
	int numLayers;
public:
	Layered() { numLayers = 0; }
	void addLayer(Shader* shader, Color blend, Texture* tex = NULL);
	Color shade(const Ray& ray, const IntersectionInfo& info);		
};

class Fresnel: public Texture {
	double ior;
public:
	Fresnel(double ior): ior(ior) {}
	Color sample(const IntersectionInfo& info);
};

#endif // __SHADING_H__
