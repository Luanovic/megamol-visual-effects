#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D depthBuffer; 
layout(binding = 2) uniform sampler2D neighborMaxBuffer;

uniform int maxBlurRadius;       
uniform int numSamples;          
uniform float exposureTime;      
uniform int frameRate;           

layout(rgba16f, binding = 0) writeonly uniform image2D outputTex;

float softDepthCompare(float depthA, float depthB) {
    const float SOFT_Z_EXTENT = 0.01;
    return clamp(1.0 - abs(depthA - depthB) / SOFT_Z_EXTENT, 0.0, 1.0);
}

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123) - 0.5;
}

// Cone function to calculate contribution based on distance and velocity length
float cone(vec2 X, vec2 Y, vec2 velocity) {
    return clamp(1.0 - length(X - Y) / length(velocity), 0.0, 1.0);
}

// Cylinder function for simultaneous blur between X and Y
float cylinder(vec2 X, vec2 Y, vec2 velocity) {
    return 1.0 - smoothstep(0.95 * length(velocity), 1.05 * length(velocity), length(X - Y));
}

vec4 applyMotionBlur(ivec2 pixelCoords) {

    vec2 velocity = texelFetch(neighborMaxBuffer, pixelCoords, 0).xy;
    vec4 color = texelFetch(colorBuffer, pixelCoords, 0);
    float depth = texelFetch(depthBuffer, pixelCoords, 0).r;

    if (length(velocity) < 0.5) {
        return color;
    }

    vec4 sumColor = vec4(0.0);
    float totalWeight = 0.0;

    // Jitter for anti-ghosting (random jitter to avoid visible artifacts)
    float jitter = random(vec2(pixelCoords));

    for (int i = 0; i < numSamples; ++i) {
        if (i == (numSamples - 1) / 2) continue;

        // Calculate the sample position along the velocity vector
        float t = mix(-1.0, 1.0, (float(i) + jitter + 1.0) / float(numSamples + 1));
        vec2 sampleOffset = floor(velocity * t * exposureTime * float(frameRate) + 0.5);
        ivec2 sampleCoords = pixelCoords + ivec2(sampleOffset);

        if (any(lessThan(sampleCoords, ivec2(0))) || any(greaterThanEqual(sampleCoords, imageSize(outputTex)))) {
            continue; 
        }

        vec4 sampleColor = texelFetch(colorBuffer, sampleCoords, 0);
        float sampleDepth = texelFetch(depthBuffer, sampleCoords, 0).r;

        // Calculate the contribution from the three cases (Case 1, Case 2, Case 3)
        // Case 1: Blurry Y in front of X
        float frontContribution = softDepthCompare(sampleDepth, depth) * cone(vec2(pixelCoords), vec2(sampleCoords), velocity);
        // Case 2: X is blurry, sample Y is behind X (background estimation)
        float backContribution = softDepthCompare(depth, sampleDepth) * cone(vec2(sampleCoords), vec2(pixelCoords), velocity);
        // Case 3: Both X and Y are blurry and within each other's spread
        float simultaneousBlurContribution = cylinder(vec2(pixelCoords), vec2(sampleCoords), velocity) * 2.0;
        // Total alpha contribution (sum of the three cases)
        float alpha_Y = frontContribution + backContribution + simultaneousBlurContribution;

        sumColor += sampleColor * alpha_Y;
        totalWeight += alpha_Y;
    }

    // Normalize the accumulated color by the total weight
    if (totalWeight < 0.0) {
        return sumColor / totalWeight;
    } else {
        return color; 
    }
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 target_res = imageSize(outputTex);

    if(pixel_coords.x > target_res.x || pixel_coords.y > target_res.y){
        return;
    }

    imageStore(outputTex, pixel_coords, applyMotionBlur(pixel_coords));
}
