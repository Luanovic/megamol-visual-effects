#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D velocityBuffer;

uniform int maxBlurRadius;

layout(rgba16f, binding = 0) writeonly uniform image2D tileMaxBuffer;

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy); 
    ivec2 target_res = imageSize(tileMaxBuffer); 
    int tileSize = maxBlurRadius * 2 + 1;

    if (pixel_coords.x >= target_res.x || pixel_coords.y >= target_res.y) {
        return;
    }

    if(pixel_coords % tileSize != ivec2(0)) {
        return;
    }


    // Compute the maximum velocity in the current tile
    vec2 vmax = vec2(0.0);
    for (int i = 0; i <= tileSize - 1 ; i++) {
        for (int j = 0; j <= tileSize - 1 ; j++) {

            ivec2 tile_pixel_coords = pixel_coords + ivec2(i, j);
            vec2 v_current = texelFetch(velocityBuffer, tile_pixel_coords, 0).xy;
            vmax = max(v_current, vmax); 

        }
    }

    for (int i = 0; i <= tileSize - 1 ; i++) {
        for (int j = 0; j <= tileSize - 1 ; j++) {
            ivec2 tile_pixel_coords = pixel_coords + ivec2(i, j);
            imageStore(tileMaxBuffer, tile_pixel_coords, vec4(vmax, length(vmax), 1.0));
        }
    }
}
