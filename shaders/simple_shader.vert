#version 450

// specification of first vertex attribute
// "in" keyword signifies this variable takes it's value from a VertexBuffer
// layout location seta storage of where value comes from
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0)  uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec3 directionToLight;
} ubo;


// naming convention: uppercased version of instance
layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

void main() {
    // push.transform * position != position * push.transform
    // as matrix multiplication is not commutative
    //gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
    // bc of homogenous coordinates, don't have to set the offset value
    gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

    // goes from 4x4 model matrix to 3x3 matrix 
    // only upper portion of matrix is needed when transforming normals bc values represent directions, not positions
    // not affected by translations
    // only works if uniform scaling is being performed (sx = sy = sz)

    // calculating the inverse in a shader can be expensive and should be avoided
    // mat3 modelMatrix = transpose(inverse(mat3(push.modelMatrix)));
    // vec3 normalWorldSpace = normalize(mat3(push.modelMatrix) * normal);

    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);   

    fragColor = lightIntensity * color;
}

// only 1 push constant block can be used for shader entry point