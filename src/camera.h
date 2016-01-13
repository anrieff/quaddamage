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
 * @File camera.h
 * @Brief Contains declaration of the raytracing camera.
 */
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "vector.h"
#include "matrix.h"
#include "scene.h"

enum {
	CAMERA_CENTRAL,
	CAMERA_LEFT,
	CAMERA_RIGHT,
};

class Camera: public SceneElement {
	Vector topLeft, topRight, bottomLeft;
	Matrix rotation;
	double apertureSize;
	Vector frontDir, upDir, rightDir;
public:
	Vector position;
	double yaw, pitch, roll; //!< in degrees
	double aspectRatio;
	double fov;              //!< in degrees
	bool dof;
	double fNumber;
	double focalPlaneDist;
	int numSamples;
	bool autofocus;
	double stereoSeparation;
	Color leftMask, rightMask;
	
	Camera() {
		position.makeZero();
		yaw = pitch = roll = 0;
		aspectRatio = 4./3.; 
		fov = 90;
		dof = false;
		fNumber = 2.0;
		numSamples = 32;
		focalPlaneDist = 100;
		autofocus = false;
		stereoSeparation = 0.0;
		leftMask = Color(1, 0, 0);
		rightMask = Color(0, 1, 1);
	}
	
	void fillProperties(ParsedBlock& pb)
	{
		if (!pb.getVectorProp("position", &position))
			pb.requiredProp("pos");
		pb.getDoubleProp("aspectRatio", &aspectRatio, 1e-6);
		pb.getDoubleProp("fov", &fov, 0.0001, 179);
		pb.getDoubleProp("yaw", &yaw);
		pb.getDoubleProp("pitch", &pitch, -90, 90);
		pb.getDoubleProp("roll", &roll);
		pb.getBoolProp("dof", &dof);
		pb.getDoubleProp("fNumber", &fNumber, 0);
		pb.getIntProp("numSamples", &numSamples, 1);
		pb.getDoubleProp("focalPlaneDist", &focalPlaneDist, 0.1);
		pb.getBoolProp("autofocus", &autofocus);
		pb.getDoubleProp("stereoSeparation", &stereoSeparation, 0.0);
		pb.getColorProp("leftMask", &leftMask);
		pb.getColorProp("rightMask", &rightMask);
		
		apertureSize = 4.5 / fNumber;
	}
	
	ElementType getElementType() const { return ELEM_CAMERA; }
	void beginFrame();
	
	Ray getScreenRay(double xScreen, double yScreen, int whichCamera = CAMERA_CENTRAL);
	Ray getDOFRay(double xScreen, double yScreen, int whichCamera = CAMERA_CENTRAL);
	
	void move(double sideways, double front_back);
	void rotate(double yawDiff, double pitchDiff);

};

#endif // __CAMERA_H__
