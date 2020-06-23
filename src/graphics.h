#pragma once


namespace graphics {


	struct Color {
		float r;
		float g;
		float b;
	};

	struct Sphere {
		cl_float3 position;
		cl_float radius;
		cl_float3 color;
	};

	struct Light {
		cl_float3 position;
		cl_float intencity;
		cl_float3 color;
	};

	struct Camera {
		cl_float3 position;
		cl_float3 forward;
		cl_float3 right;
		cl_float3 up;
		cl_float width;
		cl_float height;
		cl_float zmin;
		cl_float zmax;
	};


	void init();
	void release();

	Sphere createSphere(
		const glm::vec3& position,
		float radius,
		const glm::vec3& color
	);

	void uploadSpheres(std::vector<Sphere>& spheres);

	Light createLight(
		const glm::vec3& position,
		float intensity,
		const glm::vec3& color
	);

	void uploadLights(std::vector<Light>& lights);

	Camera createCamera(
		float fov, 
		float aspect, 
		float zmin, 
		float zmax, 
		glm::vec3 position);

	void updateCamera(
		Camera& camera, 
		float rotSpeed, 
		float walkSpeed);

	void raytrace(cl_float3 clearColor, Camera& camera);

	void present();

	glm::vec3 toVec3(cl_float3& v);

	void toFloat3(
		cl_float3& out, 
		const glm::vec3& v);
}