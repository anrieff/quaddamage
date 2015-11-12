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
#include "environment.h"
using std::vector;


Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

Camera camera;
vector<Node> nodes;
Plane plane;
CheckerTexture checker;
CheckerTexture ceilingTex;
Plane plane2;
Sphere s1;
Cube cube;
CheckerTexture blue;
Lambert ceiling;
Phong ball;
Lambert pod;
Vector lightPos;
double lightIntensity;
Color ambientLight;
bool wantAA = true;
Environment* environment;
int maxRaytraceDepth = 10;

void setupScene()
{
	ambientLight = Color(1, 1, 1) * 0.1;
	camera.position = Vector(0, 60, -120);
	camera.yaw = 0;
	camera.pitch = -30;
	camera.roll = 0;
	camera.fov = 90;
	camera.aspectRatio = float(frameWidth()) / float(frameHeight());
	plane.y = 1;
	plane.limit = 100;
	plane2.y = 200;
	checker.color1 = Color(0, 0, 0.5);
	checker.color2 = Color(1, 0.5, 0);
	ceilingTex.color1 = Color(0.5, 0.5, 0.5);
	ceilingTex.color2 = Color(0.5, 0.5, 0.5);
	Texture* plochki = new BitmapTexture("data/floor.bmp", 100);
	pod.texture = plochki;
	
	Layered* layeredPod = new Layered;
	layeredPod->addLayer(&pod, Color(1, 1, 1));
	layeredPod->addLayer(new Refl(0.9), Color(1, 1, 1) * 0.02);
	
	ceiling.texture = &ceilingTex;
	nodes.push_back({ &plane, layeredPod });
	//nodes.push_back({ &plane2, &ceiling });
	lightPos = Vector(120, 180, 0);
	lightIntensity = 45000.0;
	
	// sphere:
	s1.O = Vector(0, 30, -30);
	s1.R = 27;
	cube.O = Vector(0, 6, -30);
	cube.halfSide = 15;
	CsgOp* csg = new CsgMinus;
	csg->left = &cube;
	csg->right = &s1;
	blue.color1 = Color(0.2, 0.4, 1.0);
	blue.color2 = Color(0.4, 0.4, 0.4);
	blue.scaling = 2;
	
	ball.texture = new BitmapTexture("data/world.bmp");
	ball.specularExponent = 200;
	ball.specularMultiplier = 0.5;
	
	Layered* glass = new Layered;
	const double IOR_GLASS = 1.6;
	glass->addLayer(new Refr(IOR_GLASS, 0.9), Color(1, 1, 1));
	glass->addLayer(new Refl(0.9), Color(1, 1, 1), new Fresnel(IOR_GLASS));
	
	nodes.push_back({ &s1, glass });
	
	environment = new CubemapEnvironment("data/env/forest");
	
	camera.frameBegin();
}

Color raytrace(Ray ray)
{
	if (ray.depth > maxRaytraceDepth) return Color(0, 0, 0);
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
	if (closestNode == NULL) {
		if (environment) return environment->getEnvironment(ray.dir);
		else return Color(0, 0, 0);
	} else {
		closestInfo.rayDir = ray.dir;
		return closestNode->shader->shade(ray, closestInfo);
	}
}

bool visibilityCheck(const Vector& start, const Vector& end)
{
	Ray ray;
	ray.start = start;
	ray.dir = end - start;
	ray.dir.normalize();
	
	double targetDist = (end - start).length();
	
	for (auto& node: nodes) {
		IntersectionInfo info;
		if (!node.geom->intersect(ray, info)) continue;
		
		if (info.distance < targetDist) {
			return false;
		}
	}
	return true;
}

void render()
{
	const double kernel[5][2] = {
		{ 0.0, 0.0 },
		{ 0.6, 0.0 },
		{ 0.0, 0.6 },
		{ 0.3, 0.3 },
		{ 0.6, 0.6 },
	};
	Uint32 lastTicks = SDL_GetTicks();
	for (int y = 0; y < frameHeight(); y++) {
		for (int x = 0; x < frameWidth(); x++) {
			if (wantAA) {
				Color sum(0, 0, 0);
				for (int i = 0; i < COUNT_OF(kernel); i++)
					sum += raytrace(camera.getScreenRay(x + kernel[i][0], y + kernel[i][1]));
				vfb[y][x] = sum / double(COUNT_OF(kernel));
			} else {
				Ray ray = camera.getScreenRay(x, y);
				vfb[y][x] = raytrace(ray);
			}
		}
		displayVFB(vfb);
	}
}

int main ( int argc, char** argv )
{
	initGraphics(RESX, RESY);
	setupScene();
	Uint32 startTicks = SDL_GetTicks();
	render();
	Uint32 elapsedMs = SDL_GetTicks() - startTicks;
	printf("Render took %.2fs\n", elapsedMs / 1000.0f);
	displayVFB(vfb);
	waitForUserExit();
	closeGraphics();
	printf("Exited cleanly\n");
	return 0;
}
