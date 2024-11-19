#version 430

layout (local_size_x = 8, local_size_y = 8) in;  // Workgroup size: 16x16 pixels

// Input textures: previous and current frames
layout(binding = 0) uniform sampler2D previous_frame;
layout(binding = 1) uniform sampler2D current_frame;

// Output texture: flow vectors (2D texture storing the u and v flow components)
layout(rgba16f, binding = 0) uniform writeonly image2D flow_texture;

uniform int windowSize;
uniform float threshold;    

mat3 sobel_kernel_y = mat3( 
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
);

mat3 sobel_kernel_x = mat3(
    -1, -2, -1,
    0,  0,  0,
    1,  2,  1
);

float applyFilter(mat3 kernel, ivec2 pos, sampler2D tex) {
    float gradient = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {

            ivec2 neighbor_pos = pos + ivec2(i, j);
                vec4 neighbor_color = texelFetch(tex, neighbor_pos, 0); 
                gradient += neighbor_color.x * kernel[i+1][j+1];
        }
    }
    return gradient;
}

// GLSL function to compute the eigenvalues of a 2x2 matrix
vec2 computeEigenvalues(mat2 A) {
    // Extract matrix components
    float a = A[0][0];
    float b = A[0][1];
    float c = A[1][0];
    float d = A[1][1];
    
    // Calculate the trace and determinant
    float trace = a + d;
    float determinant = a * d - b * c; 
    
    // Calculate the discriminant (sqrt of the characteristic equation)
    float discriminant = sqrt(max(trace * trace - 4.0 * determinant, 0.0));

    
    // Compute the two eigenvalues
    float lambda1 = 0.5 * (trace + discriminant);
    float lambda2 = 0.5 * (trace - discriminant);
    
    return vec2(lambda1, lambda2);
}

vec4 pixelDifference(ivec2 uv, sampler2D current_frame, sampler2D previous_frame) {
    return texelFetch(current_frame, uv, 0) - texelFetch(previous_frame, uv, 0);
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 uv = ivec2(pixel_coords);

    // Initialize matrices for least-squares estimation
    mat2 ATA = mat2(0.0);  // Matrix A^T A
    vec2 ATb = vec2(0.0);  // Vector A^T b


    for (int i = -windowSize; i <= windowSize; ++i) {
        for (int j = -windowSize; j <= windowSize; ++j) {
            ivec2 offset_uv = uv + ivec2(i, j); 

        float gradX = applyFilter(sobel_kernel_x, offset_uv, previous_frame);
        float gradY = applyFilter(sobel_kernel_y, offset_uv, previous_frame);
        float gradT = pixelDifference(offset_uv, current_frame, previous_frame).r;

            // Update ATA (A^T A) and ATb (A^T b) matrices for least-squares solution
            ATA += mat2(gradX * gradX, gradX * gradY, gradX * gradY, gradY * gradY);  // Update A^T A

            ATb += vec2(gradX * gradT, gradY * gradT);   
        }
    }

    vec2 flow = vec2(0.0);

    vec2 eigenValues = computeEigenvalues(ATA);
    float lambda1 = eigenValues.x;
    float lambda2 = eigenValues.y;

    bool isInvertible = lambda1 >= lambda2 && lambda2 >= 0;
    bool isGoodFeature = min(lambda1, lambda2) > threshold;


    if(isGoodFeature && isInvertible) {
        flow = inverse(ATA) * -ATb;
    }

    if(flow.x != 0 || flow.y != 0) {
        imageStore(flow_texture, pixel_coords, vec4(flow, length(flow), 1.0)); 
    } else {
        imageStore(flow_texture, pixel_coords, vec4(0, 0, 0, 0));
    }
}
