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
 * @File mesh.h
 * @Brief Contains the Mesh class.
 */
#ifndef __MESH_H__
#define __MESH_H__

#include <vector>
#include "geometry.h"
#include "vector.h"
#include "bbox.h"

struct KDTreeNode {
	Axis axis; // AXIS_NONE if this is a leaf node
	double splitPos;
	union {
		std::vector<int>* triangles;
		KDTreeNode* children;
	};
	//
	void initLeaf(const std::vector<int>& triangles)
	{
		axis = AXIS_NONE;
		this->triangles = new std::vector<int>(triangles);
	}
	
	void initTreeNode(Axis axis, double splitPos)
	{
		this->axis = axis;
		this->splitPos = splitPos;
		this->children = new KDTreeNode[2];
	}
	~KDTreeNode()
	{
		if (axis == AXIS_NONE)
			delete triangles;
		else
			delete [] children;
	}
};

class Mesh: public Geometry {
	std::vector<Vector> vertices;
	std::vector<Vector> normals;
	std::vector<Vector> uvs;
	std::vector<Triangle> triangles;
	BBox bbox;
	
	KDTreeNode* kdroot;
	bool useKDTree;
	int maxDepthSum;
	int numNodes;

	void computeBoundingGeometry();
	bool intersectTriangle(const Ray& ray, const Triangle& t, IntersectionInfo& info);
	void buildKD(KDTreeNode* node, BBox bbox, const std::vector<int>& triangleList, int depth);
	bool intersectKD(KDTreeNode* node, BBox bbox, const Ray& ray, IntersectionInfo& info);
public:
	
	bool faceted;
	bool backfaceCulling;

	Mesh() {
		faceted = false;
		useKDTree = true;
		backfaceCulling = true;
		kdroot = NULL;
	}
	~Mesh();
	
	bool loadFromOBJ(const char* filename);
	
	void fillProperties(ParsedBlock& pb)
	{
		pb.getBoolProp("faceted", &faceted);
		pb.getBoolProp("backfaceCulling", &backfaceCulling);
		char fn[256];
		if (pb.getFilenameProp("file", fn)) {
			if (!loadFromOBJ(fn)) {
				pb.signalError("Could not parse OBJ file!");
			}
			
		} else {
			pb.requiredProp("file");
		}
		pb.getBoolProp("useKDTree", &useKDTree);
	}
	
	void beginRender();
	
	bool intersect(const Ray& ray, IntersectionInfo& info);
};

#endif // __MESH_H__
