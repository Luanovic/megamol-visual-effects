#version 460

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) uniform sampler2D neighborMaxBuffer; // Low-resolution NeighborMax buffer
layout(rgba16f, binding = 0) writeonly uniform image2D upscaledBuffer; // High-resolution output buffer

uniform int maxBlurRadius; // Size of each tile (e.g., maxBlurRadius * 2 + 1)

ivec2 getTileCoordinates(ivec2 pixel_coords) {
    int tileSize = maxBlurRadius * 2 + 1;
    return pixel_coords / tileSize;
}

void main() {
    // Global ID for the high-resolution buffer
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 highResCoords = ivec2(gID.xy); 
    ivec2 lowRes = textureSize(neighborMaxBuffer, 0);


    // Fetch the neighbor max velocity from the low-res NeighborMaxBuffer
    vec4 neighborMaxVelocity = texelFetch(neighborMaxBuffer, getTileCoordinates(highResCoords), 0);

    // Write the neighbor max velocity to the high-res upscaled buffer at the current pixel
    imageStore(upscaledBuffer, highResCoords, neighborMaxVelocity);
}
