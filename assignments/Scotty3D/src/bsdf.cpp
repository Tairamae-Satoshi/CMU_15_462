#include "bsdf.h"

#include <algorithm>
#include <iostream>
#include <utility>


using std::min;
using std::max;
using std::swap;

namespace CMU462 {

	void make_coord_space(Matrix3x3& o2w, const Vector3D& n) {
		Vector3D z = Vector3D(n.x, n.y, n.z);
		Vector3D h = z;
		if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
			h.x = 1.0;
		else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
			h.y = 1.0;
		else
			h.z = 1.0;

		z.normalize();
		Vector3D y = cross(h, z);
		y.normalize();
		Vector3D x = cross(z, y);
		x.normalize();

		o2w[0] = x;
		o2w[1] = y;
		o2w[2] = z;
	}

	// Diffuse BSDF //

	Spectrum DiffuseBSDF::f(const Vector3D& wo, const Vector3D& wi) {
		return albedo * (1.0 / PI);
	}

	Spectrum DiffuseBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
		// TODO (PathTracer):
		// Implement DiffuseBSDF
		*wi = sampler.get_sample(pdf);
		return albedo * (1.0 / PI);
	}

	// Mirror BSDF //

	Spectrum MirrorBSDF::f(const Vector3D& wo, const Vector3D& wi) {
		return Spectrum();
	}

	Spectrum MirrorBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
		// TODO (PathTracer):
		// Implement MirrorBSDF
		reflect(wo, wi);
		*pdf = 1.0f;
		return reflectance * (1.f / fabs(wi->z));
	}

	// Glossy BSDF //

	/*
	Spectrum GlossyBSDF::f(const Vector3D& wo, const Vector3D& wi) {
	  return Spectrum();
	}

	Spectrum GlossyBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
	  *pdf = 1.0f;
	  return reflect(wo, wi, reflectance);
	}
	*/

	// Refraction BSDF //

	Spectrum RefractionBSDF::f(const Vector3D& wo, const Vector3D& wi) {
		return Spectrum();
	}

	Spectrum RefractionBSDF::sample_f(const Vector3D& wo, Vector3D* wi,
		float* pdf) {
		// TODO (PathTracer):
		// Implement RefractionBSDF
		if (!refract(wo,wi,ior))
		{
			reflect(wo, wi);
		}
		return transmittance * (1.f/fabs(wi->z));
	}

	// Glass BSDF //

	float schlick(float cosine, float ref_idx) {
		float r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}

	Spectrum GlassBSDF::f(const Vector3D& wo, const Vector3D& wi) {
		return Spectrum();
	}

	Spectrum GlassBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {
		// TODO (PathTracer):
		// Compute Fresnel coefficient and either reflect or refract based on it.
		
		float cosine = wo.z > 0 ? wo.z : ior * -wo.z;
		float Fr = refract(wo, wi, ior)? schlick(cosine, ior): 1.f;

		if ((double)(std::rand() / RAND_MAX) < Fr) {
			*pdf = Fr;
			reflect(wo, wi);
			return Fr / fabs(wi->z) * reflectance;
		}
		else{
			*pdf = 1 - Fr;
			float ni_over_nt = wo.z > 0 ? 1.f / ior : ior;
			return (ni_over_nt * ni_over_nt * (1 - Fr) / fabs(wi->z)) * transmittance;
		}

	}

	void BSDF::reflect(const Vector3D& wo, Vector3D* wi) {
		// TODO (PathTracer):
		// Implement reflection of wo about normal (0,0,1) and store result in wi.
		wi->x = -wo.x;
		wi->y = -wo.y;
		wi->z = wo.z;
	}

	bool BSDF::refract(const Vector3D& wo, Vector3D* wi, float ior) {
		// TODO (PathTracer):
		// Use Snell's Law to refract wo surface and store result ray in wi.
		// Return false if refraction does not occur due to total internal reflection
		// and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
		// ray entering the surface through vacuum.
		// ni*sin(theta)i= nt*sin(theta)t
		float ni_over_nt = wo.z > 0 ? 1.f / ior : ior;

		float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - wo.z * wo.z);
		if (discriminant > 0)
		{
			wi->x = -wo.x;
			wi->y = -wo.y;
			wi->z = (wo.z >= 0 ? -1 : 1) * sqrt(discriminant);
			wi->normalize();
			return true;
		}
		
		return false;
	}

	// Emission BSDF //

	Spectrum EmissionBSDF::f(const Vector3D& wo, const Vector3D& wi) {
		return Spectrum();
	}

	Spectrum EmissionBSDF::sample_f(const Vector3D& wo, Vector3D* wi, float* pdf) {

		*wi = sampler.get_sample(pdf);
		return Spectrum();
	}

}  // namespace CMU462
