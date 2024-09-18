#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in; 

uniform sampler2D input_tex;
layout(rgba16) writeonly uniform image2D output_tex;

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 imageSize = imageSize(output_tex);

    imageStore(output_tex, pixel_coords, texelFetch(input_tex, pixel_coords, 0));
}
