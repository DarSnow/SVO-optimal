#pragma once
#include <iostream>
#ifndef  SINGLETON
#define  SINGLETON

template<class T>
class Singleton
{	
public:
	static T* getInstance(void);
	Singleton(void);
	virtual ~Singleton(void);

private:
	static T* _instance;
};

template<class T>
T*  Singleton<T>::_instance = NULL;


template<class T>
Singleton<T>::Singleton(void) { }

template<class T>
T*  Singleton<T>::getInstance(void)
{
	if (_instance == NULL)
	{
		if (_instance == NULL)
		{
			 _instance = new T;
		}
	}
	return _instance;
}

template<class T>
Singleton<T>::~Singleton(void)
{
	if (_instance)
	{
		delete _instance;
	}
}
#endif