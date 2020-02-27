#pragma once
#include "Common\GLheader.h"
#include <iostream>
using namespace std;
class AtomicBuffer
{
public:
	GLuint bufId;
	AtomicBuffer() :bufId(0) {};
	AtomicBuffer(int num)
	{
		while (glGetError());
		//GLuint initVal = 0;
		glGenBuffers(1, &bufId);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, bufId);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_STATIC_DRAW);
		ResetCounter(0);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, bufId);
		GLenum err = glGetError();
		if (err > 0)
			cout << "Error AtomicBuffer():" << glewGetErrorString(err) << " " << err << endl;
	}
	GLuint GetCounter()
	{
		while (glGetError());
		GLuint *counts;
		GLuint count;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, bufId);
		counts = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
		count = counts[0];
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

		//glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &count);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		GLenum err = glGetError();
		if (err > 0)
			cout << "Error AtomicBuffer.GetCounter():" << glewGetErrorString(err) << " " << err << endl;
		return count;
	}
	void ResetCounter(GLuint reset)
	{
		GLuint *counts;
		while (glGetError());
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, bufId);
		counts = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
		counts[0] = reset;
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

		//glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &reset); //reset counter to zero
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		GLenum err = glGetError();
		if (err > 0)
			cout << "Error AtomicBuffer.ResetCounter():" << glewGetErrorString(err) << " " << err << endl;
	}
	~AtomicBuffer() { glDeleteBuffers(1, &bufId); };
};