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
#include "random_generator.h"
#include "scene.h"
#include "lights.h"
using std::vector;


Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];

Color raytrace(Ray ray)
{
	if (ray.depth > scene.settings.maxTraceDepth) return Color(0, 0, 0);
	Node* closestNode = NULL;
	double closestDist = INF;
	IntersectionInfo closestInfo;
	for (auto& node: scene.nodes) {
		IntersectionInfo info;
		if (!node->intersect(ray, info)) continue;
		
		if (info.distance < closestDist) {
			closestDist = info.distance;
			closestNode = node;
			closestInfo = info;
		}
	}
	// check if the closest intersection point is actually a light:
	bool hitLight = false;
	Color hitLightColor;
	for (auto& light: scene.lights) {
		if (light->intersect(ray, closestDist)) {
			hitLight = true;
			hitLightColor = light->getColor();
		}
	}
	if (hitLight) return hitLightColor;

	// check if we hit the sky:
	if (closestNode == NULL) {
		if (scene.environment) return scene.environment->getEnvironment(ray.dir);
		else return Color(0, 0, 0);
	} else {
		if (ray.debug && ray.depth == 0)
			printf("Found intersection at %.2lf\n", closestInfo.distance);
		closestInfo.rayDir = ray.dir;
		if (closestNode->bump)
			closestNode->bump->modifyNormal(closestInfo);
		return closestNode->shader->shade(ray, closestInfo);
	}
}

Color pathtrace(Ray ray, const Color& pathMultiplier, Random& rnd)
{
	if (ray.depth > scene.settings.maxTraceDepth) return Color(0, 0, 0);
	if (pathMultiplier.intensity() < 0.001f) return Color(0, 0, 0);
	Node* closestNode = NULL;
	double closestDist = INF;
	IntersectionInfo closestInfo;
	for (auto& node: scene.nodes) {
		IntersectionInfo info;
		if (!node->intersect(ray, info)) continue;
		
		if (info.distance < closestDist) {
			closestDist = info.distance;
			closestNode = node;
			closestInfo = info;
		}
	}
	// check if the closest intersection point is actually a light:
	bool hitLight = false;
	Color hitLightColor;
	for (auto& light: scene.lights) {
		if (light->intersect(ray, closestDist)) {
			hitLight = true;
			hitLightColor = light->getColor();
		}
	}
	if (hitLight) {
		return hitLightColor * pathMultiplier;
	}

	// check if we hit the sky:
	if (closestNode == NULL) {
		if (scene.environment)
			return scene.environment->getEnvironment(ray.dir) * pathMultiplier;
		else return Color(0, 0, 0);
	} 
	
	closestInfo.rayDir = ray.dir;
	if (closestNode->bump)
		closestNode->bump->modifyNormal(closestInfo);
	
	Vector w_out;
	Color brdf;
	float pdf;
	closestNode->shader->spawnRay(closestInfo, ray.dir, w_out, brdf, pdf);
	
	Ray newRay = ray;
	newRay.depth++;
	newRay.dir = w_out;
	newRay.start = closestInfo.ip + w_out * 1e-6;
	
	return pathtrace(newRay, pathMultiplier * brdf / pdf, rnd);
	
	//return closestNode->shader->shade(ray, closestInfo);
}


bool visibilityCheck(const Vector& start, const Vector& end)
{
	Ray ray;
	ray.start = start;
	ray.dir = end - start;
	ray.dir.normalize();
	
	double targetDist = (end - start).length();
	
	for (auto& node: scene.nodes) {
		IntersectionInfo info;
		if (!node->intersect(ray, info)) continue;
		
		if (info.distance < targetDist) {
			return false;
		}
	}
	return true;
}

void debugRayTrace(int x, int y)
{
	Ray ray = scene.camera->getScreenRay(x, y);
	ray.debug = true;
	raytrace(ray);
}

Color raytraceSinglePixel(double x, double y)
{
	auto getRay = scene.camera->dof ? 
		[](double x, double y, int whichCamera) {
			return scene.camera->getDOFRay(x, y, whichCamera);
		} :
		[](double x, double y, int whichCamera) {
			return scene.camera->getScreenRay(x, y, whichCamera);
		};
	
	auto trace = scene.settings.gi ? 
		[](const Ray& ray) { 
			Random& rnd = getRandomGen();
			return pathtrace(ray, Color(1, 1, 1), rnd); 
		} :
		[](const Ray& ray) { 
			return raytrace(ray); 
		};
		
	if (scene.camera->stereoSeparation > 0) {
		Ray leftRay = getRay(x, y, CAMERA_LEFT);
		Ray rightRay= getRay(x, y, CAMERA_RIGHT);
		Color colorLeft = trace(leftRay);
		Color colorRight = trace(rightRay);
		if (scene.settings.saturation != 1) {
			colorLeft.adjustSaturation(scene.settings.saturation);
			colorRight.adjustSaturation(scene.settings.saturation);
		
		}
		return  colorLeft * scene.camera->leftMask 
		      + colorRight* scene.camera->rightMask;
	} else {
		Ray ray = getRay(x, y, CAMERA_CENTRAL);
		return trace(ray);
	}
}

Color renderAAPixel(int x, int y)
{
	const double kernel[5][2] = {
		{ 0.0, 0.0 },
		{ 0.6, 0.0 },
		{ 0.0, 0.6 },
		{ 0.3, 0.3 },
		{ 0.6, 0.6 },
	};
	Color sum(0, 0, 0);
	for (int i = 0; i < COUNT_OF(kernel); i++)
		sum += raytraceSinglePixel(x + kernel[i][0], y + kernel[i][1]);
	return sum / double(COUNT_OF(kernel));
}

Color renderDOFPixel(int x, int y)
{
	Random& rnd = getRandomGen();
	Color sum(0, 0, 0);
	for (int i = 0; i < scene.camera->numSamples; i++) {
		sum += raytraceSinglePixel(x + rnd.randdouble(), y + rnd.randdouble());
	}
	return sum / scene.camera->numSamples;
}

Color renderGIPixel(int x, int y)
{
	Color sum(0, 0, 0);
	int N = scene.settings.numPaths;
	
	Ray ray = scene.camera->getScreenRay(x, y);
	Random rnd = getRandomGen();
	for (int i = 0; i < N; i++) {
		sum += pathtrace(ray, Color(1, 1, 1), rnd); 
	}
	
	return sum / N;
}

Color renderPixel(int x, int y)
{
	if (scene.camera->dof) {
		return renderDOFPixel(x, y);
	} else if (scene.settings.gi) {
		return renderGIPixel(x, y);
	} else {
		if (scene.settings.wantAA) {
			return renderAAPixel(x, y);
		} else {
			return raytraceSinglePixel(x, y);
		}
	}
}

void render()
{
	scene.beginFrame();
	vector<Rect> buckets = getBucketsList();
	
	if (scene.settings.wantPrepass || scene.settings.gi) {
		// We render the whole screen in three passes.
		// 1) First pass - use very coarse resolution rendering, tracing a single ray for a 16x16 block:
		for (Rect& r: buckets) {
			for (int dy = 0; dy < r.h; dy += 16) {
				int ey = min(r.h, dy + 16);
				for (int dx = 0; dx < r.w; dx += 16) {
					int ex = min(r.w, dx + 16);
					Color c = raytraceSinglePixel(r.x0 + dx + ex / 2, r.y0 + dy + ey / 2);
					if (!drawRect(Rect(r.x0 + dx, r.y0 + dy, r.x0 + ex, r.y0 + ey), c))
						return;
				}
			}
		}
	}
	
	for (Rect& r: buckets) {
		for (int y = r.y0; y < r.y1; y++)
			for (int x = r.x0; x < r.x1; x++) {
				vfb[y][x] = renderPixel(x, y);
			}
		if (!displayVFBRect(r, vfb)) return;
	}
}

int renderSceneThread(void* /*unused*/)
{
	render();
	rendering = false;
	return 0;
}

const char* DEFAULT_SCENE = "data/smallpt.qdmg";

int main ( int argc, char** argv )
{
	initRandom(42);
	Color::init_sRGB_cache();
	const char* sceneFile = argc == 2 ? argv[1] : DEFAULT_SCENE;
	if (!scene.parseScene(sceneFile)) {
		printf("Could not parse the scene!\n");
		return -1;
	}
	
	initGraphics(scene.settings.frameWidth, scene.settings.frameHeight);
	setWindowCaption("Quad Damage: preparing...");
	
	scene.beginRender();
	
	setWindowCaption("Quad Damage: rendering...");
	Uint32 startTicks = SDL_GetTicks();
	renderScene_threaded();
	Uint32 elapsedMs = SDL_GetTicks() - startTicks;
	printf("Render took %.2fs\n", elapsedMs / 1000.0f);
	setWindowCaption("Quad Damage: rendered in %.2fs\n", elapsedMs / 1000.0f);
	
	displayVFB(vfb);
	waitForUserExit();
	closeGraphics();
	printf("Exited cleanly\n");
	return 0;
}
