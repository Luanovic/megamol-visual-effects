#version 460

// Define local work group size
layout(local_size_x = 8, local_size_y = 8) in;

// Binding points for input textures
layout(binding = 0) uniform sampler2D I0;
layout(binding = 1) uniform sampler2D I1;

uniform float offset;
uniform float lambda;

// Binding point for output texture
layout(rgba16f, binding = 0) writeonly uniform image2D flowOutput;
layout(rgba16f, binding = 1) writeonly uniform image2D velocityOutput;

// Compute the optical flow at each pixel
void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 target_res = imageSize(flowOutput);

    if(pixel_coords.x > target_res.x || pixel_coords.y > target_res.y){
        return;
    }

    ivec2 offsetX = ivec2(offset, 0);
    ivec2 offsetY = ivec2(0 , offset);
    vec4 currentInput = texelFetch(I1, pixel_coords, 0);
    vec4 prevInput = texelFetch(I0, pixel_coords, 0); 

    // Calculate image gradients using texelFetch
    vec4 gradX = (texelFetch(I1, pixel_coords + offsetX, 0) - texelFetch(I1, pixel_coords - offsetX, 0)) +
                (texelFetch(I0, pixel_coords + offsetX, 0) - texelFetch(I0, pixel_coords - offsetX, 0));

    vec4 gradY = (texelFetch(I1, pixel_coords + offsetY, 0) - texelFetch(I1, pixel_coords - offsetY, 0)) +
                (texelFetch(I0, pixel_coords + offsetY, 0) - texelFetch(I0, pixel_coords - offsetY, 0));

    // Compute gradient magnitude with regularization term
    vec4 gradMag = sqrt((gradX * gradX) + (gradY * gradY) + vec4(lambda));

    // Compute the difference between the next and past frames using texelFetch
    vec4 diff = currentInput - prevInput;

    // Calculate the optical flow vector components
    vec2 flow = vec2((diff * (gradX / gradMag)).x, (diff * (gradY / gradMag)).x);


    if(flow.x != 0 || flow.y != 0) {
        imageStore(flowOutput, pixel_coords, vec4(flow, 0.0, 1.0)); 
    } else {
        imageStore(flowOutput, pixel_coords, vec4(0, 0, 0, 0));
    }
}
