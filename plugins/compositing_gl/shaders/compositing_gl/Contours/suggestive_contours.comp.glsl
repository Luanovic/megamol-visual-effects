#version 460

uniform sampler2D color_tex_2D;
uniform sampler2D intensity_tex;

uniform int radius;
uniform float threshold;
uniform bool useMidpointCircle;

layout(rgba16) writeonly uniform image2D target_tex;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;


bool isValley(ivec2 pixel_coords) {

    float p_max = 0;
    float p_i = texelFetch(intensity_tex, pixel_coords, 0).x;
    float s = 1 - 1 / radius;

    int pixel_count = 0;
    int strictly_darker_count = 0;

    for(int i = pixel_coords.x - radius; i <= pixel_coords.x + radius; i++) {
        for(int j = pixel_coords.y - radius; j <= pixel_coords.y + radius; j++) {

            ivec2 current_pos = ivec2(i, j);
            float current_pos_intensity = texelFetch(intensity_tex, current_pos, 0).x;

            bool condition = distance(pixel_coords, current_pos) <= radius;

            if(condition) {
                pixel_count++;

                if(p_max < current_pos_intensity) {
                    p_max = current_pos_intensity;
                }

                if(p_i < current_pos_intensity) {
                    strictly_darker_count++;
                }
            }
        } 
    } 

    if(strictly_darker_count / pixel_count < s && p_max - p_i > threshold * radius) {
        return true;
    } else {
        return false;
    }
}

vec3 spanIntensityCount(ivec2 center, ivec2 offset) {
    float p_max = 0;
    float p_i = texelFetch(intensity_tex, center, 0).x;
    float s = 1 - 1 / radius;

    int pixel_count = 0;
    int strictly_darker_count = 0;

    int startX = center.x - offset.x;
    int endX = center.x + offset.x;

    for (int x = startX; x <= endX; ++x) {
        pixel_count++;

        ivec2 current_coords = ivec2(x, center.y + offset.y);
        float current_pos_intensity = texelFetch(intensity_tex, current_coords, 0).x;

        if(p_max < current_pos_intensity) {
            p_max = current_pos_intensity;
        }

        if(p_i < current_pos_intensity) {
            strictly_darker_count++;
        }
    }

    return vec3(pixel_count, strictly_darker_count, p_max);
}


bool optimizedValleyDetection(ivec2 center) {
    int x = radius;
    int y = 0;

    int p = 1 - radius;
    vec4 color; 

    float s = 1 - 1 / radius;
    vec3 count = vec3(0);
    float pixel_count = 0;
    float p_max = 0;
    float p_i = texelFetch(intensity_tex, center, 0).x;
    float strictly_darker_count = 0;

    while (x > y) {
        vec3 span1 = spanIntensityCount(center, ivec2(x, y));
        vec3 span2 = spanIntensityCount(center, ivec2(y, x));
        vec3 span3 = spanIntensityCount(center, ivec2(y, -x));
        vec3 span4 = spanIntensityCount(center, ivec2(x, -y));
        
        y += 1;
        if (p <= 0) {
            p = p + 2 * y + 1;
        } else {
            x -= 1;
            p = p + 2 * y - 2 * x + 1;
        }

        pixel_count = pixel_count + span1.x + span2.x + span3.x + span4.x;
        strictly_darker_count = strictly_darker_count + span1.y + span2.y + span3.y + span4.y;
        p_max = max(p_max, max(span1.z, max(span2.z, max(span3.z, span4.z))));
    }

    if(strictly_darker_count / pixel_count < s && p_max - p_i > threshold * radius) {
        return true;
    } else {
        return false;
    }
}



void main() {
    uvec3 gID = gl_GlobalInvocationID.xyz;
    ivec2 pixel_coords = ivec2(gID.xy);
    ivec2 target_res = imageSize(target_tex);

    if(pixel_coords.x > target_res.x || pixel_coords.y > target_res.y){
        return;
    }

    vec4 black = vec4(0, 0, 0, 1);
    vec4 white = vec4(1, 1, 1, 1);
    float current_intensity = texelFetch(intensity_tex, pixel_coords, 0).x;

    if (useMidpointCircle) {
        if(optimizedValleyDetection(pixel_coords)) {
            imageStore(target_tex, pixel_coords, black);
        } else {
            imageStore(target_tex, pixel_coords, white);
        }
    }
    else {
        if(isValley(pixel_coords)) { 
            imageStore(target_tex, pixel_coords, black);
        } else {
            imageStore(target_tex, pixel_coords, white);
        }
    }
}

