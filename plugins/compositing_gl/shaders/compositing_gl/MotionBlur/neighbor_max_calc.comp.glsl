#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D tileMaxBuffer;

uniform int maxBlurRadius;

layout(rgba16f, binding = 0) writeonly uniform image2D neighborMaxBuffer;

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 target_res = imageSize(neighborMaxBuffer);

    if(pixel_coords.x > target_res.x || pixel_coords.y > target_res.y){
        return;
    }

    vec2 vmax = vec2(0);
    for (int i = -maxBlurRadius; i <= maxBlurRadius; i++) {
        for (int j = -maxBlurRadius; j <= maxBlurRadius; j++) {
            ivec2 current_pixel = pixel_coords + ivec2(i, j);
            vec2 v_current = texelFetch(tileMaxBuffer, current_pixel, 0).xy;
            vmax = max(v_current, vmax);
        }
    }
    imageStore(neighborMaxBuffer, pixel_coords, vec4(vmax, 0, 1));
}