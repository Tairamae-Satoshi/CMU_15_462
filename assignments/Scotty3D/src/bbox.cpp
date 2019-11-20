#include "bbox.h"

#include "GL/glew.h"

#include <algorithm>
#include <iostream>

namespace CMU462 {

	bool BBox::intersect(const Ray& r, double& t0, double& t1) const {
		// (PathTracer):
		// Implement ray - bounding box intersection test
		// If the ray intersected the bounding box within the range given by
		// t0, t1, update t0 and t1 with the new intersection times.
		for (int i = 0; i < 3; i++)
		{
			float ta = (min[i] - r.o[i]) * r.inv_d[i];
			float tb = (max[i] - r.o[i]) * r.inv_d[i];
			if (r.inv_d[i] < 0.0f)
			{
				std::swap(ta, tb);
			}
			t0 = ta > t0 ? ta : t0;
			t1 = tb < t1 ? tb : t1;
			if (t1 < t0)
			{
				return false;
			}
		}
		return true;
	}

	void BBox::draw(Color c) const {
		glColor4f(c.r, c.g, c.b, c.a);

		// top
		glBegin(GL_LINE_STRIP);
		glVertex3d(max.x, max.y, max.z);
		glVertex3d(max.x, max.y, min.z);
		glVertex3d(min.x, max.y, min.z);
		glVertex3d(min.x, max.y, max.z);
		glVertex3d(max.x, max.y, max.z);
		glEnd();

		// bottom
		glBegin(GL_LINE_STRIP);
		glVertex3d(min.x, min.y, min.z);
		glVertex3d(min.x, min.y, max.z);
		glVertex3d(max.x, min.y, max.z);
		glVertex3d(max.x, min.y, min.z);
		glVertex3d(min.x, min.y, min.z);
		glEnd();

		// side
		glBegin(GL_LINES);
		glVertex3d(max.x, max.y, max.z);
		glVertex3d(max.x, min.y, max.z);
		glVertex3d(max.x, max.y, min.z);
		glVertex3d(max.x, min.y, min.z);
		glVertex3d(min.x, max.y, min.z);
		glVertex3d(min.x, min.y, min.z);
		glVertex3d(min.x, max.y, max.z);
		glVertex3d(min.x, min.y, max.z);
		glEnd();
	}

	std::ostream& operator<<(std::ostream& os, const BBox& b) {
		return os << "BBOX(" << b.min << ", " << b.max << ")";
	}

}  // namespace CMU462
