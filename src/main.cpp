#include <SDL/SDL.h>
#include <SDL/SDL_events.h>
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
#include "cxxptl_sdl.h"

using std::vector;


Color vfb[VFB_MAX_SIZE][VFB_MAX_SIZE];
bool needsAA[VFB_MAX_SIZE][VFB_MAX_SIZE];

bool visibilityCheck(const Vector& start, const Vector& end);
ThreadPool pool;

Color raytrace(const Ray& ray)
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
		if ((ray.flags & RF_DEBUG) && ray.depth == 0)
			printf("Found intersection at %.2lf\n", closestInfo.distance);
		closestInfo.rayDir = ray.dir;
		if (closestNode->bump)
			closestNode->bump->modifyNormal(closestInfo);
		return closestNode->shader->shade(ray, closestInfo);
	}
}

Color explicitLightSample(const Ray& ray, const IntersectionInfo& info, const Color& pathMultiplier, Shader* shader, Random& rnd)
{
	// try to end a path by explicitly sampling a light. If there are no lights, we can't do that:
	if (scene.lights.empty()) return Color(0, 0, 0);
	
	// choose a random light:
	int lightIdx = rnd.randint(0, scene.lights.size() - 1);
	Light* chosenLight = scene.lights[lightIdx];
	
	// evaluate light's solid angle as viewed from the intersection point, x:
	Vector x = info.ip;
	double solidAngle = chosenLight->solidAngle(x);
	
	// is light is too small or invisible?
	if (solidAngle == 0) return Color(0, 0, 0);
	
	// choose a random point on the light:
	int samplesInLight = chosenLight->getNumSamples();
	int randSample = rnd.randint(0, samplesInLight - 1);
	
	Vector pointOnLight;
	Color unused;
	chosenLight->getNthSample(randSample, x, pointOnLight, unused);
	
	// camera -> ... path ... -> x -> lightPos
	//                       are x and lightPos visible?
	if (!visibilityCheck(x + info.normal * 1e-6, pointOnLight))
		return Color(0, 0, 0);
	
	// get the emitted light energy (color * power):
	Color L = chosenLight->getColor();
	
	
	// evaluate BRDF. It might be zero (e.g., pure reflection), so bail out early if that's the case
	Vector w_out = pointOnLight - x;
	w_out.normalize();
	Color brdfAtPoint = shader->eval(info, ray.dir, w_out);
	if (brdfAtPoint.intensity() == 0) return Color(0, 0, 0);
	
	// probability to hit this light's projection on the hemisphere
	// (conditional probability, since we're specifically aiming for this light):
	float probHitLightArea = 1.0f / solidAngle;
	
	// probability to pick this light out of all N lights:
	float probPickThisLight = 1.0f / scene.lights.size();
	
	// combined probability of this generated w_out ray:
	float chooseLightProb = probHitLightArea * probPickThisLight;
	
	/* Light flux (Li) */ /* BRDFs@path*/  /*last BRDF*/ /*MC probability*/
	return     L       *   pathMultiplier * brdfAtPoint / chooseLightProb;
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
		if (!(ray.flags & RF_DIFFUSE)) {
			// forbid light contributions after a diffuse reflection
			return hitLightColor * pathMultiplier;
		} else 
			return Color(0, 0, 0);
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
	
	// ("sampling the light"):
	// try to end the current path with explicit sampling of some light
	Color contribLight = explicitLightSample(ray, closestInfo, pathMultiplier, 
											closestNode->shader, rnd);
	// ("sampling the BRDF"):
	// also try to extend the current path randomly: 
	Ray w_out = ray;
	w_out.depth++;
	Color brdf;
	float pdf;
	closestNode->shader->spawnRay(closestInfo, ray.dir, w_out, brdf, pdf);
	
	if (pdf == -1) return Color(1, 0, 0); // BRDF not implemented
	if (pdf == 0) return Color(0, 0, 0);  // BRDF is zero
	
	
	Color contribGI = pathtrace(w_out, pathMultiplier * brdf / pdf, rnd);
	return contribLight + contribGI;	
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
	ray.flags |= RF_DEBUG;
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
	
	Random rnd = getRandomGen();
	for (int i = 0; i < N; i++) {
		Ray ray = scene.camera->getScreenRay(
			x + rnd.randdouble(), y + rnd.randdouble()
		);
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
		return raytraceSinglePixel(x, y);
	}
}

static void detectAApixels(const vector<Rect>& buckets)
{
	const int neighbours[8][2] = {
		{ -1, -1 }, { 0, -1 }, { 1, -1 },
		{ -1,  0 },            { 1,  0 },
		{ -1,  1 }, { 0,  1 }, { 1,  1 }
	};
	int W = frameWidth(), H = frameHeight();
	const float AA_THRESH = 0.1f;
	for (auto& r: buckets) {
		for (int y = r.y0; y < r.y1; y++)
			for (int x = r.x0; x < r.x1; x++) {
				needsAA[y][x] = false;
				const Color& me = vfb[y][x];
				for (int ni = 0; ni < COUNT_OF(neighbours); ni++) {
					int neighX = x + neighbours[ni][0];
					int neighY = y + neighbours[ni][1];
					if (neighX < 0 || neighX >= W || neighY < 0 || neighY >= H) continue;
					const Color& neighbour = vfb[neighY][neighX];
					for (int channel = 0; channel < 3; channel++) {
						if (fabs(min(1.0f, me[channel]) - min(1.0f, neighbour[channel])) > AA_THRESH) {
							needsAA[y][x] = true;
							break;
						}
					}
					if (needsAA[y][x]) break;
				}
			}
	}
	markAApixels(needsAA);
}

class RenderScreenTask: public Parallel {
protected:
	const vector<Rect>& buckets;
	InterlockedInt counter;
public:
	RenderScreenTask(const vector<Rect>& buckets): buckets(buckets), counter(0) {}
};

// Main rendering task. For scenes, that have implicit AA (with DOF or GI), this
// is all that is needed. If explicit AA is needed, this task is just a prepass
// that renders the base screen (1-sample-per-pixel).
struct MainRenderTask: public RenderScreenTask {
	bool finalPass;
	MainRenderTask(const vector<Rect>& buckets): RenderScreenTask(buckets)
	{
		finalPass = !scene.settings.needAApass();
	}
	
	void entry(int threadIdx, int threadCount)
	{
		int i;
		while ((i = counter++) < int(buckets.size())) {
			const Rect& r = buckets[i];
			if (!scene.settings.interactive && finalPass && !markRegion(r)) return;
			for (int y = r.y0; y < r.y1; y++)
				for (int x = r.x0; x < r.x1; x++) {
					vfb[y][x] = renderPixel(x, y);
				}
			if (!scene.settings.interactive && !displayVFBRect(r, vfb)) return;
		}
	}
};

// If explicit AA is needed, this task refines the image around sharp edges
// (as detected by detectAApixels()).
struct RefineRenderTask: public RenderScreenTask {
	RefineRenderTask(const vector<Rect>& buckets): RenderScreenTask(buckets) {}
	
	void entry(int threadIdx, int threadCount)
	{
		const double kernel[5][2] = {
			// note that this sample is already rendered in vfb[][]:
			{ 0.0, 0.0 }, 
			
			// refinement adds these samples:
			{ 0.6, 0.0 },
			{ 0.0, 0.6 },
			{ 0.3, 0.3 },
			{ 0.6, 0.6 },
		};
		int i;
		while ((i = counter++) < int(buckets.size())) {
			const Rect& r = buckets[i];
			// see if the bucket contains anything interesting at all:
			bool skipBucket = true;
			for (int y = r.y0; y < r.y1 && skipBucket; y++)
				for (int x = r.x0; x < r.x1; x++) 
					if (needsAA[y][x]) {
						skipBucket = false;
						break;
					}
			if (skipBucket) continue;
			
			if (!markRegion(r)) return;
			for (int y = r.y0; y < r.y1; y++)
				for (int x = r.x0; x < r.x1; x++) {
					if (!needsAA[y][x]) continue;
					Color& sum = vfb[y][x];
					for (int j = 1; j < COUNT_OF(kernel); j++)
						sum += raytraceSinglePixel(x + kernel[j][0], y + kernel[j][1]);
					sum /= COUNT_OF(kernel);
				}
			if (!scene.settings.interactive && !displayVFBRect(r, vfb)) return;
		}
	}
};

void render()
{
	scene.beginFrame();
	vector<Rect> buckets = getBucketsList();
	
	if (!scene.settings.interactive && (scene.settings.wantPrepass || scene.settings.gi)) {
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

	MainRenderTask mtrend(buckets);
	pool.run(&mtrend, scene.settings.numThreads);
	
	if (scene.settings.needAApass()) {
		// the previous render was without any anti-aliasing whatsoever, and the 
		// scene file specifies that AA is desired. Detect edges here, and refine in another pass:
		detectAApixels(buckets);
		
		RefineRenderTask refineTask(buckets);
		pool.run(&refineTask, scene.settings.numThreads);
	}
}

int renderSceneThread(void* /*unused*/)
{
	render();
	rendering = false;
	return 0;
}

void mainloop(void)
{
	SDL_ShowCursor(0);
	bool running = true;
	Camera& cam = *scene.camera;
	const double MOVEMENT_PER_SEC = 20;
	const double ROTATION_PER_SEC = 50;
	const double SENSITIVITY = 0.1;
	
	while (running) {
		Uint32 ticksSaved = SDL_GetTicks();
		render();
		displayVFB(vfb);
		// timeDelta is how much time the frame took to render:
		double timeDelta = (SDL_GetTicks() - ticksSaved) / 1000.0;
		// 
		SDL_Event ev;
		
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_KEYDOWN:
				{
					switch (ev.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = false;
							break;
						default:
							break;
					}
					break;
				}
			}
		}
		
		Uint8* keystate = SDL_GetKeyState(NULL);
		double movement = MOVEMENT_PER_SEC * timeDelta;
		double rotation = ROTATION_PER_SEC * timeDelta;
		if (keystate[SDLK_UP]) cam.move(0, +movement);
		if (keystate[SDLK_DOWN]) cam.move(0, -movement);
		if (keystate[SDLK_LEFT]) cam.move(-movement, 0);
		if (keystate[SDLK_RIGHT]) cam.move(+movement, 0);

		if (keystate[SDLK_KP8]) cam.rotate(0, +rotation);
		if (keystate[SDLK_KP2]) cam.rotate(0, -rotation);
		if (keystate[SDLK_KP4]) cam.rotate(+rotation, 0);
		if (keystate[SDLK_KP6]) cam.rotate(-rotation, 0);
		
		int deltax, deltay;
		SDL_GetRelativeMouseState(&deltax, &deltay);
		cam.rotate(-SENSITIVITY * deltax, -SENSITIVITY*deltay);
	}
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
	
	initGraphics(scene.settings.frameWidth, scene.settings.frameHeight,
		scene.settings.interactive && scene.settings.fullscreen);
	setWindowCaption("Quad Damage: preparing...");
	
	if (scene.settings.numThreads == 0)
		scene.settings.numThreads = get_processor_count();
	
	pool.preload_threads(scene.settings.numThreads);
	
	scene.beginRender();
	
	if (scene.settings.interactive) {
		mainloop();
	} else {
		
		setWindowCaption("Quad Damage: rendering...");
		Uint32 startTicks = SDL_GetTicks();
		renderScene_threaded();
		Uint32 elapsedMs = SDL_GetTicks() - startTicks;
		printf("Render took %.2fs\n", elapsedMs / 1000.0f);
		setWindowCaption("Quad Damage: rendered in %.2fs\n", elapsedMs / 1000.0f);
		
		displayVFB(vfb);
		waitForUserExit();
	}
	closeGraphics();
	printf("Exited cleanly\n");
	return 0;
}
