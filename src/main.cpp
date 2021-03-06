#include "sys.h"

void app_init();
void app_update(float delta);
void app_render();
void app_release();

int main(int argc, char** argv) {

	app::AppConfig config;

	config.caption = "OpenCL Raytracer: Multiple Object Types";
	config.width = 1280;
	config.height = 720;
	config.initCB = app_init;
	config.updateCB = app_update;
	config.renderCB = app_render;
	config.releaseCB = app_release;

	app::init(&config);

	app::update();

	app::release();

	return 0;
}

graphics::Camera camera;
graphics::GlobalDirectionalLight globalLight;

void app_init() {
	graphics::init();

	camera = graphics::createCamera(
		60.0f, 
		app::getWidthCast<float>() / app::getHeightCast<float>(), 
		0.1f, 
		1024.0f, 
		glm::vec3(0.0f));

	// Materials
	std::vector<graphics::Material> materials = {
		graphics::createMaterial(glm::vec3(0.5f), 0.5f),
		graphics::createMaterial(glm::vec3(0.0f, 0.5f, 0.0f), 0.5f),
		graphics::createMaterial(glm::vec3(0.0f, 0.0f, 0.5f), 0.5f),
		graphics::createMaterial(glm::vec3(0.5f, 0.0f, 0.0f), 0.5f),
		graphics::createMaterial(glm::vec3(0.5f, 0.5f, 0.0f), 0.5f)
	};

	graphics::uploadMaterials(materials);

	// Scene Objects
	std::vector<graphics::SceneObject> sceneObjects = {
		graphics::createSphereSceneObject(glm::vec3(-8, 0, 0), 0, 1),
		graphics::createSphereSceneObject(glm::vec3(0, 0, -8), 1, 1),
		graphics::createSphereSceneObject(glm::vec3(8, 0, 0), 2, 1),
		graphics::createSphereSceneObject(glm::vec3(0, 0, 9), 3, 1),
		graphics::createSphereSceneObject(glm::vec3(0, -5001, 0), 4, 5000)
	};

	graphics::uploadSceneObject(sceneObjects);
	// Lights

	glm::vec3 minLight = glm::vec3(-1.0f, -1.0f, -1.0f);
	glm::vec3 maxLight = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::vec3 dir = glm::normalize(maxLight - minLight);


	globalLight = graphics::createGlobalDirectionalLight(
		dir,
		0.6,
		glm::vec3(1.0f)
	);

}
void app_update(float delta) {
	graphics::updateCamera(camera, delta, 64.0f, 4.0f);
}

void app_render() {

	cl_float3 clearColor;

	graphics::toFloat3(clearColor, glm::vec3(0.53f, 0.81f, 0.92f));
	graphics::raytrace(clearColor, camera, globalLight);
	graphics::present();
}

void app_release() {
	graphics::release();
}