R""(
in vec2 fragUV;

out vec4 color;

layout(location = 1) uniform sampler2D baseTexture;

void main() {;
    color = texture(baseTexture, fragUV);
}

)""