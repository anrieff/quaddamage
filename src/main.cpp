#include <SDL/SDL.h>
#include <math.h>
#include <vector>
#include "vector.h"
#include "util.h"
#include "sdl.h"
#include "color.h"
#include "camera.h"
#include "geometry.h"
#include "shading.h"
using std::vector;


Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

Camera camera;
vector<Node> nodes;
Plane plane;
CheckerShader checker;
CheckerShader ceiling;
Plane plane2;
Vector lightPos;
double lightIntensity;

void setupScene()
{
	camera.position = Vector(35, 60, -100);
	camera.yaw = 0;
	camera.pitch = -30;
	camera.roll = 0;
	camera.fov = 90;
	camera.aspectRatio = float(frameWidth()) / float(frameHeight());
	plane.y = 1;
	plane2.y = 200;
	checker.color1 = Color(0, 0, 0.5);
	checker.color2 = Color(1, 0.5, 0);
	ceiling.color1 = Color(0.5, 0.5, 0.5);
	ceiling.color2 = Color(0.5, 0.5, 0.5);
	nodes.push_back({ &plane, &checker });
	nodes.push_back({ &plane2, &ceiling });
	lightPos = Vector(35, 180, 200);
	lightIntensity = 25000.0;
	
	camera.frameBegin();
}

Color raytrace(Ray ray)
{
	Node* closestNode = NULL;
	double closestDist = INF;
	IntersectionInfo closestInfo;
	for (auto& node: nodes) {
		IntersectionInfo info;
		if (!node.geom->intersect(ray, info)) continue;
		
		if (info.distance < closestDist) {
			closestDist = info.distance;
			closestNode = &node;
			closestInfo = info;
		}
	}
	// check if we hit the sky:
	if (closestNode == NULL)
		return Color(0, 0, 0); // TODO(vesko): return background color
	else
		return closestNode->shader->shade(ray, closestInfo);
}

void render()
{
	for (int y = 0; y < frameHeight(); y++)
		for (int x = 0; x < frameWidth(); x++) {
			Ray ray = camera.getScreenRay(x, y);
			vfb[y][x] = raytrace(ray);
		}
}

int main ( int argc, char** argv )
{
	initGraphics(RESX, RESY);
	setupScene();
	render();
	displayVFB(vfb);
	waitForUserExit();
	closeGraphics();
	printf("Exited cleanly\n");
	return 0;
}
