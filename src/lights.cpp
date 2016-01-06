
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
	

