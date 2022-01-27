R""(
#version 460

out uvec4 result;

layout(location = 5) uniform uvec4 set;

void main() {
	result = set;
}

)""