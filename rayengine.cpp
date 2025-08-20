#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <random>
#include <windows.h>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vector3D class with advanced operations
class Vec3 {
public:
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(double t) const { return Vec3(x * t, y * t, z * t); }
    Vec3 operator/(double t) const { return Vec3(x / t, y / t, z / t); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }

    double length() const { return std::sqrt(x*x + y*y + z*z); }
    double length_squared() const { return x*x + y*y + z*z; }

    Vec3 normalize() const {
        double len = length();
        return len > 0 ? *this / len : Vec3(0, 0, 0);
    }

    double dot(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; }
    Vec3 cross(const Vec3& v) const {
        return Vec3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
    }

    // Reflect vector about normal
    Vec3 reflect(const Vec3& n) const {
        return *this - n * (2 * dot(n));
    }

    // Refract vector through surface
    Vec3 refract(const Vec3& n, double etai_over_etat) const {
        double cos_theta = std::min(dot(-n), 1.0);
        Vec3 r_out_perp = (*this + n * cos_theta) * etai_over_etat;
        Vec3 r_out_parallel = n * (-std::sqrt(std::abs(1.0 - r_out_perp.length_squared())));
        return r_out_perp + r_out_parallel;
    }
};

Vec3 operator*(double t, const Vec3& v) { return v * t; }

// Matrix4x4 class for transformations
class Mat4 {
public:
    double m[4][4];

    Mat4() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = (i == j) ? 1.0 : 0.0;
            }
        }
    }

    static Mat4 identity() {
        return Mat4();
    }

    static Mat4 translation(const Vec3& t) {
        Mat4 mat;
        mat.m[0][3] = t.x;
        mat.m[1][3] = t.y;
        mat.m[2][3] = t.z;
        return mat;
    }

    static Mat4 rotation_x(double angle) {
        Mat4 mat;
        double c = std::cos(angle);
        double s = std::sin(angle);
        mat.m[1][1] = c; mat.m[1][2] = -s;
        mat.m[2][1] = s; mat.m[2][2] = c;
        return mat;
    }

    static Mat4 rotation_y(double angle) {
        Mat4 mat;
        double c = std::cos(angle);
        double s = std::sin(angle);
        mat.m[0][0] = c; mat.m[0][2] = s;
        mat.m[2][0] = -s; mat.m[2][2] = c;
        return mat;
    }

    static Mat4 rotation_z(double angle) {
        Mat4 mat;
        double c = std::cos(angle);
        double s = std::sin(angle);
        mat.m[0][1] = -s; mat.m[0][0] = c;
        mat.m[1][0] = s; mat.m[1][1] = c;
        return mat;
    }

    static Mat4 scale(const Vec3& s) {
        Mat4 mat;
        mat.m[0][0] = s.x;
        mat.m[1][1] = s.y;
        mat.m[2][2] = s.z;
        return mat;
    }

    Vec3 transform_point(const Vec3& p) const {
        double x = m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3];
        double y = m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3];
        double z = m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3];
        double w = m[3][0]*p.x + m[3][1]*p.y + m[3][2]*p.z + m[3][3];
        if (w != 0) return Vec3(x/w, y/w, z/w);
        return Vec3(x, y, z);
    }

    Vec3 transform_vector(const Vec3& v) const {
        double x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z;
        double y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z;
        double z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z;
        return Vec3(x, y, z);
    }

    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }

    Mat4 transpose() const {
        Mat4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = m[j][i];
            }
        }
        return result;
    }
};

// Ray class
class Ray {
public:
    Vec3 origin;
    Vec3 direction;

    Ray() {}
    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction.normalize()) {}

    Vec3 at(double t) const { return origin + direction * t; }
};

// Color class with advanced color operations
class Color {
public:
    double r, g, b;

    Color() : r(0), g(0), b(0) {}
    Color(double r, double g, double b) : r(r), g(g), b(b) {}

    Color operator+(const Color& c) const { return Color(r + c.r, g + c.g, b + c.b); }
    Color operator*(double t) const { return Color(r * t, g * t, b * t); }
    Color operator*(const Color& c) const { return Color(r * c.r, g * c.g, b * c.b); }
    Color operator/(double t) const { return Color(r / t, g / t, b / t); }
    Color operator-(const Color& c) const { return Color(r - c.r, g - c.g, b - c.b); }

    Color& operator+=(const Color& c) { r += c.r; g += c.g; b += c.b; return *this; }
    Color& operator*=(double t) { r *= t; g *= t; b *= t; return *this; }

    Color clamp() const {
        return Color(
            std::min(1.0, std::max(0.0, r)),
            std::min(1.0, std::max(0.0, g)),
            std::min(1.0, std::max(0.0, b))
        );
    }

    // Advanced tone mapping
    Color tone_map_reinhard() const {
        Color denominator = *this + Color(1, 1, 1);
        Color mapped = Color(
            r / denominator.r,
            g / denominator.g,
            b / denominator.b
        );
        return mapped.clamp();
    }

    // Advanced gamma correction
    Color gamma(double gamma = 2.2) const {
        return Color(
            std::pow(std::max(0.0, r), 1.0/gamma),
            std::pow(std::max(0.0, g), 1.0/gamma),
            std::pow(std::max(0.0, b), 1.0/gamma)
        );
    }

    // Linear to sRGB conversion
    Color linear_to_srgb() const {
        auto transform = [](double c) {
            return c <= 0.0031308 ? 
                12.92 * c : 
                1.055 * std::pow(c, 1.0/2.4) - 0.055;
        };
        return Color(transform(r), transform(g), transform(b));
    }

    int to_int(double value) const {
        return static_cast<int>(255.999 * std::max(0.0, std::min(1.0, value)));
    }
};

Color operator*(double t, const Color& c) { return c * t; }

// ONB (Orthonormal Basis) for sampling
class ONB {
public:
    Vec3 axis[3];

    ONB() {}

    Vec3 operator[](int i) const { return axis[i]; }
    Vec3 u() const { return axis[0]; }
    Vec3 v() const { return axis[1]; }
    Vec3 w() const { return axis[2]; }

    Vec3 local(double a, double b, double c) const {
        return u()*a + v()*b + w()*c;
    }

    Vec3 local(const Vec3& a) const {
        return u()*a.x + v()*a.y + w()*a.z;
    }

    void build_from_w(const Vec3& n) {
        axis[2] = n.normalize();
        Vec3 a = (std::abs(w().x) > 0.9) ? Vec3(0,1,0) : Vec3(1,0,0);
        axis[1] = w().cross(a).normalize();
        axis[0] = w().cross(v());
    }
};

// Advanced Physically Based Material
class Material {
public:
    enum Type { LAMBERTIAN, METAL, DIELECTRIC, DIFFUSE_LIGHT };
    
    Type type;
    Color albedo;
    double roughness;    // 0 = smooth, 1 = rough
    double metallic;     // 0 = dielectric, 1 = metal
    double emission;
    double fuzz;         // Metal fuzziness
    double ior;          // Index of refraction
    double specular;     // Specular reflection coefficient

    Material() : type(LAMBERTIAN), albedo(0.5, 0.5, 0.5), roughness(0.5), metallic(0.0), 
                 emission(0.0), fuzz(0.0), ior(1.5), specular(0.5) {}

    Material(Type t, const Color& a, double r = 0.5, double m = 0.0, double e = 0.0, 
             double f = 0.0, double i = 1.5, double s = 0.5)
        : type(t), albedo(a), roughness(r), metallic(m), emission(e), fuzz(f), ior(i), specular(s) {}

    // Schlick's approximation for Fresnel reflectance
    Color schlick(double cosine, const Color& F0) const {
        return F0 + (Color(1,1,1) - F0) * std::pow(1.0 - cosine, 5);
    }

    // Importance sampling for different materials
    Vec3 sample(const Vec3& wo, const Vec3& normal, std::mt19937& rng) const {
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        
        switch (type) {
            case LAMBERTIAN: {
                // Cosine-weighted hemisphere sampling
                double r1 = dis(rng);
                double r2 = dis(rng);
                double z = std::sqrt(1 - r2);
                double phi = 2 * M_PI * r1;
                double x = std::cos(phi) * std::sqrt(r2);
                double y = std::sin(phi) * std::sqrt(r2);
                
                ONB uvw;
                uvw.build_from_w(normal);
                return uvw.local(Vec3(x, y, z)).normalize();
            }
            
            case METAL: {
                Vec3 reflected = wo.reflect(normal);
                Vec3 scattered = reflected + random_in_unit_sphere(rng) * fuzz;
                return scattered.dot(normal) > 0 ? scattered : Vec3(0,0,0);
            }
            
            case DIELECTRIC: {
                Vec3 outward_normal;
                Vec3 reflected = wo.reflect(normal);
                double ni_over_nt;
                double reflect_prob;
                double cosine;
                
                if (wo.dot(normal) > 0) {
                    outward_normal = -normal;
                    ni_over_nt = ior;
                    cosine = ior * wo.dot(normal) / wo.length();
                } else {
                    outward_normal = normal;
                    ni_over_nt = 1.0 / ior;
                    cosine = -wo.dot(normal) / wo.length();
                }
                
                Vec3 refracted;
                if (wo.refract(outward_normal, ni_over_nt).length_squared() > 0) {
                    reflect_prob = schlick(cosine, Color(0.04, 0.04, 0.04)).r;
                } else {
                    reflect_prob = 1.0;
                }
                
                if (dis(rng) < reflect_prob) {
                    return reflected;
                } else {
                    return wo.refract(outward_normal, ni_over_nt);
                }
            }
            
            default:
                return normal + random_in_unit_sphere(rng);
        }
    }

    // BRDF evaluation
    Color brdf(const Vec3& wo, const Vec3& wi, const Vec3& normal) const {
        switch (type) {
            case LAMBERTIAN: {
                // Lambertian diffuse
                double cos_theta = normal.dot(wi);
                if (cos_theta <= 0) return Color(0, 0, 0);
                return albedo / M_PI;
            }
            
            case METAL: {
                // Perfect reflection for metals
                Vec3 reflected = wo.reflect(normal);
                double cos_theta = reflected.dot(wi);
                if (cos_theta <= 0) return Color(0, 0, 0);
                return albedo;
            }
            
            case DIELECTRIC: {
                // Transmission and reflection
                return Color(1, 1, 1); // Simplified
            }
            
            default:
                return albedo;
        }
    }

private:
    Vec3 random_in_unit_sphere(std::mt19937& rng) const {
        std::uniform_real_distribution<double> dis(-1.0, 1.0);
        Vec3 p;
        do {
            p = Vec3(dis(rng), dis(rng), dis(rng));
        } while (p.length_squared() >= 1.0);
        return p;
    }
};

// AABB (Axis-Aligned Bounding Box) for acceleration
class AABB {
public:
    Vec3 minimum;
    Vec3 maximum;

    AABB() {}
    AABB(const Vec3& a, const Vec3& b) { 
        minimum = a; 
        maximum = b; 
    }

    Vec3 min() const { return minimum; }
    Vec3 max() const { return maximum; }

    bool hit(const Ray& r, double t_min, double t_max) const {
        for (int a = 0; a < 3; a++) {
            double invD = 1.0 / (&r.direction.x)[a];
            double t0 = ((&minimum.x)[a] - (&r.origin.x)[a]) * invD;
            double t1 = ((&maximum.x)[a] - (&r.origin.x)[a]) * invD;
            if (invD < 0.0) std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min) return false;
        }
        return true;
    }
};

// Hit record for ray-object intersections
struct HitRecord {
    Vec3 point;
    Vec3 normal;
    double t;
    bool front_face;
    Material material;
    double u, v; // Texture coordinates

    void set_face_normal(const Ray& ray, const Vec3& outward_normal) {
        front_face = ray.direction.dot(outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// Abstract base class for hittable objects
class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const = 0;
    virtual bool bounding_box(double time0, double time1, AABB& output_box) const = 0;
};

// Sphere class with advanced features
class Sphere : public Hittable {
public:
    Vec3 center;
    double radius;
    Material material;

    Sphere(const Vec3& cen, double r, const Material& m) 
        : center(cen), radius(r), material(m) {};

    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
        Vec3 oc = ray.origin - center;
        double a = ray.direction.dot(ray.direction);
        double half_b = oc.dot(ray.direction);
        double c = oc.dot(oc) - radius*radius;

        double discriminant = half_b*half_b - a*c;
        if (discriminant < 0) return false;
        double sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        double root = (-half_b - sqrtd) / a;
        if (root < t_min || t_max < root) {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || t_max < root)
                return false;
        }

        rec.t = root;
        rec.point = ray.at(rec.t);
        Vec3 outward_normal = (rec.point - center) / radius;
        rec.set_face_normal(ray, outward_normal);
        rec.material = material;
        
        // Texture coordinates (spherical mapping)
        double theta = std::acos(-outward_normal.y);
        double phi = std::atan2(-outward_normal.z, outward_normal.x) + M_PI;
        rec.u = phi / (2*M_PI);
        rec.v = theta / M_PI;

        return true;
    }

    virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
        output_box = AABB(
            center - Vec3(radius, radius, radius),
            center + Vec3(radius, radius, radius));
        return true;
    }
};

// Triangle class for mesh rendering
class Triangle : public Hittable {
public:
    Vec3 v0, v1, v2;
    Vec3 normal;
    Material material;

    Triangle(const Vec3& _v0, const Vec3& _v1, const Vec3& _v2, const Material& mat)
        : v0(_v0), v1(_v1), v2(_v2), material(mat) {
        // Calculate face normal
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        normal = edge1.cross(edge2).normalize();
    }

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 h = ray.direction.cross(edge2);
        double a = edge1.dot(h);

        if (a > -1e-6 && a < 1e-6) return false; // Ray is parallel to triangle

        double f = 1.0 / a;
        Vec3 s = ray.origin - v0;
        double u = f * s.dot(h);

        if (u < 0.0 || u > 1.0) return false;

        Vec3 q = s.cross(edge1);
        double v = f * ray.direction.dot(q);

        if (v < 0.0 || u + v > 1.0) return false;

        double t = f * edge2.dot(q);

        if (t > t_min && t < t_max) {
            rec.t = t;
            rec.point = ray.at(t);
            rec.normal = normal;
            rec.set_face_normal(ray, normal);
            rec.material = material;
            return true;
        }

        return false;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const override {
        Vec3 min_pt = Vec3(
            std::min({v0.x, v1.x, v2.x}),
            std::min({v0.y, v1.y, v2.y}),
            std::min({v0.z, v1.z, v2.z})
        );
        Vec3 max_pt = Vec3(
            std::max({v0.x, v1.x, v2.x}),
            std::max({v0.y, v1.y, v2.y}),
            std::max({v0.z, v1.z, v2.z})
        );
        output_box = AABB(min_pt, max_pt);
        return true;
    }
};

// Cube class with 12 triangles
class Cube : public Hittable {
private:
    std::vector<std::unique_ptr<Triangle>> triangles;

public:
    Cube(const Vec3& center, const Vec3& size, const Material& material) {
        Vec3 half_size = size * 0.5;
        Vec3 min_corner = center - half_size;
        Vec3 max_corner = center + half_size;

        // Define 8 vertices of the cube
        Vec3 vertices[8] = {
            Vec3(min_corner.x, min_corner.y, min_corner.z), // 0
            Vec3(max_corner.x, min_corner.y, min_corner.z), // 1
            Vec3(max_corner.x, max_corner.y, min_corner.z), // 2
            Vec3(min_corner.x, max_corner.y, min_corner.z), // 3
            Vec3(min_corner.x, min_corner.y, max_corner.z), // 4
            Vec3(max_corner.x, min_corner.y, max_corner.z), // 5
            Vec3(max_corner.x, max_corner.y, max_corner.z), // 6
            Vec3(min_corner.x, max_corner.y, max_corner.z)   // 7
        };

        // Define 12 triangles (2 per face)
        int indices[36] = {
            // Front face
            0, 1, 2, 0, 2, 3,
            // Back face
            5, 4, 7, 5, 7, 6,
            // Left face
            4, 0, 3, 4, 3, 7,
            // Right face
            1, 5, 6, 1, 6, 2,
            // Bottom face
            4, 5, 1, 4, 1, 0,
            // Top face
            3, 2, 6, 3, 6, 7
        };

        for (int i = 0; i < 36; i += 3) {
            triangles.push_back(std::make_unique<Triangle>(
                vertices[indices[i]], 
                vertices[indices[i+1]], 
                vertices[indices[i+2]], 
                material
            ));
        }
    }

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        double closest_so_far = t_max;

        for (const auto& triangle : triangles) {
            if (triangle->hit(ray, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const override {
        if (triangles.empty()) return false;
        
        AABB box;
        if (!triangles[0]->bounding_box(time0, time1, box)) return false;
        
        output_box = box;
        for (size_t i = 1; i < triangles.size(); i++) {
            if (!triangles[i]->bounding_box(time0, time1, box)) return false;
            // Simplified bounding box combination
            output_box = AABB(
                Vec3(
                    std::min(output_box.min().x, box.min().x),
                    std::min(output_box.min().y, box.min().y),
                    std::min(output_box.min().z, box.min().z)
                ),
                Vec3(
                    std::max(output_box.max().x, box.max().x),
                    std::max(output_box.max().y, box.max().y),
                    std::max(output_box.max().z, box.max().z)
                )
            );
        }
        
        return true;
    }
};

// Hittable List for scene management
class HittableList : public Hittable {
public:
    std::vector<std::unique_ptr<Hittable>> objects;

    HittableList() {}
    HittableList(std::unique_ptr<Hittable> object) { add(std::move(object)); }

    void clear() { objects.clear(); }

    void add(std::unique_ptr<Hittable> object) {
        objects.push_back(std::move(object));
    }

    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        double closest_so_far = t_max;

        for (const auto& object : objects) {
            if (object->hit(ray, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
        if (objects.empty()) return false;

        AABB temp_box;
        bool first_box = true;

        for (const auto& object : objects) {
            if (!object->bounding_box(time0, time1, temp_box)) return false;
            // Simplified bounding box combination
            if (first_box) {
                output_box = temp_box;
                first_box = false;
            } else {
                output_box = AABB(
                    Vec3(
                        std::min(output_box.min().x, temp_box.min().x),
                        std::min(output_box.min().y, temp_box.min().y),
                        std::min(output_box.min().z, temp_box.min().z)
                    ),
                    Vec3(
                        std::max(output_box.max().x, temp_box.max().x),
                        std::max(output_box.max().y, temp_box.max().y),
                        std::max(output_box.max().z, temp_box.max().z)
                    )
                );
            }
        }

        return true;
    }
};

// Camera class with advanced features
class Camera {
public:
    Vec3 origin;
    Vec3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
    Vec3 u, v, w;
    double lens_radius;
    double time0, time1; // shutter open/close times

    Camera(
        Vec3 lookfrom,
        Vec3 lookat,
        Vec3 vup,
        double vfov, // vertical field-of-view in degrees
        double aspect_ratio,
        double aperture,
        double focus_dist,
        double t0 = 0,
        double t1 = 0
    ) : time0(t0), time1(t1) {
        double theta = degrees_to_radians(vfov);
        double h = std::tan(theta/2);
        double viewport_height = 2.0 * h;
        double viewport_width = aspect_ratio * viewport_height;

        w = (lookfrom - lookat).normalize();
        u = vup.cross(w).normalize();
        v = w.cross(u);

        origin = lookfrom;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;

        lens_radius = aperture / 2;
    }

    Ray get_ray(double s, double t, std::mt19937& rng) const {
        Vec3 rd = lens_radius * random_in_unit_disk(rng);
        Vec3 offset = u * rd.x + v * rd.y;

        return Ray(
            origin + offset,
            lower_left_corner + s*horizontal + t*vertical - origin - offset
        );
    }

private:
    double degrees_to_radians(double degrees) const {
        return degrees * M_PI / 180.0;
    }

    Vec3 random_in_unit_disk(std::mt19937& rng) const {
        std::uniform_real_distribution<double> dis(-1.0, 1.0);
        Vec3 p;
        do {
            p = Vec3(dis(rng), dis(rng), 0);
        } while (p.length_squared() >= 1.0);
        return p;
    }
};

// Windows Display Window
class WindowsDisplay {
private:
    static HWND hwnd;
    static HBITMAP hBitmap;
    static HDC hdcMem;
    static BITMAPINFO bmi;
    static Color* framebuffer;
    static int width, height;

public:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch(msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                if (hBitmap) {
                    BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
                }
                
                EndPaint(hwnd, &ps);
                break;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            case WM_KEYDOWN:
                if (wParam == VK_ESCAPE) {
                    PostQuitMessage(0);
                }
                break;
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    static bool initialize_window(int w, int h) {
        width = w;
        height = h;
        
        HINSTANCE hInstance = GetModuleHandle(NULL);
        
        WNDCLASSA wc = {0}; // Use WNDCLASSA for ANSI
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = "RayTracerWindow"; // ANSI string
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        
        if (!RegisterClassA(&wc)) { // Use RegisterClassA
            return false;
        }
        
        hwnd = CreateWindowExA(
            0,
            "RayTracerWindow", // ANSI string
            "Professional Ray Tracer - Test Scene",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, w, h,
            NULL, NULL, hInstance, NULL
        );
        
        if (!hwnd) {
            return false;
        }
        
        // Create bitmap for rendering
        ZeroMemory(&bmi, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -h; // Top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;
        
        HDC hdc = GetDC(hwnd);
        hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&framebuffer, NULL, 0);
        hdcMem = CreateCompatibleDC(hdc);
        SelectObject(hdcMem, hBitmap);
        ReleaseDC(hwnd, hdc);
        
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
        
        return true;
    }

    static void update_framebuffer(const Color* render_buffer, int w, int h) {
        if (framebuffer && render_buffer) {
            for (int i = 0; i < w * h; i++) {
                // Convert to BGR format for Windows
                ((BYTE*)framebuffer)[i * 3 + 0] = (BYTE)render_buffer[i].to_int(render_buffer[i].b); // B
                ((BYTE*)framebuffer)[i * 3 + 1] = (BYTE)render_buffer[i].to_int(render_buffer[i].g); // G
                ((BYTE*)framebuffer)[i * 3 + 2] = (BYTE)render_buffer[i].to_int(render_buffer[i].r); // R
            }
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }

    static void show_image(const Color* render_buffer, int w, int h) {
        if (!initialize_window(w, h)) {
            std::cerr << "Failed to create window\n";
            return;
        }
        
        update_framebuffer(render_buffer, w, h);
        
        std::cout << "Displaying rendered image in window...\n";
        std::cout << "Press ESC or close window to exit.\n";
        
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
};

HWND WindowsDisplay::hwnd = NULL;
HBITMAP WindowsDisplay::hBitmap = NULL;
HDC WindowsDisplay::hdcMem = NULL;
BITMAPINFO WindowsDisplay::bmi = {0};
Color* WindowsDisplay::framebuffer = NULL;
int WindowsDisplay::width = 0;
int WindowsDisplay::height = 0;

// Advanced Ray Tracer with path tracing
class RayTracer {
private:
    HittableList world;
    Camera camera;
    int image_width;
    int image_height;
    int samples_per_pixel;
    int max_depth;
    mutable std::atomic<int> completed_rows{0}; // Make mutable
    Color* framebuffer;
    std::vector<std::mt19937> rngs;

public:
    RayTracer(int width, int height, int samples = 100, int max_depth = 50)
        : image_width(width), image_height(height), samples_per_pixel(samples), max_depth(max_depth),
          camera(Vec3(0,0,0), Vec3(0,0,-1), Vec3(0,1,0), 90, double(width)/height, 0.0, 1.0), // Initialize camera
          framebuffer(nullptr) {
        
        framebuffer = new Color[image_width * image_height];
        
        // Initialize random number generators for each thread
        for (unsigned int i = 0; i < std::thread::hardware_concurrency(); i++) {
            rngs.emplace_back(std::random_device{}());
        }
    }

    ~RayTracer() {
        delete[] framebuffer;
    }

    void set_camera(const Camera& cam) {
        camera = cam;
    }

    void set_world(HittableList&& w) {
        world = std::move(w);
    }

    Color ray_color(const Ray& ray, int depth, std::mt19937& rng) const {
        HitRecord rec;

        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return Color(0,0,0);

        // If the ray hits something
        if (world.hit(ray, 0.001, std::numeric_limits<double>::infinity(), rec)) {
            Ray scattered;
            Color attenuation;
            Color emitted = rec.material.albedo * rec.material.emission;

            // Sample new direction based on material
            Vec3 scattered_direction = rec.material.sample(ray.direction, rec.normal, rng);
            if (scattered_direction.length_squared() < 1e-8) return emitted;

            scattered = Ray(rec.point, scattered_direction);
            attenuation = rec.material.brdf(-ray.direction, scattered_direction, rec.normal);

            return emitted + attenuation * ray_color(scattered, depth-1, rng);
        }

        // Sky background with sun
        Vec3 unit_direction = ray.direction.normalize();
        double t = 0.5*(unit_direction.y + 1.0);
        
        // Sun direction (hardcoded for testing)
        Vec3 sun_direction = Vec3(-0.5, 0.8, -0.3).normalize();
        double sun_intensity = std::max(0.0, unit_direction.dot(sun_direction));
        Color sun_color = Color(1.0, 1.0, 0.9) * sun_intensity * 2.0;
        
        Color sky_color = (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
        return sky_color + sun_color;
    }

    void render_row(int j, int thread_id) const {
        std::mt19937& rng = const_cast<std::mt19937&>(rngs[thread_id % rngs.size()]); // Remove const
        
        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                double u = (i + std::uniform_real_distribution<double>(0.0, 1.0)(rng)) / (image_width-1);
                double v = (j + std::uniform_real_distribution<double>(0.0, 1.0)(rng)) / (image_height-1);
                Ray ray = camera.get_ray(u, v, rng);
                pixel_color = pixel_color + ray_color(ray, max_depth, rng);
            }
            pixel_color = pixel_color * (1.0 / samples_per_pixel);
            framebuffer[j * image_width + i] = pixel_color.clamp().gamma();
        }
        completed_rows++;
    }

    void render() {
        auto start_time = std::chrono::high_resolution_clock::now();
        int img_height = image_height; // Capture by value

        // Determine number of threads (max 3 cores)
        unsigned int num_threads = std::min(3u, std::thread::hardware_concurrency());
        if (num_threads == 0) num_threads = 1;

        std::vector<std::thread> threads;
        int rows_per_thread = image_height / num_threads;
        int remaining_rows = image_height % num_threads;

        std::cout << "Rendering with " << num_threads << " threads...\n";
        std::cout << "Resolution: " << image_width << "x" << image_height << "\n";
        std::cout << "Samples per pixel: " << samples_per_pixel << "\n";
        std::cout << "Max depth: " << max_depth << "\n";

        int current_row = 0;
        for (unsigned int t = 0; t < num_threads; ++t) {
            int rows_for_this_thread = rows_per_thread + (t < remaining_rows ? 1 : 0);
            int start_row = current_row;
            int end_row = start_row + rows_for_this_thread;

            threads.emplace_back([this, start_row, end_row, t]() {
                for (int j = start_row; j < end_row; ++j) {
                    render_row(image_height - 1 - j, t); // Flip image vertically
                }
            });

            current_row = end_row;
        }

        // Progress monitoring
        std::thread progress_thread([this, img_height]() {
            while (completed_rows < img_height) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                int progress = (completed_rows * 100) / img_height;
                std::cout << "\rProgress: " << progress << "% (" 
                          << completed_rows << "/" << img_height << " rows)" << std::flush;
            }
            std::cout << "\rProgress: 100% (" << img_height << "/" << img_height << " rows)\n";
        });

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
        progress_thread.join();

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "Render completed in " << duration.count() << " ms\n";
    }

    const Color* get_framebuffer() const { return framebuffer; }
    int get_width() const { return image_width; }
    int get_height() const { return image_height; }
};

// Create test scene with spheres, cubes, and virtual sun
HittableList create_test_scene() {
    HittableList world;

    // Materials
    Material lambertian_red(Material::LAMBERTIAN, Color(0.65, 0.05, 0.05));
    Material lambertian_green(Material::LAMBERTIAN, Color(0.12, 0.45, 0.15));
    Material metal(Material::METAL, Color(0.8, 0.8, 0.8), 0.0, 0.0, 0.0, 0.1);
    Material glass(Material::DIELECTRIC, Color(1.0, 1.0, 1.0), 0.0, 0.0, 0.0, 0.0, 1.5);
    Material light(Material::DIFFUSE_LIGHT, Color(1.0, 1.0, 1.0), 0.0, 0.0, 5.0);
    Material blue_plastic(Material::LAMBERTIAN, Color(0.1, 0.2, 0.8), 0.3, 0.0, 0.0, 0.0, 1.5, 0.8);

    // Ground plane (large sphere)
    world.add(std::make_unique<Sphere>(Vec3(0, -1000, 0), 1000, lambertian_green));

    // Test objects
    world.add(std::make_unique<Sphere>(Vec3(0, 1, 0), 1.0, glass));           // Glass sphere
    world.add(std::make_unique<Sphere>(Vec3(-4, 1, 0), 1.0, lambertian_red));  // Red sphere
    world.add(std::make_unique<Sphere>(Vec3(4, 1, 0), 1.0, metal));            // Metal sphere
    
    // Cube
    world.add(std::make_unique<Cube>(Vec3(0, 1, 3), Vec3(1.5, 1.5, 1.5), blue_plastic));

    // Light source
    world.add(std::make_unique<Sphere>(Vec3(0, 5, 0), 0.5, light));

    return world;
}

int main() {
    try {
        std::cout << "Professional Physically Based Ray Tracer - Test Scene\n";
        std::cout << "===================================================\n";
        std::cout << "Scene: Spheres, Cube, Virtual Sun, Glass, Metal\n";
        std::cout << "Features: Path tracing, Physically based materials\n\n";

        // Configuration
        const int image_width = 800;
        const int image_height = 600;
        const int samples_per_pixel = 200;
        const int max_depth = 50;

        // Create ray tracer
        RayTracer tracer(image_width, image_height, samples_per_pixel, max_depth);

        // Set up camera
        Vec3 lookfrom(8, 3, 3);
        Vec3 lookat(0, 1, 0);
        Vec3 vup(0, 1, 0);
        double dist_to_focus = (lookfrom - lookat).length();
        double aperture = 0.1;
        Camera cam(lookfrom, lookat, vup, 20, double(image_width)/image_height, aperture, dist_to_focus);
        tracer.set_camera(cam);

        // Set up test scene
        HittableList world = create_test_scene();
        tracer.set_world(std::move(world));

        // Render
        std::cout << "Starting ray tracing of test scene...\n";
        tracer.render();

        // Show result in Windows window
        WindowsDisplay::show_image(tracer.get_framebuffer(), tracer.get_width(), tracer.get_height());

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
