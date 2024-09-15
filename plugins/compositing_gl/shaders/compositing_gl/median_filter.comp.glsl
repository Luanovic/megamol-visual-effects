#version 460

// Input and output image textures
layout(binding = 0, rgba32f) uniform readonly image2D inputImage;
layout(binding = 1, rgba32f) uniform writeonly image2D outputImage;

// Filter parameters
uniform int windowSize;  // Size of the filtering window (e.g., 3 for 3x3 window)
uniform float beta;  // Threshold parameter

// Helper function to calculate the Euclidean distance (L2 norm) between two color vectors
float euclideanDistance(vec3 a, vec3 b) {
    return length(a - b);  // Equivalent to sqrt((a.x - b.x)^2 + (a.y - b.y)^2 + (a.z - b.z)^2)
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy); // Get the coordinates of the current pixel
    ivec2 imageSize = imageSize(inputImage); // Get the size of the input image

    // Initialize the best pixel to be the center pixel itself
    vec3 centerPixel = imageLoad(inputImage, pixelCoord).rgb;
    float R0 = -beta;  // Initialize R0 with the negative threshold beta

    // Compute the sum of distances for the center pixel
    for (int i = -windowSize / 2; i <= windowSize / 2; ++i) {
        for (int j = -windowSize / 2; j <= windowSize / 2; ++j) {
            ivec2 offsetCoord = pixelCoord + ivec2(i, j);
            if (offsetCoord.x >= 0 && offsetCoord.x < imageSize.x &&
                offsetCoord.y >= 0 && offsetCoord.y < imageSize.y) {
                vec3 neighborPixel = imageLoad(inputImage, offsetCoord).rgb;
                R0 += euclideanDistance(centerPixel, neighborPixel);
            }
        }
    }

    float minR = R0;  // Initialize minimum distance sum
    vec3 bestPixel = centerPixel;  // Start with the assumption that the center pixel is the best

    // Iterate over each pixel in the window to find the pixel with minimum distance sum
    for (int i = -windowSize / 2; i <= windowSize / 2; ++i) {
        for (int j = -windowSize / 2; j <= windowSize / 2; ++j) {
            ivec2 neighborCoord = pixelCoord + ivec2(i, j);

            if (neighborCoord.x >= 0 && neighborCoord.x < imageSize.x &&
                neighborCoord.y >= 0 && neighborCoord.y < imageSize.y) {
                vec3 neighborPixel = imageLoad(inputImage, neighborCoord).rgb;
                float Ri = 0.0;

                // Compute the sum of distances for the current neighbor pixel
                for (int m = -windowSize / 2; m <= windowSize / 2; ++m) {
                    for (int n = -windowSize / 2; n <= windowSize / 2; ++n) {
                        ivec2 innerCoord = pixelCoord + ivec2(m, n);
                        if (innerCoord.x >= 0 && innerCoord.x < imageSize.x &&
                            innerCoord.y >= 0 && innerCoord.y < imageSize.y) {
                            vec3 innerPixel = imageLoad(inputImage, innerCoord).rgb;
                            Ri += euclideanDistance(neighborPixel, innerPixel);
                        }
                    }
                }

                // Check if the current neighbor pixel has a smaller distance sum
                if (Ri < minR) {
                    minR = Ri;
                    bestPixel = neighborPixel;
                }
            }
        }
    }

    // Write the best pixel value to the output image
    imageStore(outputImage, pixelCoord, vec4(bestPixel, 1.0));
}
