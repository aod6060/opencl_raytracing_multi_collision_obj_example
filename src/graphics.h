#pragma once


namespace graphics {


	struct Color {
		float r;
		float g;
		float b;
	};

	struct Material {
		cl_float3 color;
		cl_float specularFactor;
	};

	enum SceneObjectType {
		SOT_SPHERE = 0,
		SOT_PLANE,
		SOT_CUBE,
		SOT_TORUS,
		SOT_CAPSULE,
		SOT_CYLINDER,
		SOT_TRIANGLE,
		SOT_SIZE
	};

	struct SceneObject {
		cl_float3 position;
		SceneObjectType type;
		cl_uint materialIndex;

		// Sphere
		cl_float sphereRadius;
		// Plane

		// Cube

		// Torus

		// Capsule

		// Cylinder

		// Triangle
	};

	/*
	struct Sphere {
		cl_float3 position;
		cl_uint materialIndex;

		// Sphere Data...
		cl_float radius;
		// Plane Data
		// Cube Data
		// Torus Data
		// Capsule Data
		// Cylinder Data
		// Triangle Data
	};
	*/

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
		cl_float yaw;
		cl_float pitch;
	};


	void init();
	void release();
	/*
	Sphere createSphere(
		const glm::vec3& position,
		float radius,
		cl_uint materialIndex
	);

	void uploadSpheres(std::vector<Sphere>& spheres);
	*/

	SceneObject createSphereSceneObject(const glm::vec3& position, cl_uint materialIndex, float radius);

	void uploadSceneObject(std::vector<SceneObject>& sceneObjects);

	Light createLight(
		const glm::vec3& position,
		float intensity,
		const glm::vec3& color
	);

	Material createMaterial(
		const glm::vec3& color,
		float specularFactor
	);

	void uploadMaterials(std::vector<Material>& materials);

	void uploadLights(std::vector<Light>& lights);

	Camera createCamera(
		float fov, 
		float aspect, 
		float zmin, 
		float zmax, 
		glm::vec3 position);

	void updateCamera(
		Camera& camera,
		float delta,
		float rotSpeed, 
		float walkSpeed);

	void raytrace(cl_float3 clearColor, Camera& camera);

	void present();

	glm::vec3 toVec3(cl_float3& v);

	void toFloat3(
		cl_float3& out, 
		const glm::vec3& v);
}