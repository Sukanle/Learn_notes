#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float mixValue;

void main() {
    // linearly interpolate between both textures (80% container, 20% awesomeface)
    FragColor = mix(texture(texture0, TexCoord),
            // FragColor = mix(texture(texture1, TexCoord),
            texture(texture1, TexCoord), mixValue);
}
