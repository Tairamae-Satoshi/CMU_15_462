#include "software_renderer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

#include "triangulation.h"

using namespace std;

namespace CMU462 {


// Implements SoftwareRenderer //

void SoftwareRendererImp::draw_svg( SVG& svg ) {

	// set top level transformation
	transformation = svg_2_screen;
  // draw all elements
  for ( size_t i = 0; i < svg.elements.size(); ++i ) {
    draw_element(svg.elements[i]);
  }

  // draw canvas outline
  Vector2D a = transform(Vector2D(    0    ,     0    )); a.x--; a.y--;
  Vector2D b = transform(Vector2D(svg.width,     0    )); b.x++; b.y--;
  Vector2D c = transform(Vector2D(    0    ,svg.height)); c.x--; c.y++;
  Vector2D d = transform(Vector2D(svg.width,svg.height)); d.x++; d.y++;

  rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
  rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
  rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
  rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

  // resolve and send to render target
  resolve();

}

void SoftwareRendererImp::set_sample_rate( size_t sample_rate ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->sample_rate = sample_rate;
  if (this->supersample_target != nullptr)
  {
	  delete[] this->supersample_target;
  }
  this->supersample_target = new unsigned char[4 * this->target_w * sample_rate * this->target_h * sample_rate];
  memset(supersample_target, 255, 4 * target_w * target_h * sample_rate * sample_rate);

}

void SoftwareRendererImp::set_render_target( unsigned char* render_target,
                                             size_t width, size_t height ) {

  // Task 4: 
  // You may want to modify this for supersampling support
	  this->render_target = render_target;
	  this->target_w = width;
	  this->target_h = height;
	  if (this->supersample_target != nullptr)
	  {
		  delete[] this->supersample_target;
	  }
	  this->supersample_target = new unsigned char[4 * this->target_w * sample_rate * this->target_h * sample_rate];
	  memset(supersample_target, 255, 4 * target_w * target_h * sample_rate * sample_rate);
}

void SoftwareRendererImp::draw_element( SVGElement* element ) {

  // Task 5 (part 1):
  // Modify this to implement the transformation stack
	Matrix3x3 temp_transformation = transformation;
	transformation = transformation * element->transform;
  switch(element->type) {
    case POINT:
      draw_point(static_cast<Point&>(*element));
      break;
    case LINE:
      draw_line(static_cast<Line&>(*element));
      break;
    case POLYLINE:
      draw_polyline(static_cast<Polyline&>(*element));
      break;
    case RECT:
      draw_rect(static_cast<Rect&>(*element));
      break;
    case POLYGON:
      draw_polygon(static_cast<Polygon&>(*element));
      break;
    case ELLIPSE:
      draw_ellipse(static_cast<Ellipse&>(*element));
      break;
    case IMAGE:
      draw_image(static_cast<Image&>(*element));
      break;
    case GROUP:
      draw_group(static_cast<Group&>(*element));
      break;
    default:
      break;
  }
  transformation = temp_transformation;
}


// Primitive Drawing //

void SoftwareRendererImp::draw_point( Point& point ) {

  Vector2D p = transform(point.position);
  rasterize_point( p.x, p.y, point.style.fillColor );

}

void SoftwareRendererImp::draw_line( Line& line ) { 

  Vector2D p0 = transform(line.from);
  Vector2D p1 = transform(line.to);
  rasterize_line( p0.x, p0.y, p1.x, p1.y, line.style.strokeColor );

}

void SoftwareRendererImp::draw_polyline( Polyline& polyline ) {

  Color c = polyline.style.strokeColor;

  if( c.a != 0 ) {
    int nPoints = polyline.points.size();
    for( int i = 0; i < nPoints - 1; i++ ) {
      Vector2D p0 = transform(polyline.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polyline.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_rect( Rect& rect ) {

  Color c;
  
  // draw as two triangles
  float x = rect.position.x;
  float y = rect.position.y;
  float w = rect.dimension.x;
  float h = rect.dimension.y;

  Vector2D p0 = transform(Vector2D(   x   ,   y   ));
  Vector2D p1 = transform(Vector2D( x + w ,   y   ));
  Vector2D p2 = transform(Vector2D(   x   , y + h ));
  Vector2D p3 = transform(Vector2D( x + w , y + h ));
  
  // draw fill
  c = rect.style.fillColor;
  if (c.a != 0 ) {
    rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    rasterize_triangle( p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c );
  }

  // draw outline
  c = rect.style.strokeColor;
  if( c.a != 0 ) {
    rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    rasterize_line( p1.x, p1.y, p3.x, p3.y, c );
    rasterize_line( p3.x, p3.y, p2.x, p2.y, c );
    rasterize_line( p2.x, p2.y, p0.x, p0.y, c );
  }

}

void SoftwareRendererImp::draw_polygon( Polygon& polygon ) {

  Color c;

  // draw fill
  
	c = polygon.style.fillColor;
  if( c.a != 0 ) {

    // triangulate
    vector<Vector2D> triangles;
    triangulate( polygon, triangles );

    // draw as triangles
    for (size_t i = 0; i < triangles.size(); i += 3) {
      Vector2D p0 = transform(triangles[i + 0]);
      Vector2D p1 = transform(triangles[i + 1]);
      Vector2D p2 = transform(triangles[i + 2]);
      rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    }
  }

  // draw outline
  c = polygon.style.strokeColor;
  if( c.a != 0 ) {
    int nPoints = polygon.points.size();
    for( int i = 0; i < nPoints; i++ ) {
      Vector2D p0 = transform(polygon.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polygon.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_ellipse( Ellipse& ellipse ) {

  // Extra credit 

}

void SoftwareRendererImp::draw_image( Image& image ) {

  Vector2D p0 = transform(image.position);
  Vector2D p1 = transform(image.position + image.dimension);

  rasterize_image( p0.x, p0.y, p1.x, p1.y, image.tex );
}

void SoftwareRendererImp::draw_group( Group& group ) {

  for ( size_t i = 0; i < group.elements.size(); ++i ) {
    draw_element(group.elements[i]);
  }

}

// Rasterization //

// The input arguments in the rasterization functions 
// below are all defined in screen space coordinates

void SoftwareRendererImp::rasterize_point( float x, float y, Color color ) {

	// fill in the nearest pixel
	int sx = (int)floor(x * sample_rate);
	int sy = (int)floor(y * sample_rate);
	int width = target_w * sample_rate;
	int height = target_h * sample_rate;
	if (color.r==1&& color.g == 1&& color.b == 1)
	{
		cout << color.a << endl;
	}
	// check bounds
	if ( sx < 0 || sx >= width) return;
	if ( sy < 0 || sy >= height) return;
	float Er = color.r, Eg = color.g, Eb = color.b, Ea = color.a;
	float Cr = supersample_target[4 * (sx + sy * width)] / 255.0f, Cg = supersample_target[4 * (sx + sy * width) + 1] / 255.0f, Cb = supersample_target[4 * (sx + sy * width) + 2] / 255.0f, Ca = supersample_target[4 * (sx + sy * width) + 3] / 255.0f;

	// fill sample - NOT doing alpha blending!
	supersample_target[4 * (sx + sy * width)] = (uint8_t)(min(((1 - Ea) * Cr + Er) ,1.0f) * 255);
	supersample_target[4 * (sx + sy * width) + 1] = (uint8_t)(min(((1 - Ea) * Cg + Eg),1.0f) * 255);
	supersample_target[4 * (sx + sy * width) + 2] = (uint8_t)(min(((1 - Ea) * Cb + Eb),1.0f) *255);
	supersample_target[4 * (sx + sy * width) + 3] = (uint8_t)(min(((1 - Ea) * Ca + Ea) ,1.0f) * 255);

}

int ipart(float x)
{
	return floor(x);
}

float fpart(float x)
{
	return x - floor(x);
}

float rfpart(float x)
{
	return 1 - fpart(x);
}

void SoftwareRendererImp::rasterize_line( float x0, float y0,
                                          float x1, float y1,
                                          Color color) {

  // Task 2: 
  // Implement line rasterization
	bool steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep)
	{
		swap(x0, y0);
		swap(x1, y1);
	}
	if (x0>x1)
	{
		swap(x0, x1);
		swap(y0, y1);
	}
	float dx = x1 - x0;
	float dy = y1 - y0;
	float gradient;
	if (dx==0)
	{
		gradient = 1.0f;

	}
	else
	{
		gradient = dy / dx;
	}

	//handle first endpoint
	float xend = round(x0);
	float yend = y0 + gradient * (xend - x0);
	float xgap = rfpart(x0+0.5);
	int xpxl1 = xend;
	int ypxl1 = ipart(yend);
	if (steep)
	{
		rasterize_point(ypxl1, xpxl1, color*rfpart(yend)*xgap);
		rasterize_point(ypxl1+1, xpxl1, color * fpart(yend) * xgap);
	}
	else
	{
		rasterize_point(xpxl1, ypxl1, color * rfpart(yend) * xgap);
		rasterize_point(xpxl1, ypxl1+1, color * fpart(yend) * xgap);
	}
	float intery = yend + gradient;//first y-intersection for the main loop

	//handle second endpoint
	xend = round(x1);
	yend = y1 + gradient * (xend - x1);
	xgap = fpart(x1 + 0.5);
	int xpxl2 = xend;
	int ypxl2 = ipart(yend);
	if (steep)
	{
		rasterize_point(ypxl2, xpxl2, color * rfpart(yend) * xgap);
		rasterize_point(ypxl2 + 1, xpxl2, color * fpart(yend) * xgap);
	}
	else
	{
		rasterize_point(xpxl2, ypxl2, color * rfpart(yend) * xgap);
		rasterize_point(xpxl2, ypxl2 + 1, color * fpart(yend) * xgap);
	}

	if (steep)
	{
		for (int x = xpxl1+1; x < xpxl2-1; x++)
		{
			rasterize_point(ipart(intery), x, color * rfpart(intery));
			rasterize_point(ipart(intery)+1, x, color * fpart(intery));
			intery = intery + gradient;
		}
	}
	else
	{
		for (int x = xpxl1 + 1; x < xpxl2 - 1; x++)
		{
			rasterize_point(x, ipart(intery), color * rfpart(yend));
			rasterize_point(x, ipart(intery)+1, color * fpart(yend));
			intery = intery + gradient;
		}
	}

	/*int sx0, sy0, sx1, sy1;
	int absdx = abs((int)floor(x0) - (int)floor(x1));
	int absdy = abs((int)floor(y0) - (int)floor(y1));
	if ((absdx>= absdy&&x0<=x1)|| (absdx < absdy && y0 <= y1))
	{
		sx0 = (int)floor(x0);
		sy0 = (int)floor(y0);
		sx1 = (int)floor(x1);
		sy1 = (int)floor(y1);
	}
	else
	{
		sx0 = (int)floor(x1);
		sy0 = (int)floor(y1);
		sx1 = (int)floor(x0);
		sy1 = (int)floor(y0);
	}
	int	dx = sx1 - sx0,
		dy = sy1 - sy0,
		sx = sx0,
		sy = sy0,
		eps = 0;
	
	if (absdx >= absdy)
	{

		if (dy >= 0)
		{
			for (int sx = sx0; sx <= sx1; sx++) {
				rasterize_point((float)sx, (float)sy, color);
				eps += dy;
				if ((eps << 1) >= dx) {
					sy++;  eps -= dx;
				}
			}
		}
		else
		{
			for (int sx = sx0; sx <= sx1; sx++) {
				rasterize_point((float)sx, (float)sy, color);
				eps += dy;
				if ((eps << 1) <= -dx) {
					sy--;  eps += dx;
				}
			}
		}
	}
	else
	{
		if (dx >= 0)
		{
			for (int sy = sy0; sy <= sy1; sy++) {
				rasterize_point((float)sx, (float)sy, color);
				eps += dx;
				if ((eps << 1) >= dy) {
					sx++;  eps -= dy;
				}
			}
		}
		else
		{
			for (int sy = sy0; sy <= sy1; sy++) {
				rasterize_point((float)sx, (float)sy, color);
				eps += dx;
				if ((eps << 1) <= -dy) {
					sx--;  eps += dy;
				}
			}
		}
	}*/
	
	

}

void SoftwareRendererImp::rasterize_triangle( float x0, float y0,
                                              float x1, float y1,
                                              float x2, float y2,
                                              Color color ) {
  // Task 3: 
  // Implement triangle rasterization
	bool flag = true;
	float left = min(min(x0,x1),x2);
	float right = max(max(x0, x1), x2);
	float bottom = min(min(y0, y1), y2);
	float top = max(max(y0, y1), y2);

	float A0 = y1 - y0, B0 = x0 - x1, C0 = y0 * (x1 - x0) - x0 * (y1 - y0),
		A1 = y2 - y1, B1 = x1 - x2, C1 = y1 * (x2 - x1) - x1 * (y2 - y1),
		A2 = y0 - y2, B2 = x2 - x0, C2 = y2 * (x0 - x2) - x2 * (y0 - y2);

	float interval = 1 / (float)sample_rate;
	for (float x = (float)floor(left)+ interval/2; x <= right; x+= interval)
	{
		for (float y = (float)floor(bottom) + interval/2; y <= top; y+= interval)
		{
			float E0 = A0 * x + B0 * y + C0;
			float E1 = A1 * x + B1 * y + C1;
			float E2 = A2 * x + B2 * y + C2;

			if ((E0 < 0 && E1 < 0 && E2 < 0)|| (E0 > 0 && E1 > 0 && E2 > 0))
			{
				rasterize_point(x, y, color);
			}
			else if (E0 == 0 && ((y0 == y1 && y0== top)|| x0 == left || x1 == left))
			{
				rasterize_point(x, y, color);
			}
			else if (E1 == 0 && ((y1 == y2 && y1 == top) || x1 == left || x2 == left))
			{
				rasterize_point(x, y, color);
			}
			else if (E2 == 0 && ((y2 == y0 && y2 == top) || x2 == left || x0 == left))
			{
				rasterize_point(x, y, color);
			}
		}
	}
	
}

void SoftwareRendererImp::rasterize_image( float x0, float y0,
                                           float x1, float y1,
                                           Texture& tex ) {
  // Task 6: 
  // Implement image rasterization
	Sampler2DImp sampler(BILINEAR);
	float interval = 1 / (float)sample_rate;
	float dx = x1 - x0, dy = y1 - y0;

	for (float x = (float)floor(x0) + interval / 2; x <= x1; x += interval)
	{
		for (float y = (float)floor(y0) + interval / 2; y <= y1; y += interval)
		{
			float u_scale = (interval / dx);
			float v_scale = (interval / dy);
			//Color color(sampler.sample_bilinear(tex, (x - x0) / dx, (y - y0) / dy, 0));
			Color color(sampler.sample_trilinear(tex, (x - x0) / dx, (y - y0) / dy, u_scale, v_scale));
			rasterize_point(x, y, color);
		}
	}
}

// resolve samples to render target
void SoftwareRendererImp::resolve( void ) {

  // Task 4: 
  // Implement supersampling
  // You may also need to modify other functions marked with "Task 4".
	int supersample_rate_square = sample_rate * sample_rate;

	for (int sx = 0; sx < target_w; sx++)
	{
		for (int sy = 0; sy < target_h; sy++)
		{
			unsigned int temp_r = 0, temp_g = 0, temp_b = 0, temp_a = 0;
			for (int i = 0; i < sample_rate; i++)
			{
				for (int j = 0; j < sample_rate; j++)
				{
					unsigned int pos = 4 * (sx * sample_rate + i + (sy * sample_rate + j) * target_w * sample_rate);
					temp_r += supersample_target[pos];
					temp_g += supersample_target[pos + 1];
					temp_b += supersample_target[pos + 2];
					temp_a += supersample_target[pos + 3];
				}
			}
			render_target[4 * (sx + sy * target_w)] = (uint8_t)(temp_r / supersample_rate_square);
			render_target[4 * (sx + sy * target_w) + 1] = (uint8_t)(temp_g / supersample_rate_square);
			render_target[4 * (sx + sy * target_w) + 2] = (uint8_t)(temp_b / supersample_rate_square);
			render_target[4 * (sx + sy * target_w) + 3] = (uint8_t)(temp_a / supersample_rate_square);
		}
	}
	memset(supersample_target, 255, 4 * target_w * target_h * supersample_rate_square);
	return;

}


} // namespace CMU462
