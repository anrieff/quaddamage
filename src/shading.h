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

#include "color.h"
#include "bitmap.h"
#include "geometry.h"
#include "scene.h"

class BRDF {
public:
	virtual Color eval(const IntersectionInfo& x, const Vector& w_in, const Vector& w_out);
	
	virtual void spawnRay(const IntersectionInfo& x, const Vector& w_in,
							Ray& w_out, Color& color, float& pdf);
};

class Shader: public SceneElement, public BRDF {
public:
	virtual Color shade(const Ray& ray, const IntersectionInfo& info) = 0;
	virtual ~Shader() {}
	
	ElementType getElementType() const { return ELEM_SHADER; }
};

class Texture: public SceneElement {
public:
	virtual Color sample(const IntersectionInfo& info) = 0;
	virtual void modifyNormal(IntersectionInfo& info) {}
	virtual ~Texture() {}
	
	ElementType getElementType() const { return ELEM_TEXTURE; }
};

class CheckerTexture: public Texture {
public:
	Color color1, color2;
	double scaling;
	CheckerTexture() { color1.makeZero(); color2.makeZero(); scaling = 1; }
	virtual Color sample(const IntersectionInfo& info);

	void fillProperties(ParsedBlock& pb)
	{
		pb.getColorProp("color1", &color1);
		pb.getColorProp("color2", &color2);
		pb.getDoubleProp("scaling", &scaling);
	}
};

class Bitmap;
class BitmapTexture: public Texture {
	Bitmap* bitmap;
	double scaling;
	float assumedGamma;
public:
	BitmapTexture();
	~BitmapTexture();
	Color sample(const IntersectionInfo& info);
	void fillProperties(ParsedBlock& pb)
	{
		pb.getDoubleProp("scaling", &scaling);
		scaling = 1/scaling;
		if (!pb.getBitmapFileProp("file", *bitmap))
			pb.requiredProp("file");
		pb.getFloatProp("assumedGamma", &assumedGamma);
		
		if (assumedGamma != 1) {
			if (assumedGamma == 2.2f)
				bitmap->decompressGamma_sRGB();
			else if (assumedGamma > 0 && assumedGamma < 10)
				bitmap->decompressGamma(assumedGamma);
		}
	}
};

class BumpTexture: public Texture {
	Bitmap* bitmap;
	double strength, scaling;
public:
	BumpTexture();
	~BumpTexture();
	Color sample(const IntersectionInfo& info) {return Color(0, 0, 0);}
	void modifyNormal(IntersectionInfo& info);
	void beginRender();
	void fillProperties(ParsedBlock& pb)
	{
		pb.getDoubleProp("strength", &strength);
		pb.getDoubleProp("scaling", &scaling);
		if (!pb.getBitmapFileProp("file", *bitmap))
			pb.requiredProp("file");
	}
};


class Lambert: public Shader {
public:
	Color color;
	Texture* texture;
	Lambert() { color.makeZero(); texture = NULL; }
	Color shade(const Ray& ray, const IntersectionInfo& info);
	Color eval(const IntersectionInfo& x, const Vector& w_in, const Vector& w_out);
	void spawnRay(const IntersectionInfo& x, const Vector& w_in,
							Ray& w_out, Color& color, float& pdf);
	
	void fillProperties(ParsedBlock& pb)
	{
		pb.getColorProp("color", &color);
		pb.getTextureProp("texture", &texture);
	}
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
	void fillProperties(ParsedBlock& pb)
	{
		pb.getColorProp("color", &color);
		pb.getTextureProp("texture", &texture);
		pb.getDoubleProp("specularExponent", &specularExponent);
		pb.getDoubleProp("specularMultiplier", &specularMultiplier);
	}
};

class Refl: public Shader {
public:
	double multiplier;
	double glossiness;
	int numSamples;
	Refl(double mult = 0.99, double glossiness = 1.0, int numSamples = 32): 
			multiplier(mult), glossiness(glossiness), numSamples(numSamples) {}
	Color shade(const Ray& ray, const IntersectionInfo& info);	
	Color eval(const IntersectionInfo& x, const Vector& w_in, const Vector& w_out);
	void spawnRay(const IntersectionInfo& x, const Vector& w_in,
							Ray& w_out, Color& color, float& pdf);
	void fillProperties(ParsedBlock& pb)
	{
		pb.getDoubleProp("multiplier", &multiplier);
		pb.getDoubleProp("glossiness", &glossiness, 0, 1);
		pb.getIntProp("numSamples", &numSamples, 1);
	}
	
};

class Refr: public Shader {
public:
	double ior;
	double multiplier;
	Refr(double ior = 1.33, double mult = 0.99): ior(ior), multiplier(mult) {}
	Color shade(const Ray& ray, const IntersectionInfo& info);	
	Color eval(const IntersectionInfo& x, const Vector& w_in, const Vector& w_out);
	void spawnRay(const IntersectionInfo& x, const Vector& w_in,
							Ray& w_out, Color& color, float& pdf);
	void fillProperties(ParsedBlock& pb)
	{
		pb.getDoubleProp("multiplier", &multiplier);
		pb.getDoubleProp("ior", &ior, 1e-6, 10);
	}
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
	void fillProperties(ParsedBlock& pb);
};

class Fresnel: public Texture {
	double ior;
public:
	Fresnel(double ior = 1.33): ior(ior) {}
	Color sample(const IntersectionInfo& info);
	void fillProperties(ParsedBlock& pb)
	{
		pb.getDoubleProp("ior", &ior, 1e-6, 10);
	}
};

class Const: public Shader {
	Color color;
public:
	Const(Color color = Color(0.5f, 0.5f, 0.5f)): color(color) {}
	Color shade(const Ray& ray, const IntersectionInfo& info) { return color; }
	void fillProperties(ParsedBlock& pb)
	{
		pb.getColorProp("color", &color);
	}
};

// a texture that generates a slight random bumps on any geometry, which computes dNdx, dNdy
class Bumps: public Texture {
	float strength;
public:
	Bumps() { strength = 0; }
	
	void modifyNormal(IntersectionInfo& data);
	Color sample(const IntersectionInfo& info); // never called
	void fillProperties(ParsedBlock& pb)
	{
		pb.getFloatProp("strength", &strength);
	}
	
};

#endif // __SHADING_H__
