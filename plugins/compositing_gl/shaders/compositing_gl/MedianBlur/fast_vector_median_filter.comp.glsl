#version 460

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in; 

uniform sampler2D color_tex_2D; 
layout(rgba16) writeonly uniform image2D output_tex;

uniform float beta; 
uniform int windowSize;

// Helper function to compute L1 distance between two color vectors
float colorDistance(vec3 a, vec3 b) {
    return abs(a.r - b.r) + abs(a.g - b.g) + abs(a.b - b.b);
}

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 imageSize = imageSize(output_tex);

    if(pixel_coords.x > imageSize.x || pixel_coords.y > imageSize.y){
        return;
    }

    int windowSize = 3;
    const int halfWindow = windowSize / 2;

    float distances[9];

    // Loop over the window to compute distances
    for (int wy = -halfWindow; wy <= halfWindow; wy++) {
        for (int wx = -halfWindow; wx <= halfWindow; wx++) {
            int index = (wy + halfWindow) * windowSize + (wx + halfWindow);
            distances[index] = 0.0;

            // Position of the current neighbor
            ivec2 neighborPos = pixel_coords + ivec2(wx, wy);
            
            // Ensure neighbor is within the image bounds
            if (neighborPos.x >= 0 && neighborPos.x < imageSize.x && neighborPos.y >= 0 && neighborPos.y < imageSize.y) {
                vec3 currentColor = texelFetch(color_tex_2D, neighborPos, 0).rgb;

                // Calculate the sum of distances to all other pixels in the window
                for (int ny = -halfWindow; ny <= halfWindow; ny++) {
                    for (int nx = -halfWindow; nx <= halfWindow; nx++) {
                        ivec2 otherNeighborPos = pixel_coords + ivec2(nx, ny);
                        
                        // Ensure the other neighbor is within the image bounds
                        if (otherNeighborPos.x >= 0 && otherNeighborPos.x < imageSize.x && otherNeighborPos.y >= 0 && otherNeighborPos.y < imageSize.y) {
                            vec3 otherColor = texelFetch(color_tex_2D, otherNeighborPos, 0).rgb;
                            distances[index] += colorDistance(currentColor, otherColor);
                        }
                    }
                }
            } else {
                distances[index] = 1e10; // Large value for out-of-bounds pixels
            }
        }
    }

    // Find the pixel with the minimum distance sum
    float minDistance = distances[0];
    vec3 medianPixel = texelFetch(color_tex_2D, pixel_coords, 0).rgb; // Default to the center pixel

    for (int i = 1; i < windowSize * windowSize; i++) {
        if (distances[i] < minDistance) {
            minDistance = distances[i];
            ivec2 offset = ivec2((i % windowSize) - halfWindow, (i / windowSize) - halfWindow);
            ivec2 bestPos = pixel_coords + offset;
            if (bestPos.x >= 0 && bestPos.x < imageSize.x && bestPos.y >= 0 && bestPos.y < imageSize.y) {
                medianPixel = texelFetch(color_tex_2D, bestPos, 0).rgb;
            }
        }
    }


    // Store the best color in the output image
    imageStore(output_tex, pixel_coords, vec4(medianPixel, 1.0));
}
