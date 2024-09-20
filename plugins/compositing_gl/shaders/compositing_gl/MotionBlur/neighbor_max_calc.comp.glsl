#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D tileMaxBuffer; // Tile max velocity buffer

layout(rgba16f, binding = 0) writeonly uniform image2D neighborMaxBuffer; // Output neighbor max buffer

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy); // Current pixel coordinates in the neighbor max buffer
    ivec2 target_res = imageSize(neighborMaxBuffer); // Resolution of the neighbor max buffer

    // Bounds check to ensure pixel_coords is within the valid range
    if (pixel_coords.x >= target_res.x || pixel_coords.y >= target_res.y) {
        return;
    }

    // Initialize vmax with zero velocity
    vec2 vmax = vec2(0.0);

    // Loop over neighboring tiles within the blur radius
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ivec2 current_pixel = pixel_coords + ivec2(i, j); // Neighbor pixel

            // Bounds check to ensure current_pixel is within the valid range of the tileMaxBuffer
            ivec2 tileMax_res = textureSize(tileMaxBuffer, 0); // Resolution of tileMaxBuffer
            if (current_pixel.x >= 0 && current_pixel.y >= 0 &&
                current_pixel.x < tileMax_res.x && current_pixel.y < tileMax_res.y) {

                // Fetch the velocity for the current tile
                vec2 v_current = texelFetch(tileMaxBuffer, current_pixel, 0).xy;
                vmax = max(v_current, vmax); // Update vmax with the maximum velocity
            }
        }
    }

    // Store the maximum velocity in the neighbor max buffer
    imageStore(neighborMaxBuffer, pixel_coords, vec4(vmax, 0.0, 1.0));
}
