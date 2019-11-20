#include "triangle.h"

#include "CMU462/CMU462.h"
#include "GL/glew.h"

namespace CMU462 {
	namespace StaticScene {

		Triangle::Triangle(const Mesh* mesh, vector<size_t>& v) : mesh(mesh), v(v) { v1 = v[0]; v2 = v[1]; v3 = v[2]; }
		Triangle::Triangle(const Mesh* mesh, size_t v1, size_t v2, size_t v3)
			: mesh(mesh), v1(v1), v2(v2), v3(v3) {}

		BBox Triangle::get_bbox() const {
			// (PathTracer):
			// compute the bounding box of the triangle
			BBox bb;

			bb.expand(mesh->positions[v1]);
			bb.expand(mesh->positions[v2]);
			bb.expand(mesh->positions[v3]);

			return bb;
		}

		bool Triangle::intersect(const Ray& r) const {
			// TODO (PathTracer): implement ray-triangle intersection
			Vector3D p0 = mesh->getPosition(v1);
			Vector3D p1 = mesh->getPosition(v2);
			Vector3D p2 = mesh->getPosition(v3);
			Vector3D o = r.o;
			Vector3D d = r.d;
			Vector3D e1 = p1 - p0;
			Vector3D e2 = p2 - p0;
			Vector3D s = o - p0;
			Vector3D e1_cross_d = cross(e1, d);
			double det = dot(e1_cross_d, e2);
			if (det < 0.000001)
			{
				return false;
			}
			Vector3D s_cross_e2 = cross(s, e2);
			double det1 = -dot(s_cross_e2, d);
			double det2 = dot(e1_cross_d, s);
			double det3 = -dot(s_cross_e2, e1);
			double u = det1 / det;
			double v = det2 / det;
			double t = det3 / det;

			if (u > 1 || u < 0 || v>1 || v < 0 || (1 - u - v)>1 || (1 - u - v) < 0)
			{
				return false;
			}
			if (t<r.min_t || t>r.max_t)
			{
				return false;
			}
			r.max_t = t;

			return true;
		}

		bool Triangle::intersect(const Ray& r, Intersection* isect) const {
			// TODO (PathTracer):
			// implement ray-triangle intersection. When an intersection takes
			// place, the Intersection data should be updated accordingly
			Vector3D p0 = mesh->getPosition(v1);
			Vector3D p1 = mesh->getPosition(v2);
			Vector3D p2 = mesh->getPosition(v3);
			Vector3D o = r.o;
			Vector3D d = r.d;
			Vector3D e1 = p1 - p0;
			Vector3D e2 = p2 - p0;
			Vector3D s = o - p0;
			Vector3D e1_cross_d = cross(e1, d);
			double det = dot(e1_cross_d, e2);
			if (det < 0.000001)
			{
				return false;
			}
			Vector3D s_cross_e2 = cross(s, e2);
			double det1 = -dot(s_cross_e2, d);
			double det2 = dot(e1_cross_d, s);
			double det3 = -dot(s_cross_e2, e1);
			double u = det1 / det;
			double v = det2 / det;
			double t = det3 / det;

			if (u > 1 || u < 0 || v>1 || v < 0 || (1 - u - v)>1 || (1 - u - v) < 0)
			{
				return false;
			}
			if (t<r.min_t || t>r.max_t)
			{
				return false;
			}

			r.max_t = t;

			Vector3D N = (1 - u - v) * mesh->normals[v1] + u * mesh->normals[v2] + v * mesh->normals[v3];
			isect->t = t;
			isect->primitive = this;
			isect->n = dot(d, N) <= 0 ? N : -N;
			isect->bsdf = get_bsdf();

			return true;
		}

		void Triangle::draw(const Color& c) const {
			glColor4f(c.r, c.g, c.b, c.a);
			glBegin(GL_TRIANGLES);
			glVertex3d(mesh->positions[v1].x, mesh->positions[v1].y,
				mesh->positions[v1].z);
			glVertex3d(mesh->positions[v2].x, mesh->positions[v2].y,
				mesh->positions[v2].z);
			glVertex3d(mesh->positions[v3].x, mesh->positions[v3].y,
				mesh->positions[v3].z);
			glEnd();
		}

		void Triangle::drawOutline(const Color& c) const {
			glColor4f(c.r, c.g, c.b, c.a);
			glBegin(GL_LINE_LOOP);
			glVertex3d(mesh->positions[v1].x, mesh->positions[v1].y,
				mesh->positions[v1].z);
			glVertex3d(mesh->positions[v2].x, mesh->positions[v2].y,
				mesh->positions[v2].z);
			glVertex3d(mesh->positions[v3].x, mesh->positions[v3].y,
				mesh->positions[v3].z);
			glEnd();
		}

	}  // namespace StaticScene
}  // namespace CMU462
