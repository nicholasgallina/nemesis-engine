#version 450

// specification of first vertex attribute
// "in" keyword signifies this variable takes it's value from a VertexBuffer
// layout location seta storage of where value comes from
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;


// naming convention: uppercased version of instance
layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

void main() {
    // push.transform * position != position * push.transform
    // as matrix multiplication is not commutative
    //gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
    // bc of homogenous coordinates, don't have to set the offset value
    gl_Position = push.transform * vec4(position, 1.0);
    fragColor = color;
}

// only 1 push constant block can be used for shader entry point