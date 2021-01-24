#version 330 core

uniform float entityId;

out float outEntityId;

void main()
{
	outEntityId = entityId;
}

