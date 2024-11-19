#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in; 

uniform sampler2D input_tex;
layout(rgba16) writeonly uniform image2D output_tex;

uniform bool useCalcLuminance;

float calculateBrightness(ivec2 pixel_coords) {
    vec3 color = texelFetch(input_tex, pixel_coords, 0).rgb;
    return 0.2989 * color.r + 0.5870 * color.g + 0.1140 * color.b;
}

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 imageSize = imageSize(output_tex);

    if(useCalcLuminance) {
        imageStore(output_tex, pixel_coords, vec4(vec3(calculateBrightness(pixel_coords)), 1));
    } else {
        imageStore(output_tex, pixel_coords, texelFetch(input_tex, pixel_coords, 0));
    }
}
