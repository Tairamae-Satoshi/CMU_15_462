#include "bvh.h"

#include "CMU462/CMU462.h"
#include "static_scene/triangle.h"

#include <stack>

using namespace std;

namespace CMU462 {
	namespace StaticScene {


		BVHAccel::BVHAccel(const std::vector<Primitive*>& _primitives, size_t max_leaf_size)
			:maxPrimsInNode(max_leaf_size) {
			this->primitives = _primitives;

			// (PathTracer):
			// Construct a BVH from the given vector of primitives and maximum leaf
			// size configuration. The starter code build a BVH aggregate with a
			// single leaf node (which is also the root) that encloses all the
			// primitives.

			// Initialize primitiveInfo array for primitives
			std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
			for (size_t i = 0; i < primitives.size(); i++)
			{
				primitiveInfo[i] = BVHPrimitiveInfo(i, primitives[i]->get_bbox());
			}

			// Build BVH tree for primitives using primitiveInfo
			std::vector<Primitive*> orderedPrims;
			root = recursiveBuild(primitiveInfo, 0, primitives.size(), orderedPrims);
			primitives.swap(orderedPrims);

		}


		BVHAccel::~BVHAccel() {
			// (PathTracer):
			// Implement a proper destructor for your BVH accelerator aggregate
			recursiveDelete(root);
			root = nullptr;

		}

		BBox BVHAccel::get_bbox() const { return root->bb; }

		void BVHAccel::find_closest_hit(const Ray& ray, BVHNode* node, Intersection* closest, double t0, double t1) const
		{
			if (node->isLeaf())
			{
				for (size_t i = node->start; i < node->start + node->range; i++)
				{
					primitives[i]->intersect(ray, closest);
				}
			}
			else
			{
				double t10 = t0, t11 = t1, t20 = t0, t21 = t1;
				bool hit1 = node->l->bb.intersect(ray, t10, t11);
				bool hit2 = node->r->bb.intersect(ray, t20, t21);

				if (hit1 && !hit2)
				{
					find_closest_hit(ray, node->l, closest, t10, t11);
				}
				else if (!hit1 && hit2)
				{
					find_closest_hit(ray, node->r, closest, t20, t21);
				}
				else if (hit1 && hit2)
				{
					BVHNode* first = t10 <= t20 ? node->l : node->r;
					BVHNode* second = t10 <= t20 ? node->r : node->l;

					find_closest_hit(ray, node->l, closest, t10, t11);
					if (t20 < closest->t)
					{
						find_closest_hit(ray, node->r, closest, t20, t21);
					}
				}				
			}
		}

		bool BVHAccel::intersect(const Ray& ray) const {
			// TODO (PathTracer):
			// Implement ray - bvh aggregate intersection test. A ray intersects
			// with a BVH aggregate if and only if it intersects a primitive in
			// the BVH that is not an aggregate.
			Intersection isect;
			double t0 = ray.min_t, t1 = ray.max_t;

			if (root->bb.intersect(ray, t0, t1))
			{
				find_closest_hit(ray, root, &isect, t0, t1);
				if (isect.primitive != NULL)
				{
					return true;
				}
			}

			return false;
		}

		bool BVHAccel::intersect(const Ray& ray, Intersection* isect) const {
			// (PathTracer):
			// Implement ray - bvh aggregate intersection test. A ray intersects
			// with a BVH aggregate if and only if it intersects a primitive in
			// the BVH that is not an aggregate. When an intersection does happen.
			// You should store the non-aggregate primitive in the intersection data
			// and not the BVH aggregate itself.

			double t0 = ray.min_t, t1 = ray.max_t;

			if (root->bb.intersect(ray, t0, t1))
			{

				find_closest_hit(ray, root, isect, t0, t1);
				if (isect->primitive!=nullptr)
				{
					return true;
				}
			}

			return false;
		}

	}  // namespace StaticScene
}  // namespace CMU462
