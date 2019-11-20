#include "sphere.h"

#include <cmath>

#include "../bsdf.h"
#include "../misc/sphere_drawing.h"

namespace CMU462 {
	namespace StaticScene {

		bool Sphere::test(const Ray& r, double& t1, double& t2) const {
			// TODO (PathTracer):
			// Implement ray - sphere intersection test.
			// Return true if there are intersections and writing the
			// smaller of the two intersection times in t1 and the larger in t2.
			// dot(p-C,p-C)=R*R
			// dot((A+t∗B−C),(A+t∗B−C))=R*R
			// t2⋅dot(B,B)+2t⋅dot(B,A−C)+dot(A−C,A−C)−R*R=0
			
			Vector3D oc = r.o - o;
			double a = dot(r.d, r.d);
			double b = dot(oc, r.d);
			double c = dot(oc, oc) - r2;
			float discriminant = b * b - a * c;
			if (discriminant<0)
			{
				return false;
			}

			t1 = (-b - sqrt(discriminant)) / a;
			t2 = (-b + sqrt(discriminant)) / a;
			return true;
		}

		bool Sphere::intersect(const Ray& r) const {
			// TODO (PathTracer):
			// Implement ray - sphere intersection.
			// Note that you might want to use the the Sphere::test helper here.
			double t1, t2;
			
			if (test(r, t1, t2))
			{
				if (t1 > r.min_t && t1 < r.max_t)
				{
					r.max_t = t1;
					return true;
				}
				else if (t2 > r.min_t && t2 < r.max_t)
				{
					r.max_t = t2;
					return true;
				}
			}
			return false;
		}

		bool Sphere::intersect(const Ray& r, Intersection* isect) const {
			// TODO (PathTracer):
			// Implement ray - sphere intersection.
			// Note again that you might want to use the the Sphere::test helper here.
			// When an intersection takes place, the Intersection data should be updated
			// correspondingly.
			double t1, t2;
			if (test(r, t1, t2))
			{
				float t;
				if (t1 > r.min_t && t1 < r.max_t)
				{
					t = t1;
					r.max_t = t1;
					isect->t = t1;
					isect->primitive = this;
					isect->n = normal(r.at_time(t1));
					isect->bsdf = get_bsdf();

					return true;
				}
				else if (t2 > r.min_t && t2 < r.max_t)
				{
					t = t2;
					r.max_t = t2;
					isect->t = t2;
					isect->primitive = this;
					isect->n = normal(r.at_time(t2));
					isect->bsdf = get_bsdf();


					return true;
				}

				return true;
			}
			return false;
		}

		void Sphere::draw(const Color& c) const { Misc::draw_sphere_opengl(o, r, c); }

		void Sphere::drawOutline(const Color& c) const {
			// Misc::draw_sphere_opengl(o, r, c);
		}

	}  // namespace StaticScene
}  // namespace CMU462
