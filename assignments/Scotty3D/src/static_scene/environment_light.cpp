#include "environment_light.h"
#include<random>
#include<algorithm>
namespace CMU462 {
	namespace StaticScene {
		static std::uniform_real_distribution<double> dMap(0.0, 1.0);
		static std::default_random_engine engine;

		void EnvironmentLight::Init(const std::vector<double>& pMap)
		{
			std::vector<double> p_theta;
			std::vector<std::vector<double>> p_phi_under_theta;
			size_t w = envMap->w;
			size_t h = envMap->h;
			p_theta.resize(h);
			F_theta.resize(h);
			p_phi_under_theta.resize(h);
			F_phi_under_theta.resize(h);
			for (size_t i = 0; i < h; ++i)
			{
				p_phi_under_theta[i].resize(w);
				F_phi_under_theta[i].resize(w);
			}
			

			for (size_t y = 0; y < h; y++)
			{
				p_theta[y] = 0.0;
				for (size_t x = 0; x < w; x++)
				{
					p_theta[y] += pMap[x + y * w];
				}
				F_theta[y] = y == 0 ? p_theta[y] : F_theta[y - 1] + p_theta[y];

			}

			for (size_t y = 0; y < h; y++)
			{
				for (size_t x = 0; x < w; x++)
				{
					p_phi_under_theta[y][x] = pMap[x + y * w] / p_theta[y];
					F_phi_under_theta[y][x] = x == 0 ? p_phi_under_theta[y][x] : F_phi_under_theta[y][x - 1] + p_phi_under_theta[y][x];
				}
			}

		}

		double EnvironmentLight::Sample(double uniform1, double uniform2, double& texcX, double& texcY) const {
			int w = envMap->w;
			int h = envMap->h;

			auto y_iter = std::upper_bound(F_theta.begin(), F_theta.end(), uniform1);
			int y = y_iter - F_theta.begin();
			auto x_iter = std::upper_bound(F_phi_under_theta[y].begin(), F_phi_under_theta[y].end(), uniform2);
			int x = x_iter - F_phi_under_theta[y].begin();
			texcX = x + dMap(engine) / w;
			texcY = y + dMap(engine) / h;

			return pMap[x + y * w];
		}

		EnvironmentLight::EnvironmentLight(const HDRImageBuffer* envMap)
			: envMap(envMap) {
			// TODO: (PathTracer) initialize things here as needed
			size_t w = envMap->w;
			size_t h = envMap->h;
			double thetaStep = PI / h;
			double sum = 0;
			pMap.resize(w * h);
			double theta = 0.5 * thetaStep;
			for (size_t y = 0; y < h; y++, theta+=thetaStep)
			{
				for (size_t x = 0; x < w; x++)
				{
					size_t idx = x + y * w;
					pMap[idx] = envMap->data[idx].illum() * sin(theta);
					sum += pMap	[idx];
				}
			}

			for (size_t i = 0; i < pMap.size(); i++)
			{
				pMap[i] /= sum;
			}

			Init(pMap);
		}

		Spectrum EnvironmentLight::sample_L(const Vector3D& p, Vector3D* wi,
			float* distToLight, float* pdf) const {
			// TODO: (PathTracer) Implement
			size_t w = envMap->w;
			size_t h = envMap->h;
			double texcX;
			double texcY;

			*pdf = Sample(dMap(engine), dMap(engine), texcX, texcY);

			double theta = PI * texcY;
			double phi = 2 * PI * texcX;

			wi->x = sin(theta) * sin(phi);
			wi->y = cos(theta);
			wi->z = sin(theta) * cos(phi);

			*distToLight = FLT_MAX;

			return sample_dir(Ray(p, *wi));
		}

		float EnvironmentLight::pdf(const Vector3D& p, const Vector3D& wi) {
			size_t w = envMap->w;
			size_t h = envMap->h;

			double theta = acos(wi.y);
			double phi = atan2(wi.x, -wi.z) + PI;

			double texcX = phi / (2.0 * PI);
			double texcY = theta / PI;

			int x = texcX * w;
			int y = texcY * h;
			return pMap[x + y * w];
		}

		Spectrum EnvironmentLight::sample_dir(const Ray& r) const {
			// TODO: (PathTracer) Implement
			size_t w = envMap->w;
			size_t h = envMap->h;

			Vector3D dir = r.d.unit();
			double theta = acos(dir.y);
			double phi = atan2(dir.x, -dir.z) + PI;

			double tx = phi / (2.0 * PI) * w;
			double ty = theta / PI * h;

			//Image pixels correspond to samples at half-integer coordinates in texture space
			float x1 = roundf(tx) - 0.5, x2 = roundf(tx) + 0.5;
			float y1 = roundf(ty) - 0.5, y2 = roundf(ty) + 0.5;
			//Clamp sample points in texture space
			//If sample points are not in the texture space, then sample the nearest pixel.
			
			float sx1 = clamp(x1, 0.5, w - 0.5);
			float sx2 = clamp(x2, 0.5, w - 0.5);
			float sy1 = clamp(y1, 0.5, h - 0.5);
			float sy2 = clamp(y2, 0.5, h - 0.5);

			Spectrum f11 = envMap->data[floor(sx1) + floor(sy1) * w];
			Spectrum f21 = envMap->data[floor(sx2) + floor(sy1) * w];
			Spectrum f12 = envMap->data[floor(sx1) + floor(sy2) * w];
			Spectrum f22 = envMap->data[floor(sx2) + floor(sy2) * w];

			Spectrum color = f11 * (x2 - tx) * (y2 - ty) + f21 * (tx - x1) * (y2 - ty) + f12 * (x2 - tx) * (ty - y1) + f22 * (tx - x1) * (ty - y1);

			return color;
		
		}

	}  // namespace StaticScene
}  // namespace CMU462
