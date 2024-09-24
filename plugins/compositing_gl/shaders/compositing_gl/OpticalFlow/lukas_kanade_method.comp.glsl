#version 430

layout (local_size_x = 8, local_size_y = 8) in;  // Workgroup size: 16x16 pixels

// Input textures: previous and current frames
layout(binding = 0) uniform sampler2D prev_frame;
layout(binding = 1) uniform sampler2D next_frame;

// Output texture: flow vectors (2D texture storing the u and v flow components)
layout(rgba16f, binding = 0) uniform writeonly image2D flow_texture;

uniform int window_size = 2;
uniform int offset;    

// Calculate the pixel difference (for temporal gradient)
vec4 pixelDifference(ivec2 uv, sampler2D tex1, sampler2D tex2) {
    return texelFetch(tex1, uv, 0) - texelFetch(tex2, uv, 0);
}

// Calculate the central difference for spatial gradients
float gradientX(ivec2 uv, sampler2D tex) {
    return texelFetch(tex, uv + ivec2(offset, 0), 0).r - texelFetch(tex, uv - ivec2(offset, 0), 0).r;
}

float gradientY(ivec2 uv, sampler2D tex) {
    return texelFetch(tex, uv + ivec2(0, offset), 0).r - texelFetch(tex, uv - ivec2(0, offset), 0).r;
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 uv = ivec2(pixel_coords);

    // Initialize matrices for least-squares estimation
    mat2 A = mat2(0.0);  // Matrix A for spatial gradients
    vec2 b = vec2(0.0);  // Vector b for temporal gradients


    for (int i = -window_size; i <= window_size; ++i) {
        for (int j = -window_size; j <= window_size; ++j) {
            ivec2 offset_uv = uv + ivec2(i, j); 

            // Compute spatial gradients (Ix and Iy) using central differences
            float gradX_prev = gradientX(offset_uv, prev_frame);
            float gradX_next = gradientX(offset_uv, next_frame);
            float gradX = gradX_prev + gradX_next;  // Combine gradients from both frames

            float gradY_prev = gradientY(offset_uv, prev_frame);
            float gradY_next = gradientY(offset_uv, next_frame);
            float gradY = gradY_prev + gradY_next;

            // Compute temporal gradient (It) as the difference between the two frames
            float gradT = pixelDifference(offset_uv, next_frame, prev_frame).r;

            // Update A matrix and b vector for least-squares solution
            A += mat2(gradX * gradX, gradX * gradY, gradX * gradY, gradY * gradY);
            b += vec2(gradX * gradT, gradY * gradT);
        }
    }

    vec2 flow = vec2(0.0);

    if (determinant(A) > 0.0001) {  // Ensure A is invertible
        flow = inverse(A) * -b;      // Solve for [u, v]
    }

    // Write the flow vector to the output texture
    imageStore(flow_texture, pixel_coords, vec4(flow, length(flow), 1.0));  // Store as vec4 (u, v, 0, 1)
}
