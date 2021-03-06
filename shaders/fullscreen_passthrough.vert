#version 450 core

layout (location = 0) in vec2 vertPos;
out vec2 fragTexCoord;

void main() {
	gl_Position = vec4(vertPos, 0, 1);
	fragTexCoord = vertPos * 0.5 + 0.5;
}
