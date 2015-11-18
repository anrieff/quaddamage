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
#include "mesh.h"
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

	camera.yaw = 5;
	camera.pitch = -5;
	camera.roll = 0;
	camera.fov = 60;
	camera.aspectRatio = 4. / 3.0;
	camera.position = Vector(45, 120, -300);
	
	camera.frameBegin();
	
	lightPos = Vector(-90, 1200, -750);
	lightIntensity = 1200000;
	ambientLight = Color(0.5, 0.5, 0.5);
	
	/* Create a floor node, with a layered shader: perfect reflection on top of woody diffuse */
	Plane* plane = new Plane;
	plane->limit = 200;
	Texture* texture = new BitmapTexture("data/texture/wood.bmp", 100);
	Lambert* lambert = new Lambert;
	lambert->texture = texture;
	Layered* planeShader = new Layered;
	planeShader->addLayer(lambert, Color(1, 1, 1));
	planeShader->addLayer(new Refl, Color(0.05, 0.05, 0.05), new Fresnel(1.33));
	nodes.push_back({plane, planeShader});
	
	
	Mesh* mesh = new Mesh(false);
	mesh->setFaceted(false);
	CheckerTexture* checker = new CheckerTexture;
	checker->color1 = Color(0.7, 0.7, 0.7);
	checker->color2 = Color(0.75, 0.15, 0.15);
	
	mesh->translate(Vector(-100, 50, 0));
	mesh->computeBoundingGeometry();
	Lambert* meshShader = new Lambert;
	meshShader->color = Color(1, 1, 1) * 0.75;
	meshShader->texture = checker;
	nodes.push_back({mesh, meshShader});
	
	/* Create a glossy sphere */
	Sphere* sphere = new Sphere;
	sphere->O = Vector(100, 50, 60);
	sphere->R = 50;
	Shader* glossy = new Refl(0.9, 0.97, 25);
	
	nodes.push_back({sphere, glossy});
	
	Color colors[3] = { Color(1, 0, 0), Color(1, 1, 0), Color(0, 1, 0) };
	// desaturate a bit:
	for (int i = 0; i < 3; i++) colors[i].adjustSaturation(0.9f);
	for (int i = 0; i < 3; i++) {
		nodes.push_back({new Sphere(Vector(10 + 32*i, 15, 0), 15.0), new Phong(colors[i]*0.75, 32) });
	}
		
	environment = new CubemapEnvironment("data/env/forest");
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

void debugRayTrace(int x, int y)
{
	Ray ray = camera.getScreenRay(x, y);
	ray.debug = true;
	raytrace(ray);
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
		if (SDL_GetTicks() - lastTicks > 100) {
			displayVFB(vfb);
			lastTicks = SDL_GetTicks();
		}
	}
}

int main ( int argc, char** argv )
{
	initGraphics(RESX, RESY);
	setWindowCaption("Quad Damage: rendering...");
	setupScene();
	Uint32 startTicks = SDL_GetTicks();
	render();
	Uint32 elapsedMs = SDL_GetTicks() - startTicks;
	printf("Render took %.2fs\n", elapsedMs / 1000.0f);
	setWindowCaption("Quad Damage: rendered in %.2fs\n", elapsedMs / 1000.0f);
	displayVFB(vfb);
	waitForUserExit();
	closeGraphics();
	printf("Exited cleanly\n");
	return 0;
}
