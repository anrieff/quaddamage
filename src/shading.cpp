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
 * @File shading.cpp
 * @Brief Contains implementations of shader classes
 */
#include <string.h>
#include "shading.h"
#include "bitmap.h"
#include "lights.h"
#include "random_generator.h"

bool visibilityCheck(const Vector& start, const Vector& end);

Color CheckerTexture::sample(const IntersectionInfo& info)
{
	int x = (int) floor(info.u * scaling / 5.0);
	int y = (int) floor(info.v * scaling / 5.0);
	
	Color checkerColor = ((x + y) % 2 == 0) ? color1 : color2;
	return checkerColor;
}

Color getLightContrib(const IntersectionInfo& info, const Vector& lightPos, const Color& lightColor)
{
	double distanceToLightSqr = (info.ip - lightPos).lengthSqr();

	if (!visibilityCheck(info.ip + info.normal * 1e-6, lightPos)) {
		return Color(0, 0, 0);
	} else {
		return lightColor / distanceToLightSqr;
	}
}

Color Lambert::shade(const Ray& ray, const IntersectionInfo& info)
{
	Color diffuse = texture ? texture->sample(info) : this->color;
	
	Color result(0, 0, 0);
	for (auto& light: scene.lights) {
		int N = light->getNumSamples();
		Color sum (0, 0, 0);
		for (int i = 0; i < N; i++) {
			Vector lightPos;
			Color lightColor;
			light->getNthSample(i, info.ip, lightPos, lightColor);
			Vector v2 = info.ip - lightPos; // from light towards the intersection point
			Vector v1 = faceforward(ray.dir, info.normal); // orient so that surface points to the light
			v2.normalize();
			double lambertCoeff = dot(v1, -v2);
			sum += diffuse * lambertCoeff * getLightContrib(info, lightPos, lightColor);
		}
		result += sum / N;
	}
	result += scene.settings.ambientLight * diffuse;
	return result;
	
}

Color Phong::shade(const Ray& ray, const IntersectionInfo& info)
{
	
	Color diffuse = texture ? texture->sample(info) : this->color;
	
	Color result(0, 0, 0);
	for (auto& light: scene.lights) {
		int N = light->getNumSamples();
		Color sum (0, 0, 0);
		for (int i = 0; i < N; i++) {
			Vector lightPos;
			Color lightColor;
			light->getNthSample(i, info.ip, lightPos, lightColor);
			Vector v2 = info.ip - lightPos; // from light towards the intersection point
			Vector v1 = faceforward(ray.dir, info.normal); // orient so that surface points to the light
			v2.normalize();
			double lambertCoeff = dot(v1, -v2);
			Color fromLight = getLightContrib(info, lightPos, lightColor);

			Vector r = reflect(v2, v1);
			Vector toCamera = -ray.dir;
			double cosGamma = dot(toCamera, r);
			double phongCoeff;
			if (cosGamma > 0)
				phongCoeff = pow(cosGamma, specularExponent);
			else
				phongCoeff = 0;
			
			sum += diffuse * lambertCoeff * fromLight
				  + (phongCoeff * specularMultiplier * fromLight);

		}
		result += sum / N;
	}
	result += scene.settings.ambientLight * diffuse;
	return result;
	
}


BitmapTexture::BitmapTexture()
{
	bitmap = new Bitmap();
	scaling = 1.0; 
	assumedGamma = 1;
}
BitmapTexture::~BitmapTexture() { delete bitmap; }

Color BitmapTexture::sample(const IntersectionInfo& info)
{
	float x = fmod(info.u * scaling * bitmap->getWidth(), bitmap->getWidth());
	float y = fmod(info.v * scaling * bitmap->getHeight(), bitmap->getHeight());
	if (x < 0) x += bitmap->getWidth();
	if (y < 0) y += bitmap->getHeight();
	
	// 0 <= x < bitmap.width
	// 0 <= y < bitmap.height
	if (x < 0) x += bitmap->getWidth();
	if (y < 0) y += bitmap->getHeight();
	
	return bitmap->getFilteredPixel(x, y);
}

extern Color raytrace(Ray ray);

Color Refl::shade(const Ray& ray, const IntersectionInfo& info)
{
	Vector n = faceforward(ray.dir, info.normal);

	if (glossiness == 1) {
		Ray newRay = ray;
		newRay.start = info.ip + n * 0.000001;
		newRay.dir = reflect(ray.dir, n);
		newRay.depth++; 
		
		return raytrace(newRay) * multiplier;
	} else {
		Random& rnd = getRandomGen();
		Color result(0, 0, 0);
		int count = numSamples;
		if (ray.depth > 0)
			count = 2;
		for (int i = 0; i < count; i++) {
			Vector a, b;
			orthonormalSystem(n, a, b);
			double x, y, scaling;
			
			rnd.unitDiscSample(x, y);
			//
			scaling = tan((1 - glossiness) * PI/2);
			x *= scaling;
			y *= scaling;
			
			Vector modifiedNormal = n + a * x + b * y;

			Ray newRay = ray;
			newRay.start = info.ip + n * 0.000001;
			newRay.dir = reflect(ray.dir, modifiedNormal);
			newRay.depth++; 
			
			result += raytrace(newRay) * multiplier;
		}
		return result / count;
	}
}

inline Vector refract(const Vector& i, const Vector& n, double ior)
{
	double NdotI = (double) (i * n);
	double k = 1 - (ior * ior) * (1 - NdotI * NdotI);
	if (k < 0.0)		// Check for total inner reflection
		return Vector(0, 0, 0);
	return ior * i - (ior * NdotI + sqrt(k)) * n;
}

Color Refr::shade(const Ray& ray, const IntersectionInfo& info)
{
// ior = eta2 / eta1
	Vector refr;
	if (dot(ray.dir, info.normal) < 0) {
		// entering the geometry
		refr = refract(ray.dir, info.normal, 1 / ior);
	} else {
		// leaving the geometry
		refr = refract(ray.dir, -info.normal, ior);
	}
	if (refr.lengthSqr() == 0) return Color(0, 0, 0);
	Ray newRay = ray;
	newRay.start = info.ip - faceforward(ray.dir, info.normal) * 0.000001;
	newRay.dir = refr;
	newRay.depth++;
	return raytrace(newRay) * multiplier;
}

void Layered::addLayer(Shader* shader, Color blend, Texture* tex)
{
	if (numLayers == COUNT_OF(layers)) return;
	layers[numLayers++] = { shader, blend, tex };
}

void Layered::fillProperties(ParsedBlock& pb)
{
	char name[128];
	char value[256];
	int srcLine;
	for (int i = 0; i < pb.getBlockLines(); i++) {
		// fetch and parse all lines like "layer <shader>, <color>[, <texture>]"
		pb.getBlockLine(i, srcLine, name, value);
		if (!strcmp(name, "layer")) {
			char shaderName[200];
			char textureName[200] = "";
			bool err = false;
			if (!getFrontToken(value, shaderName)) {
				err = true;
			} else {
				stripPunctuation(shaderName);
			}
			if (!strlen(value)) err = true;
			if (!err && value[strlen(value) - 1] != ')') {
				if (!getLastToken(value, textureName)) {
					err = true;
				} else {
					stripPunctuation(textureName);
				}
			}
			if (!err && !strcmp(textureName, "NULL")) strcpy(textureName, "");
			Shader* shader = NULL;
			Texture* texture = NULL;
			if (!err) {
				shader = pb.getParser().findShaderByName(shaderName);
				err = (shader == NULL);
			}
			if (!err && strlen(textureName)) {
				texture = pb.getParser().findTextureByName(textureName);
				err = (texture == NULL);
			}
			if (err) throw SyntaxError(srcLine, "Expected a line like `layer <shader>, <color>[, <texture>]'");
			double x, y, z;
			get3Doubles(srcLine, value, x, y, z);
			addLayer(shader, Color((float) x, (float) y, (float) z), texture);
		}
	}
}

Color Layered::shade(const Ray& ray, const IntersectionInfo& info)
{
	Color result(0, 0, 0);
	for (int i = 0; i < numLayers; i++) {
		Color fromLayer = layers[i].shader->shade(ray, info);
		Color blendAmount = layers[i].blend;
		if (layers[i].tex) blendAmount = blendAmount * layers[i].tex->sample(info);
		result = blendAmount * fromLayer + (Color(1, 1, 1) - blendAmount) * result;
	}
	return result;
}

inline double fresnel(const Vector& i, const Vector& n, double ior)
{
	// Schlick's approximation
	double f = sqr((1.0f - ior) / (1.0f + ior));
	double NdotI = (double) -dot(n, i);
	return f + (1.0f - f) * pow(1.0f - NdotI, 5.0f);
}

Color Fresnel::sample(const IntersectionInfo& info)
{
	double eta = ior;
	if (dot(info.normal, info.rayDir) > 0)
		eta = 1 / eta;
	Vector n = faceforward(info.rayDir, info.normal);
	double fr = fresnel(info.rayDir, n, eta);
	return Color(fr, fr, fr);
}

BumpTexture::BumpTexture()
{
	bitmap = new Bitmap();
	strength = 1.0;
	scaling = 1.0;
}
BumpTexture::~BumpTexture()
{
	delete bitmap;
}

void BumpTexture::modifyNormal(IntersectionInfo& info)
{
	float x = fmod(info.u * scaling * bitmap->getWidth(), bitmap->getWidth());
	float y = fmod(info.v * scaling * bitmap->getHeight(), bitmap->getHeight());
	if (x < 0) x += bitmap->getWidth();
	if (y < 0) y += bitmap->getHeight();
	
	Color bump = bitmap->getFilteredPixel(x, y);
	float dx = bump.r;
	float dy = bump.g;
	
	info.normal += (info.dNdx * dx + info.dNdy * dy) * strength;
	info.normal.normalize();
}

void BumpTexture::beginRender()
{
	bitmap->differentiate();
}

void Bumps::modifyNormal(IntersectionInfo& data)
{
	if (strength > 0) {
		float freqX[3] = { 0.5f, 1.21f, 1.9f }, freqZ[3] = { 0.4f, 1.13f, 1.81f };
		float fm = 0.2f;
		float intensityX[3] = { 0.1f, 0.08f, 0.05f }, intensityZ[3] = { 0.1f, 0.08f, 0.05f };
		double dx = 0, dy = 0;
		for (int i = 0; i < 3; i++) {
			dx += sin(fm * freqX[i] * data.u) * intensityX[i] * strength; 
			dy += sin(fm * freqZ[i] * data.v) * intensityZ[i] * strength;
		}
		data.normal += dx * data.dNdx + dy * data.dNdy;
		data.normal.normalize();
	}
}

Color Bumps::sample(const IntersectionInfo& info)
{
	// never called
	return Color(0, 0, 0);
}
