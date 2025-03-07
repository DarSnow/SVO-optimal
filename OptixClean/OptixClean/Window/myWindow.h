#pragma once
#include "../Common/GLheader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;


#include <fstream>
#include <iostream>
#include <queue>
using namespace std;

// GLFW
#include <GLFW/glfw3.h>

#include "../Common/Singleton.h"

namespace WindowCallBack {
	static char msg[128];
	static GLint width, height;
	static GLfloat xpos, ypos;
	void Init(int _width, int _height);
	void error_callback(int error, const char* description);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void cursor_position_callback(GLFWwindow* window, double x, double y);
	void scroll_callback(GLFWwindow* window, double x, double y);

	class MyWindow : public Singleton<MyWindow>{
	public:
		//初始化window，传入宽高窗口名，返回对象指针
		static MyWindow *InitWindow(int width, int height, string name) {
			MyWindow* instance = getInstance();
			instance->SetMyWindow(width, height, name);
			return instance;
		}

		//设置鼠标位置，坐标是到窗口左上的像素数量
		void setCursorPos(double x, double y) {
			glfwSetCursorPos(window, x, y);
		}
		double getTime() {
			return glfwGetTime();
		}
		void close() {
			glfwTerminate();
		}
		//每次渲染完调用
		void flush() {
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		int width, height;

	public:
		class Message {
		public:
			enum Type {
				mousePress, mouseRelese, keybordPress, keybordRelese, window, unknow, mouseMove
			};
			enum MauseEvent {
				left, middle, right
			};
			struct Value {
				Value(){}
				//鼠标事件
				MauseEvent mouse;
				//键盘事件，值借用了glfw预定义的宏，如GLFW_KEY_W、GLFW_KEY_SPACE
				int value;
				vec2 pos;
			};
			Type type;
			Value value;
		};
		//消息队列，从中读取各类消息，如上Message类
		queue<Message> messagePump;
		GLFWwindow* window;
	private:

		void SetMyWindow(int _width = 800, int _height = 800, string name = "default") 
		{
			width = _width;
			height = _height;

			WindowCallBack::Init(width, height);
			glfwSetErrorCallback(WindowCallBack::error_callback);
			if (!glfwInit())
			{
				cout << "faile to init glfw" << endl;
				return;
			}
			window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
			if (window == NULL)
			{
				glfwTerminate();
				cout << "faile to create window" << endl;
				return;
			}
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


			glfwSetKeyCallback(window, WindowCallBack::key_callback);
			glfwSetMouseButtonCallback(window, WindowCallBack::mouse_button_callback);
			glfwSetCursorPosCallback(window, WindowCallBack::cursor_position_callback);
			glfwSetScrollCallback(window, WindowCallBack::scroll_callback);
			glfwMakeContextCurrent(window);
			glewExperimental = true; // Needed for core propbmpfile  
		}


	};
}