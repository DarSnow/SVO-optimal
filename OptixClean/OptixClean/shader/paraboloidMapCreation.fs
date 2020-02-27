#version 430 core

varying vec2 depth;
varying float clip;

vec4 mapDepthToARGB32(float value)
{
    const vec4 bitSh = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    const vec4 mask = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    highp vec4 res = fract(value * bitSh);
    res -= res.xxyz * mask;
    return res;
}


void main()
{

	//这里影响目前结果图的边界，剔除掉了 pos4.z 小于0的情况
	//其他不用操作，在vs中 gl_Position = pos4 已经将深度值给当前framebuffer了，gl_Position.z即深度值
    if(clip < 0)
        discard;

    float finalDepth = depth.x / depth.y;
	gl_FragDepth = finalDepth;
    //gl_FragColor = mapDepthToARGB32(finalDepth)*20; //vec4(depth.y, depth.y, depth.y, 1.0f);
	//gl_FragColor = vec4(vec3(finalDepth), 1.0);
	//FragColor = vec4(vec3(depthValue), 1.0); // orthographic

}