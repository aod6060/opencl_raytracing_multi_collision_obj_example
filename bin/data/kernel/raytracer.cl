/*
    raytracer.cl
*/
struct SDL_Color {
    uchar r;
    uchar g;
    uchar b;
    uchar a;
};

struct Color {
    float r;
    float g;
    float b;
};

struct Ray {
    float3 position;
    float3 direction;
};

struct Camera {
    float3 position;
    float3 foward;
    float3 right;
    float3 up;
    float width;
    float height;
    float zmin;
    float zmax;
    float yaw;
    float pitch;
};

struct Material {
    float3 color;
    float specularFactor;
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
    float3 position;
    enum SceneObjectType type;
    uint materialIndex;

    // Sphere
    float sphereRadius;
    // Plane

    // Cube

    // Torus

    // Capsule

    // Cylinder

    // Triangle

};

/*
struct Sphere {
    float3 position;
    uint materialIndex;

    float radius;
};
*/
struct Light {
    float3 position;
    float intencity;
    float3 color;
};

struct Hit {
    bool isHit;
    struct SceneObject sceneObject;
    float t;
};

struct ReflectHit {
    struct Ray nextRay;
    float3 color;
    float specularFactor;
};

struct Ray camera_makeRay(float2 point, struct Camera camera) {
    float3 d = camera.foward + point.x * camera.width * camera.right + point.y * camera.height * camera.up;
    struct Ray ray;
    ray.position = camera.position;
    ray.direction = normalize(d);
    return ray;
}

float2 sphereIntersection(struct Ray ray, struct SceneObject sphere) {
    float3 v = ray.position - sphere.position;

    float k1 = dot(ray.direction, ray.direction);
    float k2 = 2 * dot(v, ray.direction);
    float k3 = dot(v, v) - sphere.sphereRadius * sphere.sphereRadius;

    float d = k2 * k2 - 4 * k1 * k3;

    float2 temp;

    temp.x = (-k2 + sqrt(d)) / (2 * k1);
    temp.y = (-k2 - sqrt(d)) / (2 * k1);

    return temp;
}

struct Hit closestIntersection(
    struct Ray ray, 
    float zmin, 
    float zmax,
     __global struct SceneObject* sceneObjects, 
     uint sceneObjectsLength, 
     float t) {
    //bool b = false;
    struct Hit hit;
    hit.isHit = false;
    hit.t = t;

    for(uint i = 0; i < sceneObjectsLength; i++) {
        float2 tv = (float2)(0.0f, 0.0f);

        if(sceneObjects[i].type == SOT_SPHERE) {
            tv = sphereIntersection(ray, sceneObjects[i]);
        }

        if((tv.x >= zmin && tv.x <= zmax) && tv.x < hit.t) {
            hit.t = tv.x;
            hit.sceneObject = sceneObjects[i];
            hit.isHit = true;
        }

        if((tv.y >= zmin && tv.y <= zmax) && tv.y < hit.t) {
            hit.t = tv.y;
            hit.sceneObject = sceneObjects[i];
            hit.isHit = true;
        }
    }

    return hit;
}

float3 reflect(float3 R, float3 N) {
    return 2.0f * N * dot(N, R) - R;
}

struct ReflectHit computeAmbient(
    struct Ray ray,
    float zmin,
    float zmax,
    __global struct SceneObject* sceneObjects,
    uint sceneObjectsLength,
    __global struct Light* lights, 
    uint lightLength,
    __global struct Material* materials,
    uint materialsLength,
    float3 clearColor
) {
    struct Hit hit = closestIntersection(
        ray, 
        zmin, 
        zmax, 
        sceneObjects, 
        sceneObjectsLength, 
        zmax);

    if(!hit.isHit) {
        // Reflect Ray
        struct ReflectHit reflectHit;
        reflectHit.nextRay = ray;
        reflectHit.color = clearColor;
        return reflectHit;
    }

    float3 P = ray.position + ray.direction * hit.t;
    float3 N = P - hit.sceneObject.position;
    N = normalize(N);
    float3 V = -ray.direction;

    float3 light = (float3)(0.0, 0.0, 0.0);

    struct Material m = materials[hit.sceneObject.materialIndex];

    for(int i = 0; i < lightLength; i++) {
        struct Ray shadowRay;
        shadowRay.position = P;
        shadowRay.direction = lights[i].position - P;

        struct Hit shadowHit = closestIntersection(
            shadowRay,
            0.001f,
            1024.0f,
            sceneObjects,
            sceneObjectsLength,
            1024.0f
        );

        if(shadowHit.isHit) {
            continue;
        }

        float3 L = normalize(lights[i].position - P);
        float3 H = normalize(L + V);


        float ndotl = dot(N, L);
        float ndoth = dot(N, H);

        float3 diffuse = m.color * lights[i].color * ndotl;
        float3 specular = lights[i].color * pow(ndoth, m.specularFactor * 256.0f);

        light += (diffuse + specular) * lights[i].intencity;
    }

    light += (float3)(0.1f, 0.1f, 0.1f) * m.color;

    // Reflect Ray
    float3 R = reflect(-ray.direction, N);

    struct Ray reflectRay;
    reflectRay.position = P;
    reflectRay.direction = R;

    struct ReflectHit reflectHit;
    reflectHit.nextRay = reflectRay;
    reflectHit.color = light;
    reflectHit.specularFactor = m.specularFactor;
    return reflectHit;
}

struct Color computeLighting(
    struct Ray ray,
    float3 P, 
    float3 N, 
    float3 V, 
    struct Hit hit, 
    __global struct SceneObject* sceneObjects,
    uint sceneObjectsLength,
    __global struct Light* lights, 
    uint lightLength,
    __global struct Material* materials,
    uint materialsLength,
    float3 clearColor) {

    float3 light = (float3)(0.0f, 0.0f, 0.0f);

    struct Material m = materials[hit.sceneObject.materialIndex];

    for(uint i = 0; i < lightLength; i++) {

        struct Ray shadowRay;
        shadowRay.position = P;
        shadowRay.direction = lights[i].position - P;

        struct Hit shadowHit = closestIntersection(
            shadowRay,
            0.001f,
            1024.0f,
            sceneObjects,
            sceneObjectsLength,
            1024.0f
        );

        if(shadowHit.isHit) {
            continue;
        }

        float3 L = normalize(lights[i].position - P);
        float3 H = normalize(L + V);

        float ndotl = dot(N, L);
        float ndoth = dot(N, H);

        float3 diffuse = m.color * lights[i].color * ndotl;
        float3 specular = lights[i].color * pow(ndoth, m.specularFactor * 256.0f);

        light += (diffuse + specular) * lights[i].intencity;
    }

    light += (float3)(0.1, 0.1, 0.1) * m.color;
    // Iteration1
    
    if(m.specularFactor <= 0) {
        struct Color temp;
        temp.r = light.x;
        temp.g = light.y;
        temp.b = light.z;
        return temp;
    }

    // Ambient
    float3 R = reflect(-ray.direction, N);

    struct Ray reflectRay;
    reflectRay.position = P;
    reflectRay.direction = R;

    // Redo the scene...
    struct ReflectHit iteration1 = computeAmbient(
        reflectRay,
        0.1f,
        1024.0f,
        sceneObjects,
        sceneObjectsLength,
        lights,
        lightLength,
        materials,
        materialsLength,
        clearColor
    );

    struct ReflectHit finalIteration = computeAmbient(
        iteration1.nextRay,
        0.1f,
        1024.0f,
        sceneObjects,
        sceneObjectsLength,
        lights,
        lightLength,
        materials,
        materialsLength,
        clearColor
    );

    float3 ambient = iteration1.color * iteration1.specularFactor + finalIteration.color * finalIteration.specularFactor;

    struct Color temp;
    temp.r = light.x * (1.0f - m.specularFactor) + ambient.x * (m.specularFactor);
    temp.g = light.y * (1.0f - m.specularFactor) + ambient.y * (m.specularFactor);
    temp.b = light.z * (1.0f - m.specularFactor) + ambient.z * (m.specularFactor);
    
    return temp;
}

struct Color raytracer(
    struct Ray ray, 
    float zmin, 
    float zmax, 
    struct Color clearColor, 
    __global struct SceneObject* sceneObjects, 
    uint sceneObjectsLength,
    __global struct Light* lights,
    uint lightsLength,
    __global struct Material* materials,
    uint materialsLength) 
{
    struct Hit hit = closestIntersection(
        ray, 
        zmin, 
        zmax, 
        sceneObjects, 
        sceneObjectsLength, 
        zmax);

    if(!hit.isHit) {
        return clearColor;
    }

    // Lighting
    float3 P = ray.position + ray.direction * hit.t;
    float3 N = P - hit.sceneObject.position;
    N = normalize(N);

    struct Color temp = computeLighting(
        ray,
        P,
        N,
        -ray.direction,
        hit,
        sceneObjects,
        sceneObjectsLength,
        lights,
        lightsLength,
        materials,
        materialsLength,
        (float3)(clearColor.r, clearColor.g, clearColor.b)
    );

    return temp;
}

__kernel void renderer(
    __global struct Color* framebuffer,
    __global struct SceneObject* sceneObjects,
    uint sceneObjectsLength,
    __global struct Light* lights,
    uint lightsLength,
    __global struct Material* materials,
    uint materialsLength,
    struct Camera camera,
    struct Color clearColor
) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    float2 sc;
    sc.x = convert_float(x * 2) / width - 1.0;
    sc.y = convert_float(y * 2) / height - 1.0;

    struct Ray ray = camera_makeRay(sc, camera);

    struct Color color = raytracer(
        ray, 
        camera.zmin, 
        camera.zmax, 
        clearColor, 
        sceneObjects, 
        sceneObjectsLength,
        lights,
        lightsLength,
        materials,
        materialsLength);

    
    framebuffer[y * width + x].r = clamp(color.r, 0.0f, 1.0f);
    framebuffer[y * width + x].g = clamp(color.g, 0.0f, 1.0f);
    framebuffer[y * width + x].b = clamp(color.b, 0.0f, 1.0f);
}

__kernel void present(
    __global struct SDL_Color* screen,
    __global struct Color* framebuffer
) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);

    screen[y * width + x].r = convert_uchar(framebuffer[y * width + x].b * 255);
    screen[y * width + x].g = convert_uchar(framebuffer[y * width + x].g * 255);
    screen[y * width + x].b = convert_uchar(framebuffer[y * width + x].r* 255);
    screen[y * width + x].a = 255;
}