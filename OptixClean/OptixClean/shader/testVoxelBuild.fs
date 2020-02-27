#version 430

layout(binding = 0, r8) uniform image3D staticVoxelFlag;
in vec3 outposition;

void main(){
	gl_FragColor = vec4(1,1,1,1);
	imageStore(staticVoxelFlag, ivec3((outposition*100).xy,0), vec4(1.0));
}