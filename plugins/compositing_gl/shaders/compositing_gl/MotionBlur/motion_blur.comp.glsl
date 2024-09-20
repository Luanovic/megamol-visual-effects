
#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D depthBuffer; 
layout(binding = 2) uniform sampler2D velocityBuffer; 
layout(binding = 3) uniform sampler2D neighborMaxBuffer;

uniform int maxBlurRadius;       
uniform int numSamples;          
uniform float exposureTime;      
uniform int frameRate;           

layout(rgba16f, binding = 0) writeonly uniform image2D outputTex;

ivec2 getTileCoordinates(ivec2 pixel_coords) {
    int tileSize = maxBlurRadius * 2 + 1;
    return pixel_coords / tileSize;
}

float softDepthCompare(float depthA, float depthB) {
    const float SOFT_Z_EXTENT = 0.01;
    return clamp(1.0 - (depthA - depthB) / SOFT_Z_EXTENT, 0.0, 1.0);
}

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}


// Function to perform motion blur on a given pixel
vec4 applyMotionBlur(ivec2 pixelCoords) {
    ivec2 tile_coords = getTileCoordinates(pixelCoords);

    vec2 velocity = texelFetch(neighborMaxBuffer, tile_coords, 0).xy;
    vec4 color = texelFetch(colorBuffer, pixelCoords, 0);
    vec4 depth = texelFetch(depthBuffer, pixelCoords, 0);


    // Early exit if the velocity is close to zero (no blur needed)
    if (length(velocity) < 0.5) {
        return color;
    }

    // Get the depth of the current pixel
    // Initialize the sum and weight for color accumulation
    vec4 sumColor = vec4(0.0);
    float totalWeight = 0.0;

    // Jitter for anti-ghosting (random jitter to avoid visible artifacts)
    float jitter = random(pixelCoords) - 0.5;

    // Iterate over numSamples along the velocity vector (S samples)
    for (int i = 0; i < numSamples; ++i) {
        // Calculate the sample position along the velocity vector
        float t = mix(-1.0, 1.0, (float(i) + jitter + 1.0) / float(numSamples + 1));
        vec2 sampleOffset = velocity * t * exposureTime * frameRate;
        ivec2 sampleCoords = pixelCoords + ivec2(sampleOffset);

        // Get the depth of the sample pixel
        float depthY = texture(depthBuffer, sampleCoords).r;

        // Perform soft depth comparison to get foreground/background information
        float frontContribution = softDepthCompare(depth.x, depth.y);  
        float backContribution = softDepthCompare(depth.y, depth.x);   

        // Fetch the color of the sample pixel
        vec4 sampleColor = texture(colorBuffer, sampleCoords);

        // Calculate the contribution of the sample based on depth and distance
        float sampleWeight = frontContribution + backContribution;
        
        // Accumulate the sample color and weight
        sumColor += sampleColor * sampleWeight;
        totalWeight += sampleWeight;
    }

    // Normalize the accumulated color by the total weight
    vec4 finalColor = sumColor / totalWeight;

    return finalColor;
}

void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 target_res = imageSize(outputTex);
    imageStore(outputTex, pixel_coords, applyMotionBlur(pixel_coords));
}