#include <float.h>
#include <assert.h>
#include "meshEdit.h"
#include "mutablePriorityQueue.h"
#include "error_dialog.h"

namespace CMU462 {

	bool HalfedgeMesh::isValid(EdgeIter e)
	{
		return e == edgesEnd() ? false : true;
	}


	HalfedgeIter HalfedgeMesh::findHalfedgeFrom(VertexIter v0, FaceIter f0)
	{
		// Don't check whether v0 is on f0
		HalfedgeIter h = f0->halfedge();
		do
		{
			if (h->vertex() == v0)
			{
				return h;
			}
		} while (h != f0->halfedge());
	}


	HalfedgeIter HalfedgeMesh::findHalfedgeTo(VertexIter v0, FaceIter f0)
	{
		// Don't check whether v0 is on f0
		HalfedgeIter h = f0->halfedge();
		do
		{
			if (h->next()->vertex() == v0)
			{
				return h;
			}
		} while (h != f0->halfedge());
	}


	EdgeIter HalfedgeMesh::connectVertex(HalfedgeIter h_v0, HalfedgeIter h_v1)
	{
		// Connect two vertices in the same face, returning a pointer to the new edge
		FaceIter f0 = h_v0->face();
		FaceIter f1 = h_v1->face();
		if (f0 != f1 || f0->degree() == 3)
		{
			return edgesEnd();
		}
		VertexIter v0 = h_v0->vertex();
		VertexIter v1 = h_v1->vertex();

		HalfedgeIter h_from_v0 = h_v0;
		HalfedgeIter h_to_v0 = h_v0->pre();
		HalfedgeIter h_from_v1 = h_v1;
		HalfedgeIter h_to_v1 = h_v1->pre();

		f1 = newFace();
		HalfedgeIter h0 = newHalfedge();
		HalfedgeIter h1 = newHalfedge();
		EdgeIter e0 = newEdge();
		e0->isNew = true;

		h0->twin() = h1;
		h0->next() = h_from_v1;
		h0->vertex() = v0;
		h0->edge() = e0;
		h0->face() = f1;

		h1->twin() = h0;
		h1->next() = h_from_v0;
		h1->vertex() = v1;
		h1->edge() = e0;
		h1->face() = f0;

		h_to_v0->next() = h0;
		h_to_v1->next() = h1;

		e0->halfedge() = h0;

		f0->halfedge() = h1;
		f1->halfedge() = h0;

		HalfedgeIter h = h_from_v1;
		do
		{
			h->face() = f1;
			h = h->next();
		} while (h != f1->halfedge());

		return e0;
	}

	VertexIter HalfedgeMesh::insertVertice(double ratio, HalfedgeIter h)
	{
		/*
		After:
		--------------.--------------
		--->          |/\        --->
		h0_pre	      |h1_new	 h1_next
					  e0
					h0|
					\/|
					  .v0(inserted point)
					  |/\
					  |h1
					  e1
		h0_next	h0_new|		   h1_pre
		<---	    \/|          <---
		------------- .--------------

		*/
		EdgeIter e0 = h->edge();
		HalfedgeIter h0 = e0->halfedge();
		HalfedgeIter h0_next = h0->next();
		HalfedgeIter h1 = h0->twin();
		HalfedgeIter h1_next = h1->next();
		FaceIter f0 = h0->face();
		FaceIter f1 = h1->face();

		HalfedgeIter h0_new = newHalfedge();
		HalfedgeIter h1_new = newHalfedge();
		EdgeIter e1 = newEdge();
		VertexIter v0 = newVertex();
		v0->position = h->vertex()->position * (1 - ratio) + h->twin()->vertex()->position * ratio;

		h0->twin() = h1_new;
		h0->next() = h0_new;

		h1->twin() = h0_new;
		h1->next() = h1_new;
		h1->edge() = e1;

		h0_new->twin() = h1;
		h0_new->next() = h0_next;
		h0_new->vertex() = v0;
		h0_new->edge() = e1;
		h0_new->face() = f0;

		h1_new->twin() = h0;
		h1_new->next() = h1_next;
		h1_new->vertex() = v0;
		h1_new->edge() = e0;
		h1_new->face() = f1;

		v0->halfedge() = h1_new;

		e1->halfedge() = h1;

		return v0;
	}

	VertexIter HalfedgeMesh::insertVertice(HalfedgeIter hv0)
	{
		/*
			After:
						  v0
			--------------.--------------
			<-h_from_v0   |<-h_to_v0
						/\|
				 h_from_v1|
						  |
						  |h_to_v1
						  |\/
						  .
						  v1
		*/

		HalfedgeIter h_from_v0 = hv0;
		HalfedgeIter h_to_v0 = h_from_v0->pre();
		VertexIter v0 = h_from_v0->vertex();
		FaceIter f0 = h_from_v0->face();

		HalfedgeIter h_from_v1 = newHalfedge();
		HalfedgeIter h_to_v1 = newHalfedge();
		VertexIter v1 = newVertex();
		v1->position = v0->position;
		EdgeIter e0 = newEdge();

		h_from_v1->twin() = h_to_v1;
		h_from_v1->next() = h_from_v0;
		h_from_v1->vertex() = v1;
		h_from_v1->edge() = e0;
		h_from_v1->face() = f0;

		h_to_v1->twin() = h_from_v1;
		h_to_v1->next() = h_from_v1;
		h_to_v1->vertex() = v0;
		h_to_v1->edge() = e0;
		h_to_v1->face() = f0;

		h_to_v0->next() = h_to_v1;

		v1->halfedge() = h_from_v1;

		e0->halfedge() = h_from_v1;

		return v1;
	};

	EdgeIter HalfedgeMesh::findEdge(VertexIter v0, VertexIter v1)
	{
		HalfedgeIter h = v0->halfedge();
		do
		{
			HalfedgeIter h_twin = h->twin();
			if (h_twin->vertex()==v1)
			{
				return h_twin->edge();
			}
			h = h_twin->next();
		} while (h != v0->halfedge());

		h = v1->halfedge();
		do
		{
			HalfedgeIter h_twin = h->twin();
			if (h_twin->vertex() == v0)
			{
				return h_twin->edge();
			}
			h = h_twin->next();
		} while (h != v1->halfedge());

		showError("FindEdge: Cannot find any edge that connect v0 and v1.", true);
		return edgesEnd();
	};


	VertexIter HalfedgeMesh::splitEdge(EdgeIter e0) {
		// (meshEdit)
		// This method should split the given edge and return an iterator to the
		// newly inserted vertex. The halfedge of this vertex should point along
		// the edge that was split, rather than the new edges.
		HalfedgeIter h0 = e0->halfedge()->pre();
		HalfedgeIter h1 = e0->halfedge()->twin()->pre();

		if (!(h0->face()->degree() == 3 && h1->face()->degree() == 3))
		{
			showError("One side of this edge is not triangle.", true);
			return verticesEnd();
		}

		VertexIter v0 = insertVertice(0.5, e0->halfedge());
		v0->isNew = true;

		HalfedgeIter h2 = h0->next()->next();
		HalfedgeIter h3 = h1->next()->next();
		if (!h2->isBoundary())
		{
			connectVertex(h0, h2);
		}
		if (!h3->isBoundary())
		{
			connectVertex(h1, h3);
		}

		return v0;
	}

	VertexIter HalfedgeMesh::collapseEdge(EdgeIter e) {
		// This method should collapse the given edge and return an iterator to
		// the new vertex created by the collapse.

		// Do not consider the boundary condition
		
		if (e->isBoundary())
		{
			showError("This edge is boundary.");
			return verticesEnd();
		}

		if (e->isBridge())
		{
			showError("This edge is a bridge.");
			return verticesEnd();
		}

		VertexIter v0 = insertVertice(0.5, e->halfedge());
		HalfedgeIter h1 = v0->halfedge();
		HalfedgeIter h2 = h1->twin()->next();
		HalfedgeIter h1_next = h1->next();
		HalfedgeIter h2_next = h2->next();
		HalfedgeIter h1_twin = h1->twin();
		HalfedgeIter h2_twin = h2->twin();
		VertexIter v1 = h1_twin->vertex();
		VertexIter v2 = h2_twin->vertex();
		vector<Index> indices;
		
		connectVertex(h1, h1_next->next());
		Index indice = 0;
		indices.push_back(indice);
		HalfedgeIter h1_end = h1_twin;
		for (HalfedgeIter h = h1_next->twin()->next(); h != h1_end; h = h->twin()->next())
		{
			indice += h->face()->degree() - 2;
			indices.push_back(indice);
		}

		if (h2->face()->degree()!=4)
		{
			indice += h2->face()->degree() - 4;
			indices.push_back(indice);
		}
		Index mark = indice;
		HalfedgeIter h2_end = h2->twin();
		for (HalfedgeIter h = h2_next->twin()->next(); h != h2_end; h = h->twin()->next())
		{
			indice += h->face()->degree() - 2;
			indices.push_back(indice);
		}

		eraseVertex(v1);
		FaceIter f = eraseVertex(v2);

		HalfedgeIter h = f->halfedge();
		do {
			if (h->next()==h->twin())
			{
				break;
			}
			h = h->next();
		} while (h != f->halfedge());
		v0 = h->next()->vertex();

		vector<HalfedgeIter> hs_f;
		for (HalfedgeIter h_f = h->next()->next(); h_f != h->next(); h_f = h_f->next())
		{
			hs_f.push_back(h_f);
		} 
		
		if (indices.back()== hs_f.size()-1)
		{
			indices.pop_back();
		}

		HalfedgeIter h_from_v0 = v0->halfedge();
		reverse(hs_f.begin(), hs_f.end());

		for (size_t i=1; i<indices.size();i++)
		{
			EdgeIter e = connectVertex(h_from_v0, hs_f[indices[i]]);
			if (indices[i]==mark)
			{
				v0->halfedge() = e->halfedge()->vertex() == v0?e->halfedge(): v0->halfedge() = e->halfedge()->twin();
			}
		}

		return v0;
		
	}

	VertexIter HalfedgeMesh::collapseFace(FaceIter f) {
		// This method should collapse the given face and return an iterator to
		// the new vertex created by the collapse.

		if (f->isBoundary())
		{
			//showError("This face is boundary.");
			return verticesEnd();
		}

		Vector3D position = f->centroid();
		VertexIter v0,v1;
		HalfedgeIter h = f->halfedge(), h_next;

		while (f->degree() > 3)
		{
			h_next = h->next()->next();
			v0 = collapseEdge(h->edge());
			f = h_next->face();
			h = v0->findHalfedgeOn(f);
		}

		h_next = h->next()->next();
		
		v0 = collapseEdge(h->edge());
		
		v0 = collapseEdge(v0->halfedge()->edge());
		/*
			FIXME: How to find the last edge to collapse
			(Can fix the insertVertice function, redirect the inserted v0. 
			And then fix collapseEdge by redirecting the new vertice to the edge)
		*/
		

		v0->position = position;
		return v0;
	}

	FaceIter HalfedgeMesh::eraseVertex(VertexIter v) {
		// This method should replace the given vertex and all its neighboring
		// edges and faces with a single face, returning the new face.

		vector<EdgeIter> adjes = v->adjEdges();
		vector<FaceIter> adjfs = v->adjFaces();
		Vector3D vec = adjfs[0]->normal();
		FaceIter f;

		for (auto adje : adjes)
		{
			if (adje->isBridge() && !adje->isSingle())
			{
				showError("One of this vertex's neighboring edges is bridge. ", true);
				return facesEnd();
			}
		}

		for (auto adje : adjes)
		{
			f = eraseEdge(adje);
		}

		return f;
	}

	FaceIter HalfedgeMesh::eraseEdge(EdgeIter e) {
		// This method should erase the given edge and return an iterator to the
		// merged face.

		if (e->isBoundary())
		{
			showError("EraseEdge: This edge is boundary that cannot be erased.", true);
			return facesEnd();
		}
		if (e->isBridge() && !e->isSingle())
		{
			showError("EraseEdge: This edge is a bridge that cannot be erased.", true);
			return facesEnd();
		}
		/*
		    Not single
			Before:
			                v0
			----------------.----------------
				   <-h0_next|<-h1_pre
						  /\|
						  h0|
							|
				f0			|			f1
							|h1
							|\/
					h0_pre->|h1_next->
			----------------.----------------
			                v1
		*/                  
		HalfedgeIter h0 = e->halfedge();
		HalfedgeIter h0_next = h0->next();
		HalfedgeIter h0_pre = h0->pre();
		HalfedgeIter h1 = h0->twin();
		HalfedgeIter h1_next = h1->next();
		HalfedgeIter h1_pre = h1->pre();
		VertexIter v0 = h0_next->vertex();
		VertexIter v1 = h1_next->vertex();
		FaceIter f0 = h0->face();
		FaceIter f1 = h1->face();

		if (e->isSingle())
		{
			if (h1_next == h0)
			{
				h0 = h1;
				h1 = h0->twin();
				h1_next = h1->next();
				h0_pre = h0->pre();
				v0 = h0->next()->vertex();
				v1 = h1->next()->vertex();
				f0 = h0->face();
				f1 = h1->face();
			}

			h0_pre->next() = h1_next;
			v1->halfedge() = h1_next;
			f0->halfedge() = h0_pre;

			deleteVertex(v0);
		}
		else
		{
			h0_pre->next() = h1_next;
			h1_pre->next() = h0_next;
			v0->halfedge() = h0_next;
			v1->halfedge() = h1_next;
			f0->halfedge() = h0_next;
			deleteFace(f1);
		}
		HalfedgeIter h = h1_next;
		do
		{
			h->face() = f0;
			h = h->next();
		} while (h != h1_next);

		deleteHalfedge(h0);
		deleteHalfedge(h1);
		deleteEdge(e);

		return f0;
	}

	EdgeIter HalfedgeMesh::flipEdge(EdgeIter e0) {
		// (meshEdit)
		// This method should flip the given edge and return an iterator to the
		// flipped edge.
		if (e0->isBoundary())
		{
			showError("FlipEdge: This edge is boundary that cannot be flipped.", false);
			return e0;
		}
		if (e0->isBridge())
		{
			showError("FlipEdge: This edge is a bridge that cannot be flipped.", false);
			return e0;
		}

		HalfedgeIter h0 = e0->halfedge();
		HalfedgeIter h1 = h0->twin();

		HalfedgeIter h2 = h0->next()->next();
		HalfedgeIter h3 = h1->next()->next();
		eraseEdge(e0);
		e0 = connectVertex(h2, h3);

		return e0;
	}

	void HalfedgeMesh::subdivideQuad(bool useCatmullClark) {
		// Unlike the local mesh operations (like bevel or edge flip), we will perform
		// subdivision by splitting *all* faces into quads "simultaneously."  Rather
		// than operating directly on the halfedge data structure (which as you've
		// seen
		// is quite difficult to maintain!) we are going to do something a bit nicer:
		//
		//    1. Create a raw list of vertex positions and faces (rather than a full-
		//       blown halfedge mesh).
		//
		//    2. Build a new halfedge mesh from these lists, replacing the old one.
		//
		// Sometimes rebuilding a data structure from scratch is simpler (and even
		// more
		// efficient) than incrementally modifying the existing one.  These steps are
		// detailed below.

		// Step I: Compute the vertex positions for the subdivided mesh.  Here
		// we're
		// going to do something a little bit strange: since we will have one vertex
		// in
		// the subdivided mesh for each vertex, edge, and face in the original mesh,
		// we
		// can nicely store the new vertex *positions* as attributes on vertices,
		// edges,
		// and faces of the original mesh.  These positions can then be conveniently
		// copied into the new, subdivided mesh.

		if (useCatmullClark) {
			computeCatmullClarkPositions();
		}
		else {
			computeLinearSubdivisionPositions();
		}

		// Step II: Assign a unique index (starting at 0) to each vertex, edge,
		// and
		// face in the original mesh.  These indices will be the indices of the
		// vertices
		// in the new (subdivided mesh).  They do not have to be assigned in any
		// particular
		// order, so long as no index is shared by more than one mesh element, and the
		// total number of indices is equal to V+E+F, i.e., the total number of
		// vertices
		// plus edges plus faces in the original mesh.  Basically we just need a
		// one-to-one
		// mapping between original mesh elements and subdivided mesh vertices.
		// [See subroutine for actual "TODO"s]
		assignSubdivisionIndices();

		// Step III: Build a list of quads in the new (subdivided) mesh, as
		// tuples of
		// the element indices defined above.  In other words, each new quad should be
		// of
		// the form (i,j,k,l), where i,j,k and l are four of the indices stored on our
		// original mesh elements.  Note that it is essential to get the orientation
		// right
		// here: (i,j,k,l) is not the same as (l,k,j,i).  Indices of new faces should
		// circulate in the same direction as old faces (think about the right-hand
		// rule).
		// [See subroutines for actual "TODO"s]
		vector<vector<Index> > subDFaces;
		vector<Vector3D> subDVertices;
		buildSubdivisionFaceList(subDFaces);
		buildSubdivisionVertexList(subDVertices);

		// Step IV: Pass the list of vertices and quads to a routine that clears
		// the
		// internal data for this halfedge mesh, and builds new halfedge data from
		// scratch,
		// using the two lists.
		rebuild(subDFaces, subDVertices);
	}

	/**
	 * Compute new vertex positions for a mesh that splits each polygon
	 * into quads (by inserting a vertex at the face midpoint and each
	 * of the edge midpoints).  The new vertex positions will be stored
	 * in the members Vertex::newPosition, Edge::newPosition, and
	 * Face::newPosition.  The values of the positions are based on
	 * simple linear interpolation, e.g., the edge midpoints and face
	 * centroids.
	 */
	void HalfedgeMesh::computeLinearSubdivisionPositions() {
		// For each vertex, assign Vertex::newPosition to
		// its original position, Vertex::position.
		for (auto& v : vertices)
		{
			v.newPosition = v.position;
		}

		// For each edge, assign the midpoint of the two original
		// positions to Edge::newPosition.
		for (auto& e : edges)
		{
			e.newPosition = e.centroid();
		}

		// For each face, assign the centroid (i.e., arithmetic mean)
		// of the original vertex positions to Face::newPosition.  Note
		// that in general, NOT all faces will be triangles!
		for (auto& f : faces)
		{
			f.newPosition = f.centroid();
		}


	}

	/**
	 * Compute new vertex positions for a mesh that splits each polygon
	 * into quads (by inserting a vertex at the face midpoint and each
	 * of the edge midpoints).  The new vertex positions will be stored
	 * in the members Vertex::newPosition, Edge::newPosition, and
	 * Face::newPosition.  The values of the positions are based on
	 * the Catmull-Clark rules for subdivision.
	 */
	void HalfedgeMesh::computeCatmullClarkPositions() {
		// The implementation for this routine should be
		// a lot like HalfedgeMesh::computeLinearSubdivisionPositions(),
		// except that the calculation of the positions themsevles is
		// slightly more involved, using the Catmull-Clark subdivision
		// rules. (These rules are outlined in the Developer Manual.)

		// face
		for (auto& f : faces)
		{
			f.newPosition = f.centroid();
		}

		// edges
		for (auto& e : edges)
		{
			HalfedgeIter h1 = e.halfedge();
			HalfedgeIter h2 = h1->twin();
			e.newPosition = (h1->vertex()->position + h2->vertex()->position + h1->face()->newPosition + h2->face()->newPosition) / 4;
		}

		// vertices
		for (auto& v : vertices)
		{
			Vector3D Q(0), R(0), S(0);
			int n = v.degree();
			vector<HalfedgeIter> hs_out = v.outHalfedges();
			for (auto h_out : hs_out)
			{
				Q += h_out->face()->newPosition;
				R += h_out->edge()->centroid();
			}
			Q /= n;
			R /= n;
			S = v.position;
			v.newPosition = (Q + 2 * R + (n - 3) * S) / n;
		}
	}

	/**
	 * Assign a unique integer index to each vertex, edge, and face in
	 * the mesh, starting at 0 and incrementing by 1 for each element.
	 * These indices will be used as the vertex indices for a mesh
	 * subdivided using Catmull-Clark (or linear) subdivision.
	 */
	void HalfedgeMesh::assignSubdivisionIndices() {
		// Start a counter at zero; if you like, you can use the
		// "Index" type (defined in halfedgeMesh.h)
		Index i = 0;
		// Iterate over vertices, assigning values to Vertex::index
		for (auto& v : vertices)
		{
			v.index = i++;
		}

		// Iterate over edges, assigning values to Edge::index
		for (auto& e : edges)
		{
			e.index = i++;
		}
		// Iterate over faces, assigning values to Face::index
		for (auto& f : faces)
		{
			f.index = i++;
		}

	}

	/**
	 * Build a flat list containing all the vertex positions for a
	 * Catmull-Clark (or linear) subdivison of this mesh.  The order of
	 * vertex positions in this list must be identical to the order
	 * of indices assigned to Vertex::newPosition, Edge::newPosition,
	 * and Face::newPosition.
	 */
	void HalfedgeMesh::buildSubdivisionVertexList(vector<Vector3D>& subDVertices) {
		// Resize the vertex list so that it can hold all the vertices.

		// Iterate over vertices, assigning Vertex::newPosition to the
		// appropriate location in the new vertex list.
		for (auto& v : vertices)
		{
			subDVertices.push_back(v.newPosition);
		}
		// Iterate over edges, assigning Edge::newPosition to the appropriate
		// location in the new vertex list.
		for (auto& e : edges)
		{
			subDVertices.push_back(e.newPosition);
		}
		// Iterate over faces, assigning Face::newPosition to the appropriate
		// location in the new vertex list.
		for (auto& f : faces)
		{
			subDVertices.push_back(f.newPosition);
		}

	}

	/**
	 * Build a flat list containing all the quads in a Catmull-Clark
	 * (or linear) subdivision of this mesh.  Each quad is specified
	 * by a vector of four indices (i,j,k,l), which come from the
	 * members Vertex::index, Edge::index, and Face::index.  Note that
	 * the ordering of these indices is important because it determines
	 * the orientation of the new quads; it is also important to avoid
	 * "bowties."  For instance, (l,k,j,i) has the opposite orientation
	 * of (i,j,k,l), and if (i,j,k,l) is a proper quad, then (i,k,j,l)
	 * will look like a bowtie.
	 */
	void HalfedgeMesh::buildSubdivisionFaceList(vector<vector<Index> >& subDFaces) {
		// This routine is perhaps the most tricky step in the construction of
		// a subdivision mesh (second, perhaps, to computing the actual Catmull-Clark
		// vertex positions).  Basically what you want to do is iterate over faces,
		// then for each for each face, append N quads to the list (where N is the
		// degree of the face).  For this routine, it may be more convenient to simply
		// append quads to the end of the list (rather than allocating it ahead of
		// time), though YMMV.  You can of course iterate around a face by starting
		// with its first halfedge and following the "next" pointer until you get
		// back to the beginning.  The tricky part is making sure you grab the right
		// indices in the right order---remember that there are indices on vertices,
		// edges, AND faces of the original mesh.  All of these should get used.  Also
		// remember that you must have FOUR indices per face, since you are making a
		// QUAD mesh!

		// iterate over faces
		// loop around face
		// build lists of four indices for each sub-quad
		// append each list of four indices to face list

		for (auto& f : faces)
		{
			HalfedgeIter h = f.halfedge();
			do {
				vector<Index> subDFace(4);
				HalfedgeIter h_next = h->next();

				subDFace[0] = h_next->vertex()->index;
				subDFace[1] = h_next->edge()->index;
				subDFace[2] = f.index;
				subDFace[3] = h->edge()->index;
				subDFaces.push_back(subDFace);
				subDFace.clear();

				h = h_next;
			} while (h != f.halfedge());
		}

	}

	FaceIter HalfedgeMesh::bevelVertex(VertexIter v) {
		// This method should replace the vertex v with a face, corresponding to
		// a bevel operation. It should return the new face.  NOTE: This method is
		// responsible for updating the *connectivity* of the mesh only---it does not
		// need to update the vertex positions.  These positions will be updated in
		// HalfedgeMesh::bevelVertexComputeNewPositions (which you also have to
		// implement!)
		vector<EdgeIter> es = v->adjEdges();
		vector<HalfedgeIter> out_hs = v->outHalfedges();

		vector<HalfedgeIter>::iterator i = out_hs.begin();
		for (auto e : es)
		{
			insertVertice(0, *(i++));
		}
		for (auto out_h : out_hs)
		{
			connectVertex(out_h->next(), out_h->pre());
		}

		return eraseVertex(v);
	}

	FaceIter HalfedgeMesh::bevelEdge(EdgeIter e) {
		// This method should replace the edge e with a face, corresponding to a
		// bevel operation. It should return the new face.  NOTE: This method is
		// responsible for updating the *connectivity* of the mesh only---it does not
		// need to update the vertex positions.  These positions will be updated in
		// HalfedgeMesh::bevelEdgeComputeNewPositions (which you also have to
		// implement!)

		HalfedgeIter h0 = e->halfedge();
		HalfedgeIter h1 = h0->twin();

		VertexIter v0 = h0->vertex();
		VertexIter v1 = h1->vertex();
		vector<HalfedgeIter> hs;

		for (HalfedgeIter h = h0->next(); h != h1; h = h->twin()->next())
		{
			insertVertice(0, h);
			hs.push_back(h);
		}

		for (HalfedgeIter h = h1->next(); h != h0; h = h->twin()->next())
		{
			insertVertice(0, h);
			hs.push_back(h);
		}

		int n = hs.size();
		for (int i = 0; i < n; ++i)
		{
			connectVertex(hs[i]->next(), hs[(i - 1 + n) % n]->twin());
		}

		eraseVertex(v0);
		return eraseVertex(v1);
	}

	FaceIter HalfedgeMesh::bevelFace(FaceIter f) {
		// This method should replace the face f with an additional, inset face
		// (and ring of faces around it), corresponding to a bevel operation. It
		// should return the new face.  NOTE: This method is responsible for updating
		// the *connectivity* of the mesh only---it does not need to update the vertex
		// positions.  These positions will be updated in
		// HalfedgeMesh::bevelFaceComputeNewPositions (which you also have to
		// implement!)

		vector<HalfedgeIter> hs = f->halfedges();
		vector<VertexIter> vs;

		for (auto h : hs)
		{
			vs.push_back(insertVertice(h));
		}

		for (int i = 0; i < vs.size(); i++)
		{
			connectVertex(vs[i]->halfedge(), i < vs.size() - 1 ? vs[i + 1]->halfedge() : vs[0]->halfedge()->twin()->next());
		}

		HalfedgeIter h0 = hs[0]->next()->next()->twin();
		FaceIter f0 = h0->face();
		f0->halfedge() = h0;

		return f0;

	}

	Vector3D intersection(Vector3D p1, Vector3D v1, Vector3D p2, Vector3D v2)
	{
		/*
		p1 + t * v1 = p2 + s * v2
		x1 + t * (x2 - x1) = x3 + s * (x4 - x3)
		y1 + t * (y2 - y1) = y3 + s * (y4 - y3)
		t * (x2 - x1) - s * (x4 - x3) = x3 - x1
		t * (y2 - y1) - s * (y4 - y3) = y3 - y1
		*/
		double a = v1.x;// x2-x1
		double b = -v2.x;// -(x4-x3)
		double c = v1.y;// y2-y1
		double d = -v2.y;// -(y4-y3)
		double g = p2.x - p1.x;// x3-x1
		double h = p2.y - p1.y;//y3-y1
		double f = a * d - b * c;

		double t = (d * g - b * h) / f;
		double s = (-c * g + a * h) / f;

		return p1 + v1 * t;
	}

	void HalfedgeMesh::bevelFaceComputeNewPositions(
		vector<Vector3D>& originalVertexPositions,
		vector<HalfedgeIter>& newHalfedges, double normalShift,
		double tangentialInset) {
		// Compute new vertex positions for the vertices of the beveled face.
		//
		// These vertices can be accessed via newHalfedges[i]->vertex()->position for
		// i = 1, ..., newHalfedges.size()-1.
		//
		// The basic strategy here is to loop over the list of outgoing halfedges,
		// and use the preceding and next vertex position from the original mesh
		// (in the originalVertexPositions array) to compute an offset vertex
		// position.
		//
		// Note that there is a 1-to-1 correspondence between halfedges in
		// newHalfedges and vertex positions
		// in orig.  So, you can write loops of the form
		//
		// for( int i = 0; i < newHalfedges.size(); hs++ )
		// {
		//    Vector3D pi = originalVertexPositions[i]; // get the original vertex
		//    position correponding to vertex i
		// }
		//
		Vector3D nor = newHalfedges[0]->twin()->next()->twin()->face()->normal().unit();
		vector<Vector3D> dirs;

		int n = originalVertexPositions.size();
		for (int i = 0; i < n; i++)
		{
			Vector3D v1 = (originalVertexPositions[(i - 1 + n) % n] - originalVertexPositions[i]).unit();
			Vector3D v2 = (originalVertexPositions[(i + 1) % n] - originalVertexPositions[i]).unit();
			dirs.push_back((v1 + v2).unit());
		}

		double dir_max_norm2 = DBL_MAX;
		for (int i = 0; i < n; i++)
		{
			int a = i;
			int b = (i + 1) % n;
			Vector3D intersection_p = intersection(originalVertexPositions[a], dirs[a], originalVertexPositions[b], dirs[b]);
			double dir_length = (intersection_p - originalVertexPositions[a]).norm2();

			if (dir_max_norm2 > dir_length)
			{
				dir_max_norm2 = dir_length;
			}
		}

		double dir_max_norm = sqrt(dir_max_norm2);
		double dir_norm = dot(newHalfedges[0]->vertex()->position - originalVertexPositions[0], dirs[0]);


		if (dir_norm + tangentialInset > dir_max_norm || dir_norm + tangentialInset < 0)
		{
			return;
		}

		for (int i = 0; i < n; i++)
		{
			newHalfedges[i]->vertex()->position = newHalfedges[i]->vertex()->position + dirs[i] * tangentialInset + nor * normalShift;
		}


	}

	void HalfedgeMesh::bevelVertexComputeNewPositions(
		Vector3D originalVertexPosition, vector<HalfedgeIter>& newHalfedges,
		double tangentialInset) {
		// TODO Compute new vertex positions for the vertices of the beveled vertex.
		//
		// These vertices can be accessed via newHalfedges[i]->vertex()->position for
		// i = 1, ..., hs.size()-1.
		//
		// The basic strategy here is to loop over the list of outgoing halfedges,
		// and use the preceding and next vertex position from the original mesh
		// (in the orig array) to compute an offset vertex position.
		for (auto h : newHalfedges)
		{
			VertexIter v0 = h->vertex();
			VertexIter v1 = h->twin()->vertex();

			Vector3D dir = v1->position - originalVertexPosition;
			Vector3D delta = dir * tangentialInset * 10.0;
			if ((tangentialInset > 0 && delta.norm2() >= (v1->position - v0->position).norm2()) || (tangentialInset < 0 && delta.norm2() >= (v0->position - originalVertexPosition).norm2()))
			{
				return;
			}
			v0->position += delta;
		}
	}

	void HalfedgeMesh::bevelEdgeComputeNewPositions(
		vector<Vector3D>& originalVertexPositions,
		vector<HalfedgeIter>& newHalfedges, double tangentialInset) {
		// Compute new vertex positions for the vertices of the beveled edge.
		//
		// These vertices can be accessed via newHalfedges[i]->vertex()->position for
		// i = 1, ..., newHalfedges.size()-1.
		//
		// The basic strategy here is to loop over the list of outgoing halfedges,
		// and use the preceding and next vertex position from the original mesh
		// (in the orig array) to compute an offset vertex position.
		//
		// Note that there is a 1-to-1 correspondence between halfedges in
		// newHalfedges and vertex positions
		// in orig.  So, you can write loops of the form
		//
		// for( int i = 0; i < newHalfedges.size(); i++ )
		// {
		//    Vector3D pi = originalVertexPositions[i]; // get the original vertex
		//    position correponding to vertex i
		// }
		//
		vector<Vector3D> dirs;

		int n = originalVertexPositions.size();
		double dir_max_norm2 = DBL_MAX;

		for (int i = 0; i < n; i++)
		{
			Vector3D dir = newHalfedges[i]->twin()->vertex()->position - originalVertexPositions[i];
			double dir_norm2 = dir.norm2();
			if (dir_max_norm2 > dir_norm2)
			{
				dir_max_norm2 = dir_norm2;
			}
			dirs.push_back(dir.unit());
		}

		double dir_norm = dot(newHalfedges[0]->vertex()->position - originalVertexPositions[0], dirs[0]);
		if (dir_norm + tangentialInset >= sqrt(dir_max_norm2) || dir_norm + tangentialInset <= 0)
		{
			return;
		}

		for (int i = 0; i < n; i++)
		{
			newHalfedges[i]->vertex()->position = newHalfedges[i]->vertex()->position + dirs[i] * tangentialInset;
		}
	}

	void HalfedgeMesh::splitPolygons(vector<FaceIter>& fcs) {
		for (auto f : fcs) splitPolygon(f);
	}

	void HalfedgeMesh::splitPolygon(FaceIter f) {
		// Triangulate a polygonal face

		if (f->degree() == 3)
		{
			return;
		}
		HalfedgeIter h0 = f->halfedge();

		int n = f->degree() - 3;
		HalfedgeIter h1 = h0->next()->next();
		for (int i = 0; i < n; ++i)
		{
			connectVertex(h0, h1);
			h1 = h1->next();
		}

	}

	EdgeRecord::EdgeRecord(EdgeIter& _edge) : edge(_edge) {
		// (meshEdit)
		// Compute the combined quadric from the edge endpoints.
		Matrix4x4 K = edge->halfedge()->vertex()->quadric + edge->halfedge()->twin()->vertex()->quadric;

		// -> Build the 3x3 linear system whose solution minimizes the quadric error
		//    associated with these two endpoints.
		Matrix3x3 A = K.to3x3();
		Vector3D b = -K[3].to3D();

		// -> Use this system to solve for the optimal position, and store it in
		//    EdgeRecord::optimalPoint.
		optimalPoint = abs(A.det()) < 0.000001 ? edge->centroid() : A.inv() * b;

		// -> Also store the cost associated with collapsing this edg in
		//    EdgeRecord::Cost.
		score = dot(Vector4D(optimalPoint, 1), K * Vector4D(optimalPoint, 1));
	}

	void MeshResampler::upsample(HalfedgeMesh& mesh)
		// This routine should increase the number of triangles in the mesh using Loop
		// subdivision.
	{
		// (meshEdit)
		// Compute new positions for all the vertices in the input mesh, using
		// the Loop subdivision rule, and store them in Vertex::newPosition.
		// -> At this point, we also want to mark each vertex as being a vertex of the
		//    original mesh.
		// -> Next, compute the updated vertex positions associated with edges, and
		//    store it in Edge::newPosition.
		// -> Next, we're going to split every edge in the mesh, in any order.  For
		//    future reference, we're also going to store some information about which
		//    subdivided edges come from splitting an edge in the original mesh, and
		//    which edges are new, by setting the flat Edge::isNew. Note that in this
		//    loop, we only want to iterate over edges of the original mesh.
		//    Otherwise, we'll end up splitting edges that we just split (and the
		//    loop will never end!)
		// -> Now flip any new edge that connects an old and new vertex.
		// -> Finally, copy the new vertex positions into final Vertex::position.

		for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)
		{
			v->isNew = false;
		}
		for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
		{
			e->isNew = false;
		}

		// Each vertex and edge of the original surface can be associated with a
		// vertex in the new (subdivided) surface.
		// Therefore, our strategy for computing the subdivided vertex locations is to
		// *first* compute the new positions
		// using the connectity of the original (coarse) mesh; navigating this mesh
		// will be much easier than navigating
		// the new subdivided (fine) mesh, which has more elements to traverse.  We
		// will then assign vertex positions in
		// the new mesh based on the values we computed for the original mesh.
		
		// Compute updated positions for all the vertices in the original mesh, using
		// the Loop subdivision rule.
		for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)
		{
			vector<VertexIter> adjvs = v->adjVertices();

			int n = v->degree();
			double u = n == 3 ? 0.1875 : 0.375 / n;
			v->newPosition = (1 - n * u) * v->position;
			for (auto adjv:adjvs)
			{
				v->newPosition += u * adjv->position;
			}
		}
		// Next, compute the updated vertex positions associated with edges.
		for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
		{
			Vector3D A = e->halfedge()->vertex()->position;
			Vector3D B = e->halfedge()->twin()->vertex()->position;
			Vector3D C = e->halfedge()->pre()->vertex()->position;
			Vector3D D = e->halfedge()->twin()->pre()->vertex()->position;
			e->newPosition = 0.375 * (A + B) + 0.125 * (C + D);
		}
		// Next, we're going to split every edge in the mesh, in any order.  For
		// future
		// reference, we're also going to store some information about which
		// subdivided
		// edges come from splitting an edge in the original mesh, and which edges are
		// new.
		// In this loop, we only want to iterate over edges of the original
		// mesh---otherwise,
		// we'll end up splitting edges that we just split (and the loop will never
		// end!)

		int n = mesh.nEdges();
		EdgeIter e = mesh.edgesBegin();
		for (int i = 0; i < n; i++) {
			EdgeIter nextEdge = e;
			nextEdge++;
			mesh.splitEdge(e);
			e = nextEdge;
		}

		// Finally, flip any new edge that connects an old and new vertex.
		n = mesh.nEdges();
		e = mesh.edgesBegin();
		for (int i = 0; i < n; i++) {
			EdgeIter nextEdge = e;
			nextEdge++;
			VertexIter v1 = e->halfedge()->vertex();
			VertexIter v2 = e->halfedge()->twin()->vertex();
			if (e->isNew && v1->isNew != v2->isNew)
			{
				HalfedgeIter h1 = v1->halfedge();
				HalfedgeIter h2 = v2->halfedge();
				mesh.flipEdge(e);
				v1->halfedge() = h1;
				v2->halfedge() = h2;
			}
			e = nextEdge;
		}

		// Copy the updated vertex positions to the subdivided mesh.
		for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)
		{
			if (v->isNew)
			{
				v->position = v->halfedge()->edge()->newPosition;
			}
			else
			{
				v->position = v->newPosition;
			}
		}

	}

	void MeshResampler::downsample(HalfedgeMesh& mesh) {
		// (meshEdit)
		// Compute initial quadrics for each face by simply writing the plane equation
		// for the face in homogeneous coordinates. These quadrics should be stored
		// in Face::quadric
		for (FaceIter f= mesh.facesBegin();f!=mesh.facesEnd();f++)
		{
			double d = -dot(f->normal(), f->halfedge()->vertex()->position);
			Vector4D v(f->normal(), d);
			f->quadric = outer(v, v);
		}
		// -> Compute an initial quadric for each vertex as the sum of the quadrics
		//    associated with the incident faces, storing it in Vertex::quadric
		for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++)
		{
			vector<FaceIter> fs = v->adjFaces();
			v->quadric.zero();
			for (auto f: fs)
			{
				v->quadric += f->quadric;
			}
		}
		// -> Build a priority queue of edges according to their quadric error cost,
		//    i.e., by building an EdgeRecord for each edge and sticking it in the
		//    queue.
		MutablePriorityQueue<EdgeRecord> queue;
		for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
		{
			e->record = EdgeRecord(e);
			queue.insert(e->record);
		}
		
		// -> Until we reach the target edge budget, collapse the best edge. Remember
		//    to remove from the queue any edge that touches the collapsing edge
		//    BEFORE it gets collapsed, and add back into the queue any edge touching
		//    the collapsed vertex AFTER it's been collapsed. Also remember to assign
		//    a quadric to the collapsed vertex, and to pop the collapsed edge off the
		//    top of the queue.

		int targetFaceNum = 0.25 * mesh.nFaces();
		while (mesh.nFaces() >= targetFaceNum)
		{
			// 1. Get the cheapest edge from the queue.
			EdgeRecord bestEdge = queue.top();
			EdgeIter e = bestEdge.edge;

			// 2. Remove the cheapest edge from the queue by calling pop().
			queue.pop();

			// 3. Compute the new quadric by summing the quadrics at its two endpoints.
			Matrix4x4 newQuadric = e->halfedge()->vertex()->quadric + e->halfedge()->twin()->vertex()->quadric;

			// 4. Remove any edge touching either of its endpoints from the queue.
			vector<EdgeIter> adjEs_e = e->getAdjEdges();
			for (auto e : adjEs_e)
			{
				queue.remove(e->record);
			}

			// 5. Collapse the edge.
			VertexIter newV = mesh.collapseEdge(e);
			newV->position = bestEdge.optimalPoint;
			
			// 6. Set the quadric of the new vertex to the quadric computed in Step 3.
			newV->quadric = newQuadric;

			// 7. Insert any edge touching the new vertex into the queue, creating new edge records for each of them.
			vector<EdgeIter> adjEs_v = newV->adjEdges();
			for (auto e : adjEs_v)
			{
				e->record = EdgeRecord(e);
				queue.insert(e->record);
			}
		}
		
	}

	void MeshResampler::resample(HalfedgeMesh& mesh) {
		// (meshEdit)
		// Compute the mean edge length.
		double mean = 0;
		for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
		{
			mean += e->length();
		}
		mean /= mesh.nEdges();

		// Repeat the four main steps for 5 or 6 iterations
		
		for (int i = 0; i < 5; i++)
		{
			set<EdgeIter> edges;
			// -> Split edges much longer than the target length (being careful about
			//    how the loop is written!)
			
			for (auto e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
			{
				edges.insert(e);
			}
			for (auto e:edges)
			{		
				if (e->length() > 4.0 / 3.0 * mean )
				{
					mesh.splitEdge(e);
				}
			}
			edges.clear();

			// -> Collapse edges much shorter than the target length.  Here we need to
			//    be EXTRA careful about advancing the loop, because many edges may have
			//    been destroyed by a collapse (which ones?)

			for (auto e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
			{
				edges.insert(e);
			}

			while (edges.size()>0)
			{
				EdgeIter e = *edges.begin();
				edges.erase(e);
				
				if (e->length() < 0.8 * mean)
				{
					auto adjEs = e->getAdjEdges();
					for (auto adjE:adjEs)
					{
						edges.erase(adjE);
					}
					mesh.collapseEdge(e);
				}
			}

			// -> Now flip each edge if it improves vertex degree

			for (auto e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++)
			{
				edges.insert(e);
			}
			while (edges.size()>0)
			{
				EdgeIter e = *edges.begin();
				edges.erase(e);

				int a1 = e->halfedge()->next()->next()->vertex()->degree();
				int a2 = e->halfedge()->twin()->next()->next()->vertex()->degree();
				int b1 = e->halfedge()->vertex()->degree();
				int b2 = e->halfedge()->twin()->vertex()->degree();

				double init_deviation = abs(a1 - 6) + abs(a2 - 6) + abs(b1 - 6) + abs(b2 - 6);
				double after_deviation = abs(a1 - 5) + abs(a2 - 5) + abs(b1 - 7) + abs(b2 - 7);
				if (init_deviation > after_deviation)
				{
					mesh.flipEdge(e);
				}
			}

			// -> Finally, apply some tangential smoothing to the vertex positions
			for (VertexIter vi = mesh.verticesBegin(); vi != mesh.verticesEnd(); vi++)
			{
				Vector3D p = vi->centroid();
				Vector3D c = vi->neighborhoodCentroid();
				double w = 1;
				Vector3D v = c - p;
				Vector3D N = vi->normal();
				v = v - dot(N, v) * N;
				vi->newPosition = p + w * v;
			}

			for (VertexIter vi = mesh.verticesBegin(); vi != mesh.verticesEnd(); vi++)
			{
				vi->position = vi->newPosition;
			}

		}
		
	}

}  // namespace CMU462
