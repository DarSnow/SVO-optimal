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

	//����Ӱ��Ŀǰ���ͼ�ı߽磬�޳����� pos4.z С��0�����
	//�������ò�������vs�� gl_Position = pos4 �Ѿ������ֵ����ǰframebuffer�ˣ�gl_Position.z�����ֵ
    if(clip < 0)
        discard;

    float finalDepth = depth.x / depth.y;
	gl_FragDepth = finalDepth;
    //gl_FragColor = mapDepthToARGB32(finalDepth)*20; //vec4(depth.y, depth.y, depth.y, 1.0f);
	//gl_FragColor = vec4(vec3(finalDepth), 1.0);
	//FragColor = vec4(vec3(depthValue), 1.0); // orthographic

}