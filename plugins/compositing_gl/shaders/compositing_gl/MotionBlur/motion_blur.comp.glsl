#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D depthBuffer; 
layout(binding = 2) uniform sampler2D neighborMaxBuffer;

uniform int numSamples;          

layout(rgba16f, binding = 0) writeonly uniform image2D outputTex;

float softDepthCompare(float depthA, float depthB) {
    const float SOFT_Z_EXTENT = 0.1;
    return clamp(1.0 - (depthA - depthB) / SOFT_Z_EXTENT, 0.0, 1.0);
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

vec3 applyMotionBlur(ivec2 pixelCoords) {

    vec2 velocity = texelFetch(neighborMaxBuffer, pixelCoords, 0).xy;
    vec3 color = texelFetch(colorBuffer, pixelCoords, 0).xyz;
    float depth = texelFetch(depthBuffer, pixelCoords, 0).r;

    if (length(velocity) < 0.5) {
        return color;
    }

    float weight = 1 / length(velocity);
    vec3 sum = color * weight;

    // Jitter for anti-ghosting (random jitter to avoid visible artifacts)
    float jitter = random(vec2(pixelCoords));

    for (int i = 0; i < numSamples; ++i) {
        if (i == (numSamples - 1) / 2) continue;

        // Calculate the sample position along the velocity vector
        float t = mix(-1.0, 1.0, (float(i) + jitter + 1.0) / float(numSamples + 1));
        ivec2 sampleCoords = ivec2(pixelCoords + velocity * t + vec2(0.5));

        float sampleDepth = texelFetch(depthBuffer, sampleCoords, 0).x;
        vec3 sampleColor = texelFetch(colorBuffer, sampleCoords, 0).xyz;
        vec2 sampleVelocity = texelFetch(neighborMaxBuffer, sampleCoords, 0).xy;


        float frontContribution = softDepthCompare(depth, sampleDepth);
        float backContribution = softDepthCompare(sampleDepth, depth);

        float alpha_y = frontContribution * cone(sampleCoords, pixelCoords, sampleVelocity) 
                        + backContribution * cone(pixelCoords, sampleCoords, velocity) 
                        + cylinder(sampleCoords, pixelCoords, sampleVelocity) * cylinder(pixelCoords, sampleCoords, velocity) * 2.0;


        sum += sampleColor * alpha_y;
        weight += alpha_y;
    }
    if(weight > 0) {
        return sum / weight;
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

    vec4 color = texelFetch(colorBuffer, pixel_coords, 0);
    float depth = texelFetch(depthBuffer, pixel_coords, 0).r;
    if(depth < 1) {
        imageStore(outputTex, pixel_coords, vec4(applyMotionBlur(pixel_coords), 1));
    } else {
        imageStore(outputTex, pixel_coords, color);
    }
}
