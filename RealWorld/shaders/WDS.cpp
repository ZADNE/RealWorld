#include "WDS.hpp"

#ifdef __INTELLISENSE__
#pragma diag_suppress 29
#endif

const char* WDS::tilesDraw_vert =
	#include "tilesDraw.vert"
;

const char* WDS::tilesDraw_frag =
	#include "tilesDraw.frag"
;

const char* WDS::finalLighting_vert =
	#include "finalLighting.vert"
;

const char* WDS::PT_vert =
	#include "PT.vert"
;

const char* WDS::combineLighting_frag =
	#include "combineLighting.frag"
;

const char* WDS::worldToLight_frag =
	#include "worldToLight.frag"
;

const char* WDS::addStaticLight_vert =
	#include "addStaticLight.vert"
;

const char* WDS::sumStaticLight_frag =
	#include "sumStaticLight.frag"
;

const char* WDS::addStaticLight_frag =
	#include "addStaticLight.frag"
;

const char* WDS::addDynamicLight_vert =
	#include "addDynamicLight.vert"
;

const char* WDS::addDynamicLight_frag =
	#include "addDynamicLight.frag"
;

#ifdef _DEBUG

const char* frag_combineLightingCONCEPT = {
		"#version 440\n"

		"layout(location = 2) out vec4 colour;"

		"layout(location = 100) uniform sampler2D diaphragm;"
		"layout(location = 101) uniform sampler2D lighting;"

		"const int radius = 15;"//Main diameter constant
		"const int maxr = int(ceil(float(radius) * 0.70710678118f));"//'Radius' for inner square = ceil(radius * sqrt(2) / 2))
		"const int bp_size = (maxr + 1) * 8;"
		"vec4 bp[bp_size];"

		"void processBlock(inout vec4 light, in ivec2 pos){"
		//Direction check missing!
		"vec4 dia = texelFetch(diaphragm, pos, 0);"
		"vec4 this_light = texelFetch(lighting, pos, 0);"
		"light += this_light;"
		"light *= dia.r;"
	"}"

	"void processBlock(inout vec4 light, in ivec2 pos, vec4 sideinput){"
		//Direction check missing!
		"vec4 dia = texelFetch(diaphragm, pos, 0);"
		"vec4 this_light = texelFetch(lighting, pos, 0);"
		"light += this_light;"
		"light *= dia.r;"
		"light += sideinput;"
	"}"

	"void shift(inout vec4 bp[bp_size], const ivec2 center, const int maxr, const int r, const int ext, const bool fullside) {"
		"if (r % 2 == 0) {"//Orthogonal merging
			"int j = (-r + 1) / 2;"
			"int i = (maxr + 1 + j) * 4;"
			"bp[i + 0] += bp[i - 4];"
			"bp[i + 1] += bp[i - 3];"
			"bp[i + 2] += bp[i - 2];"
			"bp[i + 3] += bp[i - 1];"
			"j = (r - 1) / 2;"
			"i = (maxr + 1 + j) * 4;"
			"bp[i + 0] += bp[i + 4];"
			"bp[i + 1] += bp[i + 5];"
			"bp[i + 2] += bp[i + 6];"
			"bp[i + 3] += bp[i + 7];"
		"} else {"//Diagonal merging
			"int j = (-r + 1) / 2 - 2;"
			"int i = (maxr + 1 + j) * 4;"
			"bp[i + 0] += bp[i + 4];"
			"bp[i + 1] += bp[i + 5];"
			"bp[i + 2] += bp[i + 6];"
			"bp[i + 3] += bp[i + 7];"
			"j = (r - 1) / 2 + 1;"
			"i = (maxr + 1 + j) * 4;"
			"bp[i + 4] += bp[i + 0];"
			"bp[i + 5] += bp[i + 1];"
			"bp[i + 6] += bp[i + 2];"
			"bp[i + 7] += bp[i + 3];"
		"}"//First half of diagonal shift
		"int j = (-r + 1) / 2 - 1;"
		"int i = (maxr + 1 + j) * 4;"
		"for (; j > -r - 1; j--, i -= 4) {"
			"processBlock(bp[i + 0], center + ivec2(ext, j),	bp[i - 4]);"
			"processBlock(bp[i + 1], center + ivec2(-j, ext),	bp[i - 3]);"
			"processBlock(bp[i + 2], center + ivec2(-ext, -j),	bp[i - 2]);"
			"processBlock(bp[i + 3], center + ivec2(j, -ext),	bp[i - 1]);"
		"}"
		//Orthogonal shift
		"j = (-r + 1) / 2;"
		"i = (maxr + 1 + j) * 4;"
		"for (; j < (r - 1) / 2 + 1; j++, i += 4) {"
			"processBlock(bp[i + 0], center + ivec2(ext, j));"
			"processBlock(bp[i + 1], center + ivec2(-j, ext));"
			"processBlock(bp[i + 2], center + ivec2(-ext, -j));"
			"processBlock(bp[i + 3], center + ivec2(j, -ext));"
		"}"
		//Second half of diagonal shift
		"for (; j < (fullside ? r + 1 : r); j++, i += 4) {"
			"processBlock(bp[i + 0], center + ivec2(ext, j),	bp[i + 4]);"
			"processBlock(bp[i + 1], center + ivec2(-j, ext),	bp[i + 5]);"
			"processBlock(bp[i + 2], center + ivec2(-ext, -j),	bp[i + 6]);"
			"processBlock(bp[i + 3], center + ivec2(j, -ext),	bp[i + 7]);"
		"}"
	"}"

	"void main() {"
		"vec4 final = vec4(0.0, 0.0, 0.0, 0.0);"
		"ivec2 center = ivec2(gl_FragCoord.xy);"

		//Move to initializer \|/
		"for (int x = 0; x < bp.length(); x++) {"
			"bp[x] = vec4(0.0, 0.0, 0.0, 0.0);"
		"}"

		//Segments
		"int r = radius;"
		"float radius_squared = (float(radius) + 0.5) * (float(radius) + 0.5);"
		"for (float helper = float(radius); r > maxr; --r, --helper) {"
			"shift(bp, center, maxr, int(sqrt(radius_squared - helper * helper)), r, true);"
		"}"
		//Main square
		"for (; r > 0; --r) {"
			"shift(bp, center, maxr, r, r, false);"
		"}"
		//Final sum
		"for (int i = maxr * 4; i < maxr * 4 + 12; ++i) {"
			"final += bp[i];"
		"}"
		"processBlock(final, center);"
		"colour = vec4(final.rgb, 1.0 - sqrt(final.a));"
	"}"
	};

	#include <stdio.h>
	#include <math.h>
	//SHADER SIMULATION FUNCTIONS \|/
	void shift(int input[][61], int bp[], const int cntr_x, const int cntr_y, const int maxr, const int r, const int ext, const bool fullside) {
		/*
		C -> input[cntr_x][contr_y]
		length(____) -> ext
		length(||) -> r
		length(I_||) -> 2r

			I
		C____
			|
			|

		input[C + ext][C - r ... C + r - 1] AND ALL 4 ROTATED VERSIONS get processed into backpack when !fullside
		input[C + ext][C - r ... C + r]		AND ALL 4 ROTATED VERSIONS get processed into backpack when fullside
		*/
		if (r % 2 == 0) {//Orthogonal merging
			int j = (-r + 1) / 2;
			int i = (maxr + 1 + j) * 4;
			bp[i + 0] += bp[i - 4];
			bp[i + 1] += bp[i - 3];
			bp[i + 2] += bp[i - 2];
			bp[i + 3] += bp[i - 1];
			j = (r - 1) / 2;
			i = (maxr + 1 + j) * 4;
			bp[i + 0] += bp[i + 4];
			bp[i + 1] += bp[i + 5];
			bp[i + 2] += bp[i + 6];
			bp[i + 3] += bp[i + 7];
		}
		else {//Diagonal merging
			int j = (-r + 1) / 2 - 2;
			int i = (maxr + 1 + j) * 4;
			bp[i + 0] += bp[i + 4];
			bp[i + 1] += bp[i + 5];
			bp[i + 2] += bp[i + 6];
			bp[i + 3] += bp[i + 7];
			j = (r - 1) / 2 + 1;
			i = (maxr + 1 + j) * 4;
			bp[i + 4] += bp[i + 0];
			bp[i + 5] += bp[i + 1];
			bp[i + 6] += bp[i + 2];
			bp[i + 7] += bp[i + 3];
		}
		//First half of diagonal shift
		int j = (-r + 1) / 2 - 1;
		int i = (maxr + 1 + j) * 4;
		for (; j > -r - 1; j--, i -= 4) {
			bp[i + 0] = input[cntr_x + ext][cntr_y + j] + bp[i - 4];
			bp[i + 1] = input[cntr_x - j][cntr_y + ext] + bp[i - 3];
			bp[i + 2] = input[cntr_x - ext][cntr_y - j] + bp[i - 2];
			bp[i + 3] = input[cntr_x + j][cntr_y - ext] + bp[i - 1];

			//Test only \|/
			input[cntr_x + ext][cntr_y + j] = bp[i + 0];
			input[cntr_x - j][cntr_y + ext] = bp[i + 1];
			input[cntr_x - ext][cntr_y - j] = bp[i + 2];
			input[cntr_x + j][cntr_y - ext] = bp[i + 3];
		}
		//Orthogonal shift
		j = (-r + 1) / 2;
		i = (maxr + 1 + j) * 4;
		for (; j < (r - 1) / 2 + 1; j++, i += 4) {
			bp[i + 0] += input[cntr_x + ext][cntr_y + j];
			bp[i + 1] += input[cntr_x - j][cntr_y + ext];
			bp[i + 2] += input[cntr_x - ext][cntr_y - j];
			bp[i + 3] += input[cntr_x + j][cntr_y - ext];

			//Test only \|/
			input[cntr_x + ext][cntr_y + j] = bp[i + 0];
			input[cntr_x - j][cntr_y + ext] = bp[i + 1];
			input[cntr_x - ext][cntr_y - j] = bp[i + 2];
			input[cntr_x + j][cntr_y - ext] = bp[i + 3];
		}
		//Second half of diagonal shift
		for (; j < (fullside ? r + 1 : r); j++, i += 4) {
			bp[i + 0] = input[cntr_x + ext][cntr_y + j] + bp[i + 4];
			bp[i + 1] = input[cntr_x - j][cntr_y + ext] + bp[i + 5];
			bp[i + 2] = input[cntr_x - ext][cntr_y - j] + bp[i + 6];
			bp[i + 3] = input[cntr_x + j][cntr_y - ext] + bp[i + 7];

			//Test only \|/
			input[cntr_x + ext][cntr_y + j] = bp[i + 0];
			input[cntr_x - j][cntr_y + ext] = bp[i + 1];
			input[cntr_x - ext][cntr_y - j] = bp[i + 2];
			input[cntr_x + j][cntr_y - ext] = bp[i + 3];
		}
	}

	void backpack_light(){
		int test[61][61];
		for (size_t x = 0; x < 61; x++) {
			for (size_t y = 0; y < 61; y++) {
				test[x][y] = 1;
			}
		}

		const int radius = 30;//Main value
		const int maxr = 22;//(int)((float)radius * 0.70710678118f);//'Radius' for inner square, ceil(radius * sqrt(2) / 2))
		int bp[(maxr + 1) * 8];
		for (size_t x = 0; x < sizeof(bp) / sizeof(bp[0]); x++) {
			bp[x] = 0;
		}
		//Segments
		int r = radius;
		double radius_squared = ((double)radius + 0.5) * ((double)radius + 0.5);
		printf("maxr = %i\n", maxr);
		for (double helper = (double)radius; r > maxr; --r, --helper) {
			double exact = sqrt(radius_squared - helper * helper);
			int tmp = (int)sqrt(radius_squared - helper * helper);
			printf("r = %f -> %i, ext = %i, fullside = %i\n", exact, tmp, r, true);
			shift(test, bp, 30, 30, maxr, tmp, r, true);
		}
		for (int i = sizeof(bp) / sizeof(bp[0]) - 4; i >= 0; i -= 4) {
			printf("%i\n", bp[i]);
		}

		//Main square
		for (; r > 0; --r) {
			shift(test, bp, 30, 30, maxr, r, r, false);
		}
		//Final sum
		for (int i = maxr * 4; i < maxr * 4 + 12; ++i) {
			test[30][30] += bp[i];
		}

		for (int y = 60; y >= 0; --y) {
			for (int x = 0; x < 61; x++) {
				int n = test[x][y];
				if (n > 1) {
					printf("|%2i", n);
				}
				else if (n == 0) {
					printf("   ");
				}
				else {
					printf("|XX");
				}
			}
			printf("|\n");
		}
		int stop = 1;
	}
	#endif // _DEBUG
