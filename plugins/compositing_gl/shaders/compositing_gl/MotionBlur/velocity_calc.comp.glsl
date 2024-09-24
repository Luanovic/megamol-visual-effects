#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D flow_tex_2D;

uniform int frameRate;
uniform float exposureTime;
uniform int maxBlurRadius;

layout(rgba16f, binding = 0) writeonly uniform image2D velocityBuffer;

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 target_res = imageSize(velocityBuffer);

    if(pixel_coords.x > target_res.x || pixel_coords.y > target_res.y){
        return;
    }

    vec2 q_x = 0.5 * texelFetch(flow_tex_2D, pixel_coords, 0).xy * frameRate * exposureTime;
    vec2 velocity = (q_x * max(0.5, min(maxBlurRadius, length(q_x)))) / (length(q_x) + 1e-8);

    imageStore(velocityBuffer, pixel_coords, vec4(velocity, length(velocity), 1));
}