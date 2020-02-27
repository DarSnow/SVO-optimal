#pragma once

#include "../Common/GLheader.h"
#include <GLFW\glfw3.h>

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>

#include <sutil.h>
#include "commonStructs.h"
#include <Arcball.h>
#include <OptiXMesh.h>
#include "MeshBuilder.h"
#include "../ModelClass/Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include "AccelsLayout.h"
#include "../Scene/scene.h"
#include "../VPLManage/VPL.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdint.h>

using namespace optix;
using namespace ModelClass;

struct UsageReportLogger
{
	void log(int lvl, const char* tag, const char* msg)
	{
		std::cout << "[" << lvl << "][" << std::left << std::setw(12) << tag << "] " << msg;
	}
};


class OptixSampler {
public:
	float samplePosArr[5000];
	float sampleNorArr[5000];
	float sampleClrArr[5000];
	uint32_t	  sampleIdx = 0u;
	uint32_t	  samplenum = 100u;
	static const int MAXVPLNUM = 1000;
	const float3 light_em = make_float3(15.0f, 15.0f, 5.0f);

	//Shader state
	unsigned int quadVAO;
	unsigned int sampleVAO, sampleVBO;
	Shader *pointShader, *tex2ScrShader;

	//Scene state
	DynamicLayout* layout;
	Scene *scene;
	//------------------------------------------------------------------------------
	//
	// Forward decls 
	//
	//------------------------------------------------------------------------------

	~OptixSampler() {
		if (context)
		{
			context->destroy();
			context = 0;
		}
	}

	Buffer getOutputBuffer();
	void registerExitHandler();
	void createContext(int usage_report_level, UsageReportLogger* logger);
	void setupShader();
	void setupAllEntrysContext();

	void createScene(Context ctx, DynamicLayout *layout);
	void loadMesh(const std::string& filename);
	void getPNCBufferToArr();
	void getBufferToArr(string buffer_name, float* arr, unsigned length);
	void setupAllVAO();
	void drawPoints();
	
	void displayBufferToScr(RTbuffer buffer, bufferPixelFormat format);
	void getBufferContent(optix::Buffer buffer, bool isSample, bufferPixelFormat format = bufferPixelFormat::BUFFER_PIXEL_FORMAT_DEFAULT);
	
	void setupCamera();
	void setupLights();
	GeometryInstance createParallelogram(const float3& anchor,
		const float3& offset1,
		const float3& offset2);
	void updateCamera();

	void update();
	void setupVPLarr(unsigned vplnum, VPL* vplarr);
	void estimateRadiosity(float* outrad);
	void estimatePdf(float* outpdf);
	void draw(unsigned entrypoint);


	void glutKeyboardPress(unsigned char k, int x, int y);
	void glutMousePress(int button, int state, int x, int y);
	void glutMouseMotion(int x, int y);
	void glutResize(int w, int h);

	GLenum glFormatFromBufferFormat(bufferPixelFormat pixel_format, RTformat buffer_format);

	void printUsageAndExit(const std::string& argv0);
	void Initialize(Scene* scn);


private:
	const char* const SAMPLE_NAME = "optixMeshViewer";

	//------------------------------------------------------------------------------
	//
	// Globals
	//
	//------------------------------------------------------------------------------

	Context        context;
	uint32_t       width = 1024u;
	uint32_t       height = 768u;
	bool           use_pbo = true;

	// Camera state
	float vfov = 35.0f;
	float3         camera_up;
	float3         camera_lookat;
	float3         camera_eye;
	Matrix4x4      camera_rotate;
	sutil::Arcball arcball;

	// Mouse state
	int2           mouse_prev_pos;
	int            mouse_button;

	//Buffer state
	bool g_disable_srgb_conversion;

	//About sample point part
	Program ray_gen_program;
	Program sample_gen_program;
	Program miss_program;
	Program sample_miss_program;
	Program exception_program;
	Program sample_exception_program;
	Program samplelight_gen_program;
	Program sampledirect_light_gen_program;

	Program pathtrace_ray_gen_program;
	Program pathtrace_miss_program;
	Program pathtrace_exception_program;

	Program ptpdf_ray_gen_program;
	Program ptpdf_exception_program;

	// aera light material
	Program pgram_bounding_box;
	Program pgram_intersection;

	Buffer vpl_buffer;
	int vpl_count;
};

extern void glutKeyboardPress(unsigned char k, int x, int y);
extern void glutMousePress(int button, int state, int x, int y);
extern void glutMouseMotion(int x, int y);
extern void glutResize(int w, int h);

inline static float3 vec3tofloat3(glm::vec3 ele) {
	return make_float3(ele.x, ele.y, ele.z);
}