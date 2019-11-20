#include "sampler.h"

namespace CMU462 {

	// Uniform Sampler2D Implementation //

	Vector2D UniformGridSampler2D::get_sample() const {
		// TODO (PathTracer):
		// Implement uniform 2D grid sampler
		double x = (double)(std::rand()) / RAND_MAX;
		double y = (double)(std::rand()) / RAND_MAX;

		return Vector2D(x, y);
	}

	// Uniform Hemisphere Sampler3D Implementation //

	Vector3D UniformHemisphereSampler3D::get_sample() const {
		double Xi1 = (double)(std::rand()) / RAND_MAX;
		double Xi2 = (double)(std::rand()) / RAND_MAX;

		double theta = acos(Xi1);
		double phi = 2.0 * PI * Xi2;

		double xs = sinf(theta) * cosf(phi);
		double ys = sinf(theta) * sinf(phi);
		double zs = cosf(theta);

		return Vector3D(xs, ys, zs);
	}

	Vector3D CosineWeightedHemisphereSampler3D::get_sample() const {
		float f;
		return get_sample(&f);
	}

	Vector3D CosineWeightedHemisphereSampler3D::get_sample(float* pdf) const {
		// You may implement this, but don't have to.
		// Sampling Concentric Disk
		Vector2D d;
		UniformGridSampler2D uniform;
		// Map uniform random numbers to [-1, 1]^2
		Vector2D uOffset = 2.0 * uniform.get_sample() - Vector2D(1, 1);
		if (uOffset.x == 0 && uOffset.y == 0) {
			d = Vector2D(0, 0);
		}
		//Apply concentric mapping to point
		float theta, r;
		if (fabs(uOffset.x)> fabs(uOffset.y))
		{
			r = uOffset.x;
			theta = M_PI_4 * (uOffset.y / uOffset.x);
		}else {
			r = uOffset.y;
			theta = M_PI_2 - M_PI_4 * (uOffset.x / uOffset.y);
		}
		d = r * Vector2D(cos(theta), sin(theta));
		float z = sqrt(std::max(0.0, 1 - d.x * d.x - d.y * d.y));
		*pdf = z * M_1_PI;
		return Vector3D(d.x, d.y, z);
	}

}  // namespace CMU462
