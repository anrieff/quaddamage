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
 * @File camera.cpp
 * @Brief Implementation of the raytracing camera.
 */
#include "camera.h"
#include "matrix.h"
#include "util.h"
#include "sdl.h"

void Camera::beginFrame()
{	
	double x2d = aspectRatio, y2d = +1;
	
	double wantedAngle = toRadians(fov/2);
	double wantedLength = tan(wantedAngle);
	double hypotLength = sqrt(sqr(aspectRatio) + sqr(1.0));
	double scaleFactor = wantedLength / hypotLength;
	
	x2d *= scaleFactor;
	y2d *= scaleFactor;
	
	topLeft = Vector(-x2d, y2d, 1);
	topRight = Vector(x2d, y2d, 1);
	bottomLeft = Vector(-x2d, -y2d, 1);
	
	rotation = 
		rotationAroundZ(toRadians(roll)) *
		rotationAroundX(toRadians(pitch)) *
		rotationAroundY(toRadians(yaw));
	
	topLeft *= rotation;
	topRight *= rotation;
	bottomLeft *= rotation;
	
	topLeft += this->position;
	topRight += this->position;
	bottomLeft += this->position;
}

Ray Camera::getScreenRay(double xScreen, double yScreen)
{
	Vector throughPoint = 
		topLeft + (topRight - topLeft) * (xScreen / frameWidth())
				+ (bottomLeft - topLeft) * (yScreen / frameHeight());
	
	Ray ray;
	ray.dir = throughPoint - this->position;
	ray.dir.normalize();
	ray.start = this->position;
	return ray;
}
