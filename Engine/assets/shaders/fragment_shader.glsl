#version 460 core

out vec4 FragColor;
uniform vec4 ourColor;  // Uniform ╠Да©

void main() {
    FragColor = ourColor;
}