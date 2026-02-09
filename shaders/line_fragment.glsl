#version 330 core

in vec3 Color;
flat in int vSegmentID;

uniform int selectedSegmentID;
uniform float alpha;

out vec4 FragColor;

void main()
{
    vec4 outColor = vec4(Color, alpha);
    if (selectedSegmentID != -1 && vSegmentID == selectedSegmentID) {
        outColor = vec4(1.0, 1.0, 0.0, alpha); // Highlight: Yellow
    }
    FragColor = outColor;
}
