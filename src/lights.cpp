
#include "lights.h"
#include "util.h"

int RectLight::getNumSamples()
{
	return xSubd * ySubd;
}

void RectLight::getNthSample(int sampleIdx, const Vector& shadePos,
						  Vector& samplePos, Color& color)
{
	double x = (sampleIdx % xSubd + randomFloat()) / xSubd;
	double y = (sampleIdx / xSubd + randomFloat()) / ySubd;
	
	samplePos = Vector(x - 0.5, 0, y - 0.5);
	
	Vector shadePos_LS = T.undoPoint(shadePos);
	
	if (shadePos_LS.y < 0) color = this->color * power;
	else				   color.makeZero();
	
	samplePos = T.point(samplePos);
}
	

