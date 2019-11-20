#include "viewport.h"

#include "CMU462.h"

namespace CMU462 {

void ViewportImp::set_viewbox( float centerX, float centerY, float vspan ) {

  // Task 5 (part 2): 
  // Set svg coordinate to normalized device coordinate transformation. Your input
  // arguments are defined as normalized SVG canvas coordinates.
  this->centerX = centerX;
  this->centerY = centerY;
  this->vspan = vspan; 
  float vspan_mul_2 = vspan * 2;
  double data[9] = { 1 / vspan_mul_2,0,0.5 - centerX / vspan_mul_2,0,1 / vspan_mul_2,0.5 - centerY / vspan_mul_2,0,0,1 };
  Matrix3x3 svg_2_norm(data);
  set_svg_2_norm(svg_2_norm);
}

void ViewportImp::update_viewbox( float dx, float dy, float scale ) { 
  
  this->centerX -= dx;
  this->centerY -= dy;
  this->vspan *= scale;
  set_viewbox( centerX, centerY, vspan );
}

} // namespace CMU462
