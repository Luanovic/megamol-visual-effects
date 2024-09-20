#version 460

layout(local_size_x = 1, local_size_y = 1) in;

// Low-resolution tile-based buffer (e.g., tileMaxBuffer)
layout(binding = 0) uniform sampler2D neighborMaxBuffer;

// Full-resolution output buffer
layout(rgba16f, binding = 0) writeonly uniform image2D outputTex;

uniform int maxBlurRadius; 


void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy); 

    int tileSize = maxBlurRadius * 2 + 1;
    vec4 currentMaxVelocity = texelFetch(neighborMaxBuffer, pixel_coords, 0);

    for (int i = 0; i <= tileSize - 1 ; i++) {
        for (int j = 0; j <= tileSize - 1 ; j++) {
            ivec2 current_pixel = pixel_coords * tileSize + ivec2(i, j);

            ivec2 velocity_res = imageSize(outputTex);
            if (current_pixel.x >= 0 && current_pixel.y >= 0 &&
                current_pixel.x < velocity_res.x && current_pixel.y < velocity_res.y) {
                imageStore(outputTex, current_pixel, currentMaxVelocity);
            }
        }
    }
}
