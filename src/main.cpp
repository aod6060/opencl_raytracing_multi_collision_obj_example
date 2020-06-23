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
		graphics::createMaterial(glm::vec3(0.5f)),
		graphics::createMaterial(glm::vec3(0.0f, 0.5f, 0.0f)),
		graphics::createMaterial(glm::vec3(0.0f, 0.0f, 0.5f)),
		graphics::createMaterial(glm::vec3(0.5f, 0.0f, 0.0f)),
		graphics::createMaterial(glm::vec3(0.5f, 0.5f, 0.0f))
	};

	graphics::uploadMaterials(materials);

	// Spheres
	std::vector<graphics::Sphere> spheres = {
		graphics::createSphere(glm::vec3(-8, 0, 0), 1, 0),
		graphics::createSphere(glm::vec3(0, 0, -8), 1, 1),
		graphics::createSphere(glm::vec3(8, 0, 0), 1, 2),
		graphics::createSphere(glm::vec3(0, 0, 8), 1, 3),
		graphics::createSphere(glm::vec3(0, -5001, 0), 5000, 4)
	};

	graphics::uploadSpheres(spheres);

	// Lights
	std::vector<graphics::Light> lights = {
		graphics::createLight(glm::vec3(0.0f, 1.0f, 0.0f), 0.6f, glm::vec3(1.0f, 1.0f, 1.0f))
	};

	graphics::uploadLights(lights);
}
void app_update(float delta) {
	graphics::updateCamera(camera, delta, 64.0f, 4.0f);
}

void app_render() {

	cl_float3 clearColor;

	graphics::toFloat3(clearColor, glm::vec3(0.0f, 0.0f, 0.0f));

	graphics::raytrace(clearColor, camera);

	graphics::present();
}

void app_release() {
	graphics::release();
}