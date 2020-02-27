#pragma once
#ifndef IMAGETOOL_DEF
#define IMAGETOOL_DEF

#include "GLheader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FreeImage.h>
using namespace glm;
#include <iostream>
#include <fstream>
#include <cstdlib>

class ImageTool {
public:
	static int GetmapPos(BYTE mp[], int width, int height, int posx, int posy) {
		int pos = (posy * width + posx) * 4;
		return pos;
	}

	static BYTE BilinerInterpo(BYTE mp[], int width, int height, int posx, int posy, double ratio, int channel) {
		double rposx = posx / ratio, rposy = posy / ratio;
		int x1 = rposx, x2 = std::min(x1 + 1, width - 1);
		int y1 = rposy, y2 = std::min(y1 + 1, height - 1);
		BYTE fxy1, fxy2;
		BYTE res;
		//interpo x
		if (x1 == x2) {
			fxy1 = mp[GetmapPos(mp, width, height, x1, y1) + channel];
			fxy2 = mp[GetmapPos(mp, width, height, x1, y2) + channel];
		}
		else {
			fxy1 = (x2 - rposx)*mp[GetmapPos(mp, width, height, x1, y1) + channel] +
				(rposx - x1)*mp[GetmapPos(mp, width, height, x2, y1) + channel];
			fxy1 /= (x2 - x1);
			fxy2 = (x2 - rposx)*mp[GetmapPos(mp, width, height, x1, y2) + channel]
				+ (rposx - x1)*mp[GetmapPos(mp, width, height, x2, y2) + channel];
			fxy2 /= (x2 - x1);
		}

		if (y1 == y2) {
			res = fxy1;
		}
		else {
			res = ((y2 - rposy)*fxy1 + (rposy - y1)*fxy2) / (y2 - y1);
		}

		return res;
	}

	static bool SaveResizedImage(char *imgpath, int nwidth, int nheight, float resizeRate) {
		unsigned char *mpixels = new unsigned char[nwidth * nheight * 4];
		glReadBuffer(GL_FRONT);
		//glReadPixels(0, 0, nwidth, nheight, GL_RGBA, GL_UNSIGNED_BYTE, mpixels);
		//glReadBuffer(GL_BACK);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, mpixels);
		for (int i = 0; i<(int)nwidth * nheight * 4; i += 4)
		{
			mpixels[i] ^= mpixels[i + 2];
			mpixels[i + 2] ^= mpixels[i];
			mpixels[i] ^= mpixels[i + 2];
		}
		FIBITMAP* bitmap = FreeImage_Allocate(nwidth * 2, nheight * 2, 32, 8, 8, 8);

		for (int y = 0; y<FreeImage_GetHeight(bitmap); y++)
		{
			BYTE *bits = FreeImage_GetScanLine(bitmap, y);
			for (int x = 0; x<FreeImage_GetWidth(bitmap); x++)
			{
				bits[0] = BilinerInterpo(mpixels, nwidth, nheight, x, y, resizeRate, 0);
				bits[1] = BilinerInterpo(mpixels, nwidth, nheight, x, y, resizeRate, 1);
				bits[2] = BilinerInterpo(mpixels, nwidth, nheight, x, y, resizeRate, 2);				//mpixels[(y * nwidth + x) * 4 + 2];
				bits[3] = 255;
				bits += 4;

			}
		}
		bool bSuccess = FreeImage_Save(FIF_PNG, bitmap, imgpath, PNG_DEFAULT);
		FreeImage_Unload(bitmap);
		delete[] mpixels;
		return bSuccess;
	}
};
#endif