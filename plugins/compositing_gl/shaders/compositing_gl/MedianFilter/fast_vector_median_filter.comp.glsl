#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// Input textures
uniform sampler2D color_tex_2D;

// Output texture
layout(rgba16) writeonly uniform image2D output_tex;

// Uniforms
uniform float beta; 
uniform int windowSize;
uniform int mode;

float l1Distance(vec3 a, vec3 b) {
    return abs(a.r - b.r) + abs(a.g - b.g) + abs(a.b - b.b);
}

float euclideanDistance(vec3 a, vec3 b) {
    return length(a - b);
}

float calcDistanceSum(ivec2 center_pixel, ivec2 current_pixel_coords, bool ignore_center_pixel, int windowSize) {

    float sum_of_distances = 0.0;
    vec4 current_color = texelFetch(color_tex_2D, current_pixel_coords, 0);

    for (int x = -windowSize; x <= windowSize; x++) {
        for (int y = -windowSize; y <= windowSize; y++) {
            ivec2 coords_i = center_pixel + ivec2(x, y);

            if(coords_i == current_pixel_coords) continue;
            if(ignore_center_pixel && coords_i == center_pixel) continue;

            vec4 color_i = texelFetch(color_tex_2D, coords_i, 0);
            if(mode == 0) {
                sum_of_distances += l1Distance(current_color.xyz, color_i.xyz);
            } else {
                sum_of_distances += euclideanDistance(current_color.xyz, color_i.xyz);
            }
        }
    }
    return sum_of_distances;
}

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 imageSize = imageSize(output_tex);

    if(pixel_coords.x >= imageSize.x || pixel_coords.y >= imageSize.y) {
        return;  // Skip out-of-bounds pixels
    }

    float min_distance = 1.0 / 0.0;  // Initialize to a large value
    vec4 min_distance_color = vec4(0.0);
    bool exclude_center_pixel = false;

    // Calculate the reference distance sum R0 for the center pixel
    float R0 = calcDistanceSum(pixel_coords, pixel_coords, false, windowSize) - beta;

    // Loop over the window to compute distances
    for (int x = -windowSize; x <= windowSize; x++) {
        for (int y = -windowSize; y <= windowSize; y++) {

            ivec2 current_coords = pixel_coords + ivec2(x, y);

            vec4 current_color = texelFetch(color_tex_2D, current_coords, 0);
            float distance_sum = calcDistanceSum(pixel_coords, current_coords, exclude_center_pixel, windowSize);

            // Recalculate with exclusion if necessary
            if (distance_sum < R0) {
                exclude_center_pixel = true;
                distance_sum = calcDistanceSum(pixel_coords, current_coords, exclude_center_pixel, windowSize);
            }

            if (distance_sum < min_distance) {
                min_distance = distance_sum;
                min_distance_color = current_color;
            }
        }
    }

    if(!exclude_center_pixel) {
        imageStore(output_tex, pixel_coords, texelFetch(color_tex_2D, pixel_coords, 0));
    } else {
        imageStore(output_tex, pixel_coords, vec4(min_distance_color.xyz, 1.0));  
    }
}
