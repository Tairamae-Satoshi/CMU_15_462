#ifndef CMU462_BVH_H
#define CMU462_BVH_H
#include<iostream>
#include "static_scene/scene.h"
#include "static_scene/aggregate.h"
#include "bbox.h"
#include <vector>

namespace CMU462 {
	namespace StaticScene {

		/**
		 * A node in the BVH accelerator aggregate.
		 * The accelerator uses a "flat tree" structure where all the primitives are
		 * stored in one vector. A node in the data structure stores only the starting
		 * index and the number of primitives in the node and uses this information to
		 * index into the primitive vector for actual data. In this implementation all
		 * primitives (index + range) are stored on leaf nodes. A leaf node has no child
		 * node and its range should be no greater than the maximum leaf size used when
		 * constructing the BVH.
		 */
		struct BVHNode {
			BVHNode() { bb = BBox(); start = 0; range = -1; l = nullptr; r = nullptr; }
			BVHNode(BBox bb, size_t start, size_t range)
				: bb(bb), start(start), range(range), l(nullptr), r(nullptr) {}

			inline bool isLeaf() const { return l == nullptr && r == nullptr; }

			/**
			* Initiate leaf node
			*/
			void InitLeaf(int first, int n, const BBox& b) {
				start = first;
				range = n;
				bb = b;
				l = r = nullptr;
			}

			/**
			* Initiate interior node
			*/
			void InitInterior(int axis, BVHNode* c0, BVHNode* c1, size_t start, size_t range) {
				l = c0;
				r = c1;
				bb = c0->bb;
				bb.expand(c1->bb);
				this->start = start;
				this->range = range;
			}

			BBox bb;       ///< bounding box of the node
			size_t start;  ///< start index into the primitive list
			size_t range;  ///< range of index into the primitive list
			BVHNode* l;    ///< left child node
			BVHNode* r;    ///< right child node
		};

		/**
		 * For each primitive to be stored in the BVH, we store the centroid of its bounding box,
		 * its complete bounding box, and its index in the primitives array in an instance of the
		 * BVHPrimitiveInfo structure.
		 */
		struct BVHPrimitiveInfo {
			BVHPrimitiveInfo() {};
			BVHPrimitiveInfo(size_t primitiveNumber, const BBox& bounds)
				: primitiveNumber(primitiveNumber), bounds(bounds), centroid(.5f * bounds.min + .5f * bounds.max) {}
			size_t primitiveNumber;
			BBox bounds;
			Vector3D centroid;
		};

		/**
		 * Bounding Volume Hierarchy for fast Ray - Primitive intersection.
		 * Note that the BVHAccel is an Aggregate (A Primitive itself) that contains
		 * all the primitives it was built from. Therefore once a BVHAccel Aggregate
		 * is created, the original input primitives can be ignored from the scene
		 * during ray intersection tests as they are contained in the aggregate.
		 */
		class BVHAccel : public Aggregate {
		public:
			BVHAccel() {}

			/**
			 * Parameterized Constructor.
			 * Create BVH from a list of primitives. Note that the BVHAccel Aggregate
			 * stores pointers to the primitives and thus the primitives need be kept
			 * in memory for the aggregate to function properly.
			 * \param primitives primitives to build from
			 * \param max_leaf_size maximum number of primitives to be stored in leaves
			 */
			BVHAccel(const std::vector<Primitive*>& primitives, size_t max_leaf_size = 4);

			/**
			 * Destructor.
			 * The destructor only destroys the Aggregate itself, the primitives that
			 * it contains are left untouched.
			 */
			~BVHAccel();

			/**
			 * Get the world space bounding box of the aggregate.
			 * \return world space bounding box of the aggregate
			 */
			BBox get_bbox() const;

			//void find_closest_hit(const Ray& ray, BVHNode* node);

			void find_closest_hit(const Ray& ray, BVHNode* node, Intersection* closest, double t0, double t1) const;

			/**
			 * Ray - Aggregate intersection.
			 * Check if the given ray intersects with the aggregate (any primitive in
			 * the aggregate), no intersection information is stored.
			 * \param r ray to test intersection with
			 * \return true if the given ray intersects with the aggregate,
					   false otherwise
			 */
			bool intersect(const Ray& r) const;

			/**
			 * Ray - Aggregate intersection 2.
			 * Check if the given ray intersects with the aggregate (any primitive in
			 * the aggregate). If so, the input intersection data is updated to contain
			 * intersection information for the point of intersection. Note that the
			 * intersected primitive entry in the intersection should be updated to
			 * the actual primitive in the aggregate that the ray intersected with and
			 * not the aggregate itself.
			 * \param r ray to test intersection with
			 * \param i address to store intersection info
			 * \return true if the given ray intersects with the aggregate,
					   false otherwise
			 */
			bool intersect(const Ray& r, Intersection* i) const;

			/**
			 * Get BSDF of the surface material
			 * Note that this does not make sense for the BVHAccel aggregate
			 * because it does not have a surface material. Therefore this
			 * should always return a null pointer.
			 */
			BSDF* get_bsdf() const { return NULL; }

			/**
			 * Get entry point (root) - used in visualizer
			 */
			BVHNode* get_root() const { return root; }

			/**
			 * Draw the BVH with OpenGL - used in visualizer
			 */
			void draw(const Color& c) const {}

			/**
			 * Draw the BVH outline with OpenGL - used in visualizer
			 */
			void drawOutline(const Color& c) const {}

			/**
			 * recursively build BVH node
			 */
			BVHNode* recursiveBuild(std::vector<BVHPrimitiveInfo>& primitiveInfo,
				int start, int end, std::vector<Primitive*>& orderedPrims) {
				BVHNode* node = new BVHNode();
				// Compute bounds of all primitives in BVH node
				BBox bounds;
				for (int i = start; i < end; ++i) {
					bounds.expand(primitiveInfo[i].bounds);
				}

				int firstPrimOffset = orderedPrims.size();
				int nPrimitives = end - start;
				if (nPrimitives <= maxPrimsInNode) {
					// Create leaf BVHNode
					for (int i = start; i < end; ++i) {
						int primNum = primitiveInfo[i].primitiveNumber;
						orderedPrims.push_back(primitives[primNum]);
					}	
					node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
				}
				else {
					// Compute bound of primitive centroids
					BBox centroidBounds;
					for (int i = start; i < end; ++i)
						centroidBounds.expand(primitiveInfo[i].centroid);

					// Partition primitives into two sets and build children
					int mid;
					float minCost[3];
					int minCostSplitBucket[3];
					constexpr int nBuckets = 12;

					for (int dim = 0; dim < 3; ++dim) {

						// Allocate BucketInfor for SAH partition buckets
						struct BucketInfo {
							int count = 0;
							BBox bounds;
						};
						BucketInfo buckets[nBuckets];

						// Initialize BucketInfo for SAH partition buckets
						for (int i = start; i < end; ++i)
						{
							int b = nBuckets * centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
							if (b == nBuckets) {
								b = nBuckets - 1;
							}
							buckets[b].count++;
							buckets[b].bounds.expand(primitiveInfo[i].bounds);
						}

						// Compute costs for splitting after each bucket
						float cost[nBuckets - 1];
						for (int i = 0; i < nBuckets - 1; i++)
						{
							BBox b0, b1;
							int count0 = 0, count1 = 0;
							for (int j = 0; j <= i; j++)
							{
								b0.expand(buckets[j].bounds);
								count0 += buckets[j].count;
							}
							for (int j = i + 1; j < nBuckets; j++)
							{
								b1.expand(buckets[j].bounds);
								count1 += buckets[j].count;
							}
							cost[i] = .125f + (count0 * b0.surface_area() + count1 * b1.surface_area()) / bounds.surface_area();
						}

						// Find bucket to split at that minimizes SAH metric
						minCost[dim] = cost[0];
						minCostSplitBucket[dim] = 0;
						for (int i = 1; i < nBuckets - 1; i++)
						{
							if (cost[i] < minCost[dim])
							{
								minCost[dim] = cost[i];
								minCostSplitBucket[dim] = i;
							}
						}
					}

					int dim;
					if (minCost[0] < minCost[1] && minCost[0] < minCost[2])
						dim = 0;
					else if (minCost[1] < minCost[2])
						dim = 1;
					else
						dim = 2;

					// Either create leaf or split primitives at selected SAH bucket
					float leafCost = nPrimitives;
					if (minCost[dim] < leafCost)
					{
						BVHPrimitiveInfo* pmid = std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
							[=](const BVHPrimitiveInfo& pi) {
								int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
								if (b == nBuckets)
								{
									b = nBuckets - 1;
								}
								return b <= minCostSplitBucket[dim];
							});
						mid = pmid - &primitiveInfo[0];
						node->InitInterior(dim,
							recursiveBuild(primitiveInfo, start, mid, orderedPrims),
							recursiveBuild(primitiveInfo, mid, end, orderedPrims), firstPrimOffset, nPrimitives);
					}
					else
					{
						// Create leaf BVHBuildNode
						for (int i = start; i < end; i++)
						{
							int primNum = primitiveInfo[i].primitiveNumber;
							orderedPrims.push_back(primitives[primNum]);
						}
						node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
					}
				}

				return node;

			}

			void recursiveDelete(BVHNode *node)
			{
				if (!node->isLeaf())
				{
					recursiveDelete(node->l);
					recursiveDelete(node->r);
				}				
				delete node;				
			}

		private:
			BVHNode* root;  ///< root node of the BVH
			int maxPrimsInNode;
		};

	}  // namespace StaticScene
}  // namespace CMU462

#endif  // CMU462_BVH_H
