#version 430 core

varying vec2 depth;
varying float clip;

void main()
{
	//����Ӱ��Ŀǰ���ͼ�ı߽磬�޳����� pos4.z С��0�����
	//�������ò�������vs�� gl_Position = pos4 �Ѿ������ֵ����ǰframebuffer�ˣ�gl_Position.z�����ֵ
    if(clip < 0)
        discard;

    float finalDepth = depth.x / depth.y;
	gl_FragDepth = finalDepth;
}