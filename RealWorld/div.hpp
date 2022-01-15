#pragma once
#include <glm/vec2.hpp>


inline div_t floor_div(int a, int b) {
	assert(b > 0);
	div_t r = div(a, b);
	if (r.rem != 0 && a < 0) {
		r.quot--;
		r.rem += b;
	}
	return r;
}

struct ivec2_div_t {
	glm::ivec2 quot;
	glm::ivec2 rem;
};

inline ivec2_div_t floor_div(glm::ivec2 a, glm::ivec2 b) {
	assert(b.x > 0 && b.y > 0);
	ivec2_div_t r;
	div_t x = div(a.x, b.x);
	if (x.rem != 0 && a.x < 0) {
		x.quot--;
		x.rem += b.x;
	}
	div_t y = div(a.y, b.y);
	if (y.rem != 0 && a.y < 0) {
		y.quot--;
		y.rem += b.y;
	}
	r.quot = { x.quot, y.quot };
	r.rem = { x.rem, y.rem };
	return r;
}


inline glm::ivec2 floor_modulo(glm::ivec2 a, glm::ivec2 b) {
	assert(b.x > 0 && b.y > 0);
	glm::ivec2 r = a % b;
	if (a.x < 0) r.x += b.x;
	if (a.y < 0) r.y += b.y;
	return r;
}