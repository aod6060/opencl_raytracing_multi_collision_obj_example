#include "sys.h"

namespace graphics {

	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue commands;
	cl_program program;

	// Kernels
	cl_kernel rendererKernel;
	cl_kernel presentKernel;

	// Buffers
	cl_mem framebuffer;
	cl_mem screen;

	cl_mem spheres;
	size_t spheresLength;

	cl_mem lights;
	size_t lightsLength;

	void init() {
		cl_uint length;
		cl_int err;

		std::vector<cl_platform_id> platforms;

		clGetPlatformIDs(0, 0, &length);
		platforms.resize(length);
		clGetPlatformIDs(platforms.size(), platforms.data(), 0);

		for (int i = 0; i < platforms.size(); i++) {
			std::vector<cl_device_id> devices;

			clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, 0, &length);
			devices.resize(length);
			clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, devices.size(), devices.data(), 0);

			if (devices.size() > 0) {
				device = devices[0];
				platform = platforms[i];
				break;
			}
		}

		context = clCreateContext(0, 1, &device, nullptr, nullptr, &err);

		if (!context) {
			std::cout << "Context wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		commands = clCreateCommandQueue(context, device, 0, &err);

		if (!commands) {
			std::cout << "Commands wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		std::ifstream in("data/kernel/raytracer.cl");
		std::stringstream ss;
		std::string temp;

		while (std::getline(in, temp)) {
			ss << temp << std::endl;
		}

		in.close();

		std::string src = ss.str();
		const char* c_src = src.c_str();

		std::cout << c_src << std::endl;

		program = clCreateProgramWithSource(context, 1, &c_src, nullptr, &err);

		if (!program) {
			std::cout << "Program wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		err = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);

		if (err != CL_SUCCESS) {
			char buf[2048];
			std::cout << "Error: Failed to build program" << std::endl;
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buf), buf, &length);
			std::cout << buf << std::endl;
			std::getchar();
			app::exit();
			exit(1);
		}


		// Kernels
		rendererKernel = clCreateKernel(program, "renderer", &err);

		if (!rendererKernel) {
			std::cout << "RendererKernel wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		presentKernel = clCreateKernel(program, "present", &err);

		if (!presentKernel) {
			std::cout << "PresentKernel wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		size_t size = app::getWidth() * app::getHeight();

		framebuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, size * sizeof(Color), nullptr, &err);

		if (!framebuffer) {
			std::cout << "framebuffer wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		screen = clCreateBuffer(context, CL_MEM_READ_WRITE, size * sizeof(SDL_Color), nullptr, &err);

		if (!screen) {
			std::cout << "screen wasn't created" << std::endl;
			app::exit();
			exit(1);
		}
	}

	void release() {
		clReleaseMemObject(screen);
		clReleaseMemObject(framebuffer);
		clReleaseKernel(presentKernel);
		clReleaseKernel(rendererKernel);
		clReleaseProgram(program);
		clReleaseCommandQueue(commands);
		clReleaseContext(context);
	}

	Sphere createSphere(
		const glm::vec3& position,
		float radius,
		const glm::vec3& color
	) {
		Sphere temp;
		toFloat3(temp.position, position);
		temp.radius = radius;
		toFloat3(temp.color, color);
		return temp;
	}

	void uploadSpheres(std::vector<Sphere>& s) {
		if (spheres) {
			clReleaseMemObject(spheres);
		}
		cl_int err;
		spheres = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, s.size() * sizeof(Sphere), s.data(), &err);

		if (!spheres) {
			std::cout << "spheres wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		spheresLength = s.size();
	}

	Light createLight(
		const glm::vec3& position,
		float intensity,
		const glm::vec3& color
	) {
		Light light;
		toFloat3(light.position, position);
		light.intencity = intensity;
		toFloat3(light.color, color);
		return light;
	}

	void uploadLights(std::vector<Light>& l) {
		if (lights) {
			clReleaseMemObject(lights);
		}
		cl_int err;
		lights = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, l.size() * sizeof(Light), l.data(), &err);

		if (!lights) {
			std::cout << "spheres wasn't created" << std::endl;
			app::exit();
			exit(1);
		}

		lightsLength = l.size();
	}

	Camera createCamera(
		float fov, 
		float aspect, 
		float zmin, 
		float zmax, 
		glm::vec3 position) {
		Camera temp;
		toFloat3(temp.position, position);

		toFloat3(temp.forward, glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)));
		toFloat3(temp.right, glm::normalize(glm::cross(toVec3(temp.forward), glm::vec3(0.0f, 1.0f, 0.0f))));
		toFloat3(temp.up, glm::cross(toVec3(temp.forward), toVec3(temp.right)));

		temp.height = tan(fov);
		temp.width = temp.height * aspect;

		temp.zmin = zmin;
		temp.zmax = zmax;

		temp.yaw = 0.0f;
		temp.pitch = 0.0f;
		return temp;
	}

	void updateCamera(
		Camera& camera, 
		float delta,
		float rotSpeed, 
		float walkSpeed) {

		if (input::isKeyPressed(input::Keyboard::KB_LEFT)) {
			camera.yaw -= rotSpeed * delta;
		}

		if (input::isKeyPressed(input::Keyboard::KB_RIGHT)) {
			camera.yaw += rotSpeed * delta;
		}

		if (camera.yaw < -360.0f) {
			camera.yaw += 360.0f;
		}

		if (camera.yaw > 360.0f) {
			camera.yaw -= 360.0f;
		}

		if (input::isKeyPressed(input::Keyboard::KB_UP)) {
			camera.pitch += rotSpeed * delta;
		}

		if (input::isKeyPressed(input::Keyboard::KB_DOWN)) {
			camera.pitch -= rotSpeed * delta;
		}


		if (camera.pitch < -90.0f) {
			camera.pitch = -90.0f;
		}

		if (camera.pitch > 90.0f) {
			camera.pitch = 90.0f;
		}

		glm::vec3 direction = glm::vec3(
			glm::cos(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch)),
			glm::sin(glm::radians(camera.pitch)),
			glm::sin(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch))
		);

		toFloat3(camera.forward, glm::normalize(direction));
		toFloat3(camera.right, glm::normalize(glm::cross(toVec3(camera.forward), glm::vec3(0.0f, 1.0f, 0.0f))));
		toFloat3(camera.up, glm::cross(toVec3(camera.forward), toVec3(camera.right)));

		glm::vec3 forward = glm::vec3(
			camera.forward.x,
			0.0f,
			camera.forward.z);

		if (input::isKeyPressed(input::Keyboard::KB_W)) {
			camera.position.x += walkSpeed * delta * forward.x;
			camera.position.z += walkSpeed * delta * forward.z;
		}

		if (input::isKeyPressed(input::Keyboard::KB_S)) {
			camera.position.x -= walkSpeed * delta * forward.x;
			camera.position.z -= walkSpeed * delta * forward.z;
		}

		if (input::isKeyPressed(input::Keyboard::KB_A)) {
			camera.position.x -= walkSpeed * delta * camera.right.x;
			camera.position.z -= walkSpeed * delta * camera.right.z;
		}

		if (input::isKeyPressed(input::Keyboard::KB_D)) {
			camera.position.x += walkSpeed * delta * camera.right.x;
			camera.position.z += walkSpeed * delta * camera.right.z;
		}

		if (input::isKeyPressed(input::Keyboard::KB_SPACE)) {
			camera.position.y += walkSpeed * delta;
		}

		if (input::isKeyPressed(input::Keyboard::KB_LSHIFT)) {
			camera.position.y -= walkSpeed * delta;
		}
	}

	void raytrace(cl_float3 clearColor, Camera& camera) {
		cl_int err;

		size_t globalWorkSize[2] = {
			app::getWidth(),
			app::getHeight()
		};

		size_t localWorkSize[2] = {
			16, 16
		};

		err = clSetKernelArg(rendererKernel, 0, sizeof(cl_mem), (void*)&framebuffer);
		err |= clSetKernelArg(rendererKernel, 1, sizeof(cl_mem), (void*)&spheres);
		err |= clSetKernelArg(rendererKernel, 2, sizeof(size_t), (void*)&spheresLength);
		err |= clSetKernelArg(rendererKernel, 3, sizeof(cl_mem), (void*)&lights);
		err |= clSetKernelArg(rendererKernel, 4, sizeof(size_t), (void*)&lightsLength);
		err |= clSetKernelArg(rendererKernel, 5, sizeof(Camera), (void*)&camera);
		err |= clSetKernelArg(rendererKernel, 6, sizeof(Color), (void*)&clearColor);

		if (err != CL_SUCCESS) {
			std::cout << "Failed to set rendererKernel Arguments" << std::endl;
			return;
		}

		err = clEnqueueNDRangeKernel(commands, rendererKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);

		if (err != CL_SUCCESS) {
			std::cout << "Failed to submit range kernel for rendererKernel" << std::endl;
			return;
		}

		err = clFinish(commands);

	}

	void present() {
		cl_int err;

		size_t globalWorkSize[2] = {
			app::getWidth(),
			app::getHeight()
		};

		size_t localWorkSize[2] = {
			16, 16
		};

		err = clSetKernelArg(presentKernel, 0, sizeof(cl_mem), (void*)&screen);
		err |= clSetKernelArg(presentKernel, 1, sizeof(cl_mem), (void*)&framebuffer);

		if (err != CL_SUCCESS) {
			std::cout << "Failed to set presentKernal Arguments" << std::endl;
			return;
		}

		err = clEnqueueNDRangeKernel(commands, presentKernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, nullptr);

		if (err != CL_SUCCESS) {
			std::cout << "Failed to call presentKernel" << std::endl;
			return;
		}

		SDL_Surface* winScreen = app::getScreenSurface();
		SDL_LockSurface(winScreen);
		SDL_Color* screenColors = (SDL_Color*)winScreen->pixels;
		cl_uint size = app::getWidth() * app::getHeight();
		err = clEnqueueReadBuffer(commands, screen, CL_TRUE, 0, size * sizeof(SDL_Color), screenColors, 0, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			std::cout << "Failed to read screen buffer" << std::endl;
		}
		// Read screen buffer
		SDL_UnlockSurface(winScreen);

	}

	glm::vec3 toVec3(cl_float3& v) {
		return glm::vec3(v.x, v.y, v.z);
	}

	void toFloat3(
		cl_float3& out, 
		const glm::vec3& v) {
		out.x = v.x;
		out.y = v.y;
		out.z = v.z;
	}
}