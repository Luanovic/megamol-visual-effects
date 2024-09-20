#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D velocityBuffer;

uniform int maxBlurRadius;

layout(rgba16f, binding = 0) writeonly uniform image2D tileMaxBuffer;

ivec2 getTileCoordinates(ivec2 pixel_coords) {
    int tileSize = maxBlurRadius * 2 + 1;
    return pixel_coords / tileSize;
}

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy); 
    ivec2 target_res = imageSize(tileMaxBuffer); 

    // Bounds check to ensure we're within the valid range
    if (pixel_coords.x >= target_res.x || pixel_coords.y >= target_res.y) {
        return;
    }

    // Check if the pixel is aligned to the start of a tile (tile boundary check)
    int tileSize = maxBlurRadius * 2 + 1;
    if (pixel_coords.x % tileSize != 0 || pixel_coords.y % tileSize != 0) {
        return;
    }

    // Compute the maximum velocity in the current tile
    vec2 vmax = vec2(0.0);
    for (int i = 0; i <= tileSize - 1 ; i++) {
        for (int j = 0; j <= tileSize - 1 ; j++) {
            ivec2 current_pixel = pixel_coords + ivec2(i, j);

            ivec2 velocity_res = textureSize(velocityBuffer, 0);
            if (current_pixel.x >= 0 && current_pixel.y >= 0 &&
                current_pixel.x < velocity_res.x && current_pixel.y < velocity_res.y) {
                vec2 v_current = texelFetch(velocityBuffer, current_pixel, 0).xy;
                vmax = max(v_current, vmax); 
            }
        }
    }

    imageStore(tileMaxBuffer, getTileCoordinates(pixel_coords), vec4(vmax, length(vmax), 1.0));
}
