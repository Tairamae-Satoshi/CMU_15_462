#include "texture.h"
#include "color.h"

#include <assert.h>
#include <iostream>
#include <algorithm>

using namespace std;

namespace CMU462 {

inline void uint8_to_float( float dst[4], unsigned char* src ) {
  uint8_t* src_uint8 = (uint8_t *)src;
  dst[0] = src_uint8[0] / 255.f;
  dst[1] = src_uint8[1] / 255.f;
  dst[2] = src_uint8[2] / 255.f;
  dst[3] = src_uint8[3] / 255.f;
}

inline void float_to_uint8( unsigned char* dst, float src[4] ) {
  uint8_t* dst_uint8 = (uint8_t *)dst;
  dst_uint8[0] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[0])));
  dst_uint8[1] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[1])));
  dst_uint8[2] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[2])));
  dst_uint8[3] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[3])));
}

void Sampler2DImp::generate_mips(Texture& tex, int startLevel) {

  // NOTE: 
  // This starter code allocates the mip levels and generates a level 
  // map by filling each level with placeholder data in the form of a 
  // color that differs from its neighbours'. You should instead fill
  // with the correct data!

  // Task 7: Implement this

  // check start level
  if ( startLevel >= tex.mipmap.size() ) {
    std::cerr << "Invalid start level"; 
  }

  // allocate sublevels
  int baseWidth  = tex.mipmap[startLevel].width;
  int baseHeight = tex.mipmap[startLevel].height;
  int numSubLevels = (int)(log2f( (float)max(baseWidth, baseHeight)));

  numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
  tex.mipmap.resize(startLevel + numSubLevels + 1);

  int width  = baseWidth;
  int height = baseHeight;
  for (int i = 1; i <= numSubLevels; i++) {

    MipLevel& level = tex.mipmap[startLevel + i];

    // handle odd size texture by rounding down
    width  = max( 1, width  / 2); assert(width  > 0);
    height = max( 1, height / 2); assert(height > 0);

    level.width = width;
    level.height = height;
    level.texels = vector<unsigned char>(4 * width * height);

  }

  // fill all 0 sub levels with interchanging colors

  for(size_t i = 1; i < tex.mipmap.size(); ++i) {

    MipLevel& mip = tex.mipmap[i];
	std::vector<unsigned char>& higherlevel_texel = tex.mipmap[i - 1].texels;
    for(size_t j = 0; j <  mip.width; j++) {

		for (size_t k = 0; k < mip.height; k++)
		{
			int pos1 = 4 * (2 * j + 2 * k * tex.mipmap[i - 1].width);
			int pos2 = 4 * (2 * j + 1 + 2 * k * tex.mipmap[i - 1].width);
			int pos3 = 4 * (2 * j + (2 * k + 1) * tex.mipmap[i - 1].width);
			int pos4 = 4 * (2 * j + 1 + (2 * k + 1) * tex.mipmap[i - 1].width);
			mip.texels[4 * (j + k * mip.width)] = ((short)higherlevel_texel[pos1] + (short)higherlevel_texel[pos2] + (short)higherlevel_texel[pos3] + (short)higherlevel_texel[pos4]) >> 2;
			mip.texels[4 * (j + k * mip.width)+1] = ((short)higherlevel_texel[pos1 + 1] + (short)higherlevel_texel[pos2 + 1] + (short)higherlevel_texel[pos3 + 1] + (short)higherlevel_texel[pos4 + 1]) >> 2;
			mip.texels[4 * (j + k * mip.width)+2] = ((short)higherlevel_texel[pos1 + 2] + (short)higherlevel_texel[pos2 + 2] + (short)higherlevel_texel[pos3 + 2] + (short)higherlevel_texel[pos4 + 2]) >> 2;
			mip.texels[4 * (j + k * mip.width)+3] = ((short)higherlevel_texel[pos1 + 3] + (short)higherlevel_texel[pos2 + 3] + (short)higherlevel_texel[pos3 + 3] + (short)higherlevel_texel[pos4 + 3]) >> 2;
			}
    }
  }

}

Color Sampler2DImp::sample_nearest(Texture& tex, 
                                   float u, float v, 
                                   int level) {

  // Task 6: Implement nearest neighbour interpolation
	//Image pixels correspond to samples at half-integer coordinates in texture space
	int tx = floor(u * tex.mipmap[level].width), ty = floor(v * tex.mipmap[level].height);
	int pos = 4 * (tx + ty * tex.mipmap[level].width);
	unsigned char uint8_color[4] = { tex.mipmap[level].texels[pos], tex.mipmap[level].texels[pos + 1], tex.mipmap[level].texels[pos + 2], tex.mipmap[level].texels[pos + 3] };
	float float_color[4];
	uint8_to_float(float_color, uint8_color);
	Color color(float_color[0], float_color[1], float_color[2], float_color[3]);

	return color;

}

Color Sampler2DImp::sample_bilinear(Texture& tex, 
                                    float u, float v, 
                                    int level) {
	// Task 6: Implement bilinear filtering
	float float_color[4];
	float tx = u * tex.mipmap[level].width, ty = v * tex.mipmap[level].height;
	MipLevel& mip = tex.mipmap[level];
	std::vector<unsigned char>& texels = mip.texels;

	//Image pixels correspond to samples at half-integer coordinates in texture space
	float x1 = roundf(tx) - 0.5, x2 = roundf(tx) + 0.5;
	float y1 = roundf(ty) - 0.5, y2 = roundf(ty) + 0.5;
	//Clamp sample points in texture space
	//If sample points are not in the texture space, then sample the nearest pixel.
	float sx1 = x1 < 0.5 ? 0.5 : x1;
	float sx2 = x2 > mip.width + 0.5 ? mip.width + 0.5 : x2;
	float sy1 = y1 < 0.5 ? 0.5 : y1;
	float sy2 = y2 > mip.height + 0.5 ? mip.height + 0.5 : y2;

	int pos = 4 * (floor(sx1) + floor(sy1) * mip.width);
	unsigned char uint8_color11[4] = { texels[pos], texels[pos+1], texels[pos+2], texels[pos+3] };
	uint8_to_float(float_color, uint8_color11);
	Color f11(float_color[0], float_color[1], float_color[2], float_color[3]);

	pos = 4 * (floor(sx2) + floor(sy1) * mip.width);
	unsigned char uint8_color21[4] = { texels[pos], texels[pos+1], texels[pos+2], texels[pos+3] };
	uint8_to_float(float_color, uint8_color21);
	Color f21(float_color[0], float_color[1], float_color[2], float_color[3]);

	pos = 4 * (floor(sx1) + floor(sy2) * mip.width);
	unsigned char uint8_color12[4] = { texels[pos], texels[pos+1], texels[pos+2], texels[pos+3] };
	uint8_to_float(float_color, uint8_color12);
	Color f12(float_color[0], float_color[1], float_color[2], float_color[3]);

	pos = 4 * (floor(sx2) + floor(sy2) * mip.width);
	unsigned char uint8_color22[4] = { texels[pos], texels[pos+1], texels[pos+2], texels[pos+3] };
	uint8_to_float(float_color, uint8_color22);
	Color f22(float_color[0], float_color[1], float_color[2], float_color[3]);

	//Bilinear filtering
	Color color = f11 * (x2 - tx) * (y2 - ty) + f21 * (tx - x1) * (y2 - ty) + f12 * (x2 - tx) * (ty - y1) + f22 * (tx - x1) * (ty - y1);
	
	return color;

}

Color Sampler2DImp::sample_trilinear(Texture& tex, 
                                     float u, float v, 
                                     float u_scale, float v_scale) {

  // Task 7: Implement trilinear filtering
	float L = sqrt(u_scale * u_scale + v_scale * v_scale);
	float d = log2f(L)>=0? log2f(L):0;
	Sampler2DImp sampler_bilinear(BILINEAR);
	Color color1 = sampler_bilinear.sample_bilinear(tex, u, v, floor(d));
	Color color2 = sampler_bilinear.sample_bilinear(tex, u, v, floor(d)+1);

	return color1 * (floor(d) + 1 - d) + color2 * (d - floor(d));

}

} // namespace CMU462
