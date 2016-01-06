
#include "lights.h"
#include "util.h"
#include "random_generator.h"

void RectLight::beginFrame(void)
{
	center = T.point(Vector(0, 0, 0));
	Vector a = T.point(Vector(-0.5, 0.0, -0.5));
	Vector b = T.point(Vector( 0.5, 0.0, -0.5));
	Vector c = T.point(Vector( 0.5, 0.0,  0.5));
	float width = (float) (b - a).length();
	float height = (float) (b - c).length();
	area = width * height; // obtain the area of the light, in world space
}


int RectLight::getNumSamples()
{
	return xSubd * ySubd;
}

void RectLight::getNthSample(int sampleIdx, const Vector& shadePos,
						  Vector& samplePos, Color& color)
{
	Random& rnd = getRandomGen();
	double x = (sampleIdx % xSubd + rnd.randfloat()) / xSubd;
	double y = (sampleIdx / xSubd + rnd.randfloat()) / ySubd;
	
	samplePos = Vector(x - 0.5, 0, y - 0.5);
	
	Vector shadePos_LS = T.undoPoint(shadePos);
	
	if (shadePos_LS.y < 0) {
		float cosWeight = float(dot(Vector(0, -1, 0), shadePos_LS) / shadePos_LS.length());
		color = this->color * power * area * cosWeight;
	} else {
		color.makeZero();
	}
	
	samplePos = T.point(samplePos);
}
	
bool RectLight::intersect(const Ray& ray, double& intersectionDist)
{
	Ray ray_LS = T.undoRay(ray);
	// check if ray_LS (the incoming ray, transformed in local space) hits the oriented square 1x1, resting
	// at (0, 0, 0), pointing downwards:
	if (ray_LS.start.y >= 0) return false; // ray start is in the wrong subspace; no intersection is possible
	if (ray_LS.dir.y <= 0) return false; // ray direction points downwards; no intersection is possible
	double lengthToIntersection = -(ray_LS.start.y / ray_LS.dir.y); // intersect with XZ plane
	Vector p = ray_LS.start + ray_LS.dir * lengthToIntersection;
	if (fabs(p.x) < 0.5 && fabs(p.z) < 0.5) {
		// the hit point is inside the 1x1 square - calculate the length to the intersection:
		double distance = (T.point(p) - ray.start).length(); 
		
		if (distance < intersectionDist) {
			intersectionDist = distance;
			return true; // intersection found, and it improves the current closest dist
		}
	}
	return false;
}

float RectLight::solidAngle(const Vector& x)
{
	Vector x_canonic = T.undoPoint(x);
	if (x_canonic.y >= 0) return 0;
	Vector x_dir = normalize(x_canonic);
	float cosA = dot(x_dir, Vector(0, -1, 0));
	double d = (x - center).lengthSqr();
	return area * cosA / (1 + d);
}

