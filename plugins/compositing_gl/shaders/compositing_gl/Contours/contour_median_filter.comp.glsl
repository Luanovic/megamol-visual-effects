#version 460

uniform int radius;
uniform sampler2D contours_tex;

layout(rgba16) writeonly uniform image2D target_tex;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 target_res = imageSize(target_tex);
    ivec2 contours_res = textureSize(contours_tex, 0);

    if (pixel_coords.x >= target_res.x || pixel_coords.y >= target_res.y) {
        return;
    }

    int black_count = 0;
    int white_count = 0;

    vec4 black = vec4(0, 0, 0, 1);
    vec4 white = vec4(1);

    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            ivec2 neighbor_coords = pixel_coords + ivec2(i, j);
            if (neighbor_coords.x >= 0 && neighbor_coords.x < contours_res.x &&
                neighbor_coords.y >= 0 && neighbor_coords.y < contours_res.y) {
                vec4 current_color = texelFetch(contours_tex, neighbor_coords, 0);
                if (current_color.r == 1) { 
                    white_count++;
                } else {
                    black_count++;
                }
            }
        }
    }


    int total_count = black_count + white_count;
    float black_percentage = black_count / total_count;

    if (black_count > white_count) {
        imageStore(target_tex, pixel_coords, black);
    } else {
        imageStore(target_tex, pixel_coords, white);
    }
}
