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
#include "shading.h"
#include "bitmap.h"

bool visibilityCheck(const Vector& start, const Vector& end);

extern Vector lightPos;
extern double lightIntensity;
extern Color ambientLight;

Color CheckerTexture::sample(const IntersectionInfo& info)
{
	int x = (int) floor(info.u * scaling / 5.0);
	int y = (int) floor(info.v * scaling / 5.0);
	
	Color checkerColor = ((x + y) % 2 == 0) ? color1 : color2;
	return checkerColor;
}

double getLightContrib(const IntersectionInfo& info)
{
	double distanceToLightSqr = (info.ip - lightPos).lengthSqr();

	if (!visibilityCheck(info.ip + info.normal * 1e-6, lightPos)) {
		return 0;
	} else {
		return lightIntensity / distanceToLightSqr;
	}
}

Color Lambert::shade(const Ray& ray, const IntersectionInfo& info)
{
	Color diffuse = texture ? texture->sample(info) : this->color;
	
	Vector v1 = faceforward(ray.dir, info.normal);
	Vector v2 = lightPos - info.ip;
	v2.normalize();
	double lambertCoeff = dot(v1, v2);
	
	return ambientLight * diffuse
		+ diffuse * lambertCoeff * getLightContrib(info);
	
}

Color Phong::shade(const Ray& ray, const IntersectionInfo& info)
{
	Color diffuse = texture ? texture->sample(info) : this->color;
	
	Vector v1 = faceforward(ray.dir, info.normal);
	Vector v2 = lightPos - info.ip;
	v2.normalize();
	double lambertCoeff = dot(v1, v2);
	double fromLight = getLightContrib(info);
	
	Vector r = reflect(info.ip - lightPos, info.normal);
	Vector toCamera = -ray.dir;
	double cosGamma = dot(toCamera, r);
	double phongCoeff;
	if (cosGamma > 0)
		phongCoeff = pow(cosGamma, specularExponent);
	else
		phongCoeff = 0;
	
	return ambientLight * diffuse
		+ diffuse * lambertCoeff * fromLight
		+ Color(1, 1, 1) * (phongCoeff * specularMultiplier * fromLight);
}

BitmapTexture::BitmapTexture(const char* filename, double scaling)
{
	bitmap = new Bitmap;
	bitmap->loadImage(filename);
	this->scaling = 1/scaling;
}

BitmapTexture::~BitmapTexture() { delete bitmap; }

Color BitmapTexture::sample(const IntersectionInfo& info)
{
	int x = (int) floor(info.u * scaling * bitmap->getWidth());
	int y = (int) floor(info.v * scaling * bitmap->getHeight());
	// 0 <= x < bitmap.width
	// 0 <= y < bitmap.height
	x = (x % bitmap->getWidth());
	y = (y % bitmap->getHeight());
	if (x < 0) x += bitmap->getWidth();
	if (y < 0) y += bitmap->getHeight();
	
	return bitmap->getPixel(x, y);
}

extern Color raytrace(Ray ray);

Color Refl::shade(const Ray& ray, const IntersectionInfo& info)
{
	Vector n = faceforward(ray.dir, info.normal);

	Ray newRay = ray;
	newRay.start = info.ip + n * 0.000001;
	newRay.dir = reflect(ray.dir, n);
	newRay.depth++; 
	
	return raytrace(newRay) * multiplier;
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
		refr = refract(ray.dir, info.normal, 1 / ior_ratio);
	} else {
		// leaving the geometry
		refr = refract(ray.dir, -info.normal, ior_ratio);
	}
	if (refr.lengthSqr() == 0) return Color(1, 0, 0);
	Ray newRay = ray;
	newRay.start = info.ip - faceforward(ray.dir, info.normal) * 0.000001;
	newRay.dir = refr;
	newRay.depth++;
	return raytrace(newRay) * multiplier;
}
