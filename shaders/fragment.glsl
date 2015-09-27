#version 330

// varying variables
in vec3 vColor;
in vec3 vTexCoord;
in vec3 vNormal;
in vec3 vPosition;

// output color
out vec4 outputF;

uniform int pass;	// 1 -- 1st pass / 2 -- 2nd pass

// uniform variables
uniform int textureEnabled;	// 0 -- color / 1 -- texture
uniform sampler2D tex0;

uniform vec3 lightDir;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform int screenWidth;
uniform int screenHeight;
uniform int lineRendering;	// 1 -- line rendering / 0 -- regular
uniform float depthSensitivity;
uniform float normalSensitivity;
uniform int useThreshold;	// 1 -- use threshold / 0 -- otherwise
uniform float threshold;

void main() {
	if (pass == 1) {
		outputF = vec4((vNormal + 1) * 0.5, 1);
	} else {
		if (lineRendering == 1) {
			float normal_diff = 0;
			float depth_diff = 0;
			int range = 1;

			// difference in normal between this pixel and the neighbor pixels
			vec3 n = texture(normalMap, vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight)).xyz;
			float d = texture(depthMap, vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight)).x;

			for (int xx = -range; xx <= range; ++xx) {
				for (int yy = -range; yy <= range; ++yy) {
					if (xx == 0 && yy == 0) continue;

					vec3 nn = texture(normalMap, vec2((gl_FragCoord.x+xx) / screenWidth, (gl_FragCoord.y+yy) / screenHeight)).xyz;
					if (nn.x == 0 && nn.y == 0 && nn.z == 0) {
						normal_diff = normalSensitivity;
					} else {
						normal_diff = max(normal_diff, length(nn - n) * normalSensitivity);
					}

					float dd = texture(depthMap, vec2((gl_FragCoord.x+xx) / screenWidth, (gl_FragCoord.y+yy) / screenHeight)).x;
					depth_diff = max(depth_diff, abs(dd - d) * depthSensitivity);
				}
			}

			float diff = max(normal_diff, depth_diff);
			diff = min(1.0, diff);
			if (useThreshold == 1) {
				if (diff < threshold) {
					diff = 0.0;
				} else {
					diff = 1.0;
				}
			}
			outputF = vec4(1 - diff, 1 - diff, 1 - diff, 1);
		
			return;
		} else {
			// for color mode
			outputF = vec4(vColor, 1.0);
			//outputF = vec4(texture(normalMap, vec2(gl_FragCoord.x / screenWidth, gl_FragCoord.y / screenHeight)).xyz, 1);

			if (textureEnabled == 1) { // for texture mode
				outputF = texture(tex0, vTexCoord.rg);
			}

			// lighting
			vec4 ambient = vec4(0.3, 0.3, 0.3, 1.0);
			vec4 diffuse = vec4(0.7, 0.7, 0.7, 1.0) * max(0.0, dot(-lightDir, vNormal));

			outputF = (ambient + diffuse) * outputF;
		}
	}
}

