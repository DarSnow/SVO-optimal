#version 430 core

varying vec2 depth;
varying float clip;

void main()
{
	//这里影响目前结果图的边界，剔除掉了 pos4.z 小于0的情况
	//其他不用操作，在vs中 gl_Position = pos4 已经将深度值给当前framebuffer了，gl_Position.z即深度值
    if(clip < 0)
        discard;

    float finalDepth = depth.x / depth.y;
	gl_FragDepth = finalDepth;
}