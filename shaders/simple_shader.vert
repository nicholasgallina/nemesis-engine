#version 450

// specification of first vertex attribute
// "in" keyword signifies this variable takes it's value from a VertexBuffer
// layout location seta storage of where value comes from
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

// naming convention: uppercased version of instance
layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main() {
    // push.transform * position != position * push.transform
    // as matrix multiplication is not commutative
    gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
}

// only 1 push constant block can be used for shader entry point