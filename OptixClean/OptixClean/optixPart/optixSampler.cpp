#include "optixSampler.h"

OptixSampler optixSamplr;


Buffer OptixSampler::getOutputBuffer()
{
	return context["output_buffer"]->getBuffer();
}

#pragma region CallbackSetup

void destroyContext()
{

}
// Static callback
void usageReportCallback(int lvl, const char* tag, const char* msg, void* cbdata)
{
	// Route messages to a C++ object (the "logger"), as a real app might do.
	// We could have printed them directly in this simple case.

	UsageReportLogger* logger = reinterpret_cast<UsageReportLogger*>(cbdata);
	logger->log(lvl, tag, msg);
}

void OptixSampler::registerExitHandler()
{
	// register shutdown handler
#ifdef _WIN32

#else
	atexit(destroyContext);
#endif
}
#pragma endregion

void OptixSampler::setupShader() {
	pointShader = new Shader("optixPart/PointDraw.vs", "optixPart/PointDraw.fs");
	tex2ScrShader = new Shader("optixPart/Tex2Scr.vs", "optixPart/Tex2Scr.fs");
	tex2ScrShader->use();
	tex2ScrShader->setInt("scrTexture", 0);
	glUseProgram(0);
	pointShader->GetProgram();
}

void OptixSampler::createContext(int usage_report_level, UsageReportLogger* logger)
{
	// Set up context
	context = Context::create();
	context->setRayTypeCount(6);
	
	if (usage_report_level > 0)
	{
		context->setUsageReportCallback(usageReportCallback, usage_report_level, logger);
	}

	context["color_ray_type"]->setUint(color_ray_type);
	context["color_shadow_ray_type"]->setUint(color_shadow_ray_type);
	context["sample_ray_type"]->setUint(sample_ray_type);
	context["sampleDirectlight_ray_type"]->setUint(sampleDirectlight_ray_type);

	context["pathtrace_ray_type"]->setUint(pathtrace_ray_type);
	context["pathtrace_shadow_ray_type"]->setUint(pathtrace_shadow_ray_type);
	context["scene_epsilon"]->setFloat(1.e-3f);
	context["rr_begin_depth"]->setUint(1u);
	context["samples_per_pixel"]->setUint(16u);
	context["max_bounces"]->setUint(3u);
	context["sensor_size"]->setUint(0u);

	Buffer buffer = sutil::createOutputBuffer(context, RT_FORMAT_UNSIGNED_BYTE4, width, height, use_pbo);
	context["output_buffer"]->set(buffer);

	Buffer posbuffer = OptixMeshSpace::createArrBuffer(context, RT_FORMAT_FLOAT4, samplenum, use_pbo, RTbuffertype::RT_BUFFER_OUTPUT);
	context["sample_pos_buffer"]->set(posbuffer);

	Buffer norbuffer = OptixMeshSpace::createArrBuffer(context, RT_FORMAT_FLOAT4, samplenum, use_pbo, RT_BUFFER_OUTPUT);
	context["sample_nor_buffer"]->set(norbuffer);

	Buffer clrbuffer = OptixMeshSpace::createArrBuffer(context, RT_FORMAT_FLOAT4, samplenum, use_pbo, RT_BUFFER_OUTPUT);
	context["sample_clr_buffer"]->set(clrbuffer);

	// store the pathtrace radiosities
	Buffer radiosity_buffer = OptixMeshSpace::createArrBuffer(context, RT_FORMAT_FLOAT4, MAXVPLNUM, use_pbo, RT_BUFFER_OUTPUT);
	context["radiosity_buffer"]->set(radiosity_buffer);

	// vpl buffer self defined
	vpl_buffer = context->createBuffer(RT_BUFFER_INPUT_OUTPUT);
	vpl_buffer->setFormat(RT_FORMAT_USER);
	vpl_buffer->setElementSize(sizeof(opVPL));
	vpl_buffer->setSize(MAXVPLNUM);
	context["vpl_buffer"]->set(vpl_buffer);

	// Ray generation program
	ray_gen_program = context->createProgramFromPTXFile("x64/Debug/mypinhole_camera.cu.ptx", "pinhole_camera");

	// Exception program
	exception_program = context->createProgramFromPTXFile("x64/Debug/mypinhole_camera.cu.ptx", "exception");
	context["bad_color"]->setFloat(1.0f, 0.0f, 1.0f);

	// Miss program
	miss_program = context->createProgramFromPTXFile("x64/Debug/myconstantbg.cu.ptx", "miss");
	context->setMissProgram(color_ray_type, miss_program);
	context["bg_color"]->setFloat(0.0f, 0.0f, 0.0f);

	// pathtracing program for vpl
	const char *pathtraceptx = sutil::getPtxString(NULL, pathtracecu_file.c_str());
	pathtrace_ray_gen_program = context->createProgramFromPTXString(pathtraceptx, "pathtrace_entry");
	pathtrace_miss_program = context->createProgramFromPTXString(pathtraceptx, "miss");
	pathtrace_exception_program = context->createProgramFromPTXString(pathtraceptx, "exception");
	context->setMissProgram(pathtrace_ray_type, pathtrace_miss_program);

	// Sample camera ray gen program
	const char *sampleptx = sutil::getPtxString(NULL, samplecu_file.c_str());
	sample_gen_program = context->createProgramFromPTXString(sampleptx, "pinhole_camera_samCam");
	sample_exception_program = context->createProgramFromPTXString(sampleptx, "exception");
	sample_miss_program = context->createProgramFromPTXString(sampleptx, "miss");
	samplelight_gen_program = context->createProgramFromPTXString(sampleptx, "samlight_raygen");
	sampledirect_light_gen_program = context->createProgramFromPTXString(sampleptx, "samdirect_light_raygen");

	ptpdf_ray_gen_program = context->createProgramFromPTXFile("x64/Debug/pdfestimate.cu.ptx", "estimate_pdf_entry");
	ptpdf_exception_program = context->createProgramFromPTXFile("x64/Debug/pdfestimate.cu.ptx", "exception");

	// setup miss program for lightsample ray
	context->setMissProgram(sample_ray_type, sample_miss_program);
	context->setMissProgram(sampleDirectlight_ray_type, sample_miss_program);

	// entry points settings
	setupAllEntrysContext();

	// Set up parallelogram programs
	const char *ptx = sutil::getPtxString(NULL, parallelogramcu_file.c_str());
	pgram_bounding_box = context->createProgramFromPTXString(ptx, "bounds");
	pgram_intersection = context->createProgramFromPTXString(ptx, "intersect");

	setupAllVAO();
	setupShader();

}

void OptixSampler::setupAllEntrysContext() {
	context->setEntryPointCount(6u);

	context->setRayGenerationProgram(color_scene_entry, ray_gen_program);
	context->setExceptionProgram(color_scene_entry, exception_program);

	context->setRayGenerationProgram(cam_sample_scene_entry, sample_gen_program);
	context->setExceptionProgram(cam_sample_scene_entry, sample_exception_program);

	context->setRayGenerationProgram(light_sample_scene_entry, samplelight_gen_program);
	context->setExceptionProgram(light_sample_scene_entry, sample_exception_program);

	context->setRayGenerationProgram(directlight_sample_scene_entry, sampledirect_light_gen_program);
	context->setExceptionProgram(directlight_sample_scene_entry, sample_exception_program);

	context->setRayGenerationProgram(vpl_radiance_scene_entry, pathtrace_ray_gen_program);
	context->setExceptionProgram(vpl_radiance_scene_entry, pathtrace_exception_program);

	//context->setRayGenerationProgram(pathtrace_pdf_entry, ptpdf_ray_gen_program);
	//context->setExceptionProgram(pathtrace_pdf_entry, ptpdf_exception_program);

	const char *ptcamptx = sutil::getPtxString(NULL, "optixPart/pathtracecam.cu");
	Program pt_raygen_program = context->createProgramFromPTXString(ptcamptx, "pathtrace_camera");
	Program pt_excep_program = context->createProgramFromPTXString(ptcamptx, "exception");

	context->setRayGenerationProgram(pathtrace_color_entry, pt_raygen_program);
	context->setExceptionProgram(pathtrace_color_entry, pt_excep_program);
}

// pass all vpl info to the array for cuda settings
void OptixSampler::setupVPLarr(unsigned vplnum, VPL* vplarr) {
	vpl_count = vplnum;
	opVPL* vpldest = static_cast<opVPL*>(vpl_buffer->map());
	for (int i = 0; i < vplnum; ++i) {
		vpldest[i].pos = vplarr[i].pos; 
		vpldest[i].normal = vplarr[i].normal;
		vpldest[i].color = vplarr[i].color;
		vpldest[i].seed = vplarr[i].seed;
	}
	vpl_buffer->unmap();
}

// ²¼ÖÃ³¡¾°
void OptixSampler::createScene(Context ctx, DynamicLayout *layout) {
	layout->createGeometry(ctx);
	glm::mat4 modelmat = ObjectManager::getInstance()->objects["cube"].getModelMat();
	//layout->addGeometry(ctx, cube_mesh_file, Matrix4x4(glm::value_ptr(glm::transpose(modelmat))));
	layout->addMultiGeometry(ctx, cube_mesh_file, 2);
	modelmat = ObjectManager::getInstance()->objects["cornellbox"].getModelMat();;
	layout->addGeometry(ctx, cornellmesh_file, Matrix4x4(glm::value_ptr(glm::transpose(modelmat))));
	
	const char *ptx = sutil::getPtxString(NULL, pathtracecu_file.c_str());
	Material diffuse_light = context->createMaterial();
	Program diffuse_em = context->createProgramFromPTXString(ptx, "diffuseEmitter");
	diffuse_light->setClosestHitProgram(4u, diffuse_em);
	//mat->setClosestHitProgram(0u, closest_hit);
	//mat->setAnyHitProgram(1u, any_hit);
	//mat->setClosestHitProgram(2u, sample_closest_hit);
	//mat->setClosestHitProgram(3u, samplelight_closest_hit);

	const float3 light_em = make_float3(15.0f, 15.0f, 5.0f);
	GeometryInstance plight = createParallelogram(vec3tofloat3(scene->light[0].Position()),
		make_float3(5.0f, 0.0f, 0.0f),
		make_float3(0.0f, 0.0f, 5.0f));
	plight->addMaterial(diffuse_light);
	plight["emission_color"]->setFloat(light_em);
	//layout->addGeometry(ctx, plight);
	
	layout->endGeometry(ctx);
	layout->triggerGeometryMove();
}

void OptixSampler::loadMesh(const std::string& filename)
{
	OptiXMesh mesh;
	mesh.context = context;
	OptixMeshSpace::loadMesh(filename, mesh);

	GeometryGroup geometry_group = context->createGeometryGroup();
	geometry_group->addChild(mesh.geom_instance);
	geometry_group->setAcceleration(context->createAcceleration("Trbvh"));
	context["top_object"]->set(geometry_group);
	context["top_shadower"]->set(geometry_group);
}

void OptixSampler::getPNCBufferToArr() {
	optix::Buffer posbuffer = Buffer::take(context["sample_pos_buffer"]->getBuffer()->get());
	optix::Buffer norbuffer = Buffer::take(context["sample_nor_buffer"]->getBuffer()->get());
	optix::Buffer clrbuffer = Buffer::take(context["sample_clr_buffer"]->getBuffer()->get());

	// Query buffer information
	RTsize buffer_width_rts;
	posbuffer->getSize(buffer_width_rts);
	uint32_t width = static_cast<int>(buffer_width_rts);
	RTformat buffer_format = posbuffer->getFormat();

	GLfloat* posdata = reinterpret_cast<float*>(posbuffer->map());
	GLfloat* nordata = reinterpret_cast<float*>(norbuffer->map());
	GLfloat* clrdata = reinterpret_cast<float*>(clrbuffer->map());
	//for (uint32_t i = 0; i < 10; ++i) {
	//	std::cout << data[4*i] <<" "<< data[4 * i+1] << " "
	//		<< data[4 * i + 2] << " " << data[4 * i + 3] << std::endl;
	//}

	memcpy(samplePosArr + sampleIdx * 4, posdata, width * 4 * 4);
	memcpy(sampleNorArr + sampleIdx * 4, nordata, width * 4 * 4);
	memcpy(sampleClrArr + sampleIdx * 4, clrdata, width * 4 * 4);
	sampleIdx += width;

	posbuffer->unmap();
	norbuffer->unmap();
	clrbuffer->unmap();
}

void OptixSampler::getBufferToArr(string buffer_name, float* arr, unsigned length) {
	optix::Buffer buffer = Buffer::take(context[buffer_name.c_str()]->getBuffer()->get());
	GLfloat* bufferdata = reinterpret_cast<float*>(buffer->map());
	/*for (uint32_t i = 0; i < length; ++i) {
		std::cout << bufferdata[4*i] <<" "<< bufferdata[4 * i+1] << " "
			<< bufferdata[4 * i + 2] << " " << bufferdata[4 * i + 3] << std::endl;
	}*/
	memcpy(arr, bufferdata, length * 4 * 4);
	buffer->unmap();
}


void OptixSampler::setupCamera()
{
	//camera_eye = make_float3(10 * 2, 0.0f, 0);
	//camera_lookat = make_float3(0,0,0);
	//camera_up = make_float3(0.0f, 1.0f, 0.0f);
	camera_eye = vec3tofloat3(scene->camera.Position());

	glm::vec3 target = scene->camera.Position() + scene->camera.Forward();
	camera_lookat = vec3tofloat3(target);
	camera_up = vec3tofloat3(scene->camera.Up());

	camera_rotate = Matrix4x4::identity();
}

void OptixSampler::setupLights()
{
	//const float max_dim = fmaxf(aabb.extent(0), aabb.extent(1)); // max of x, y components

	BasicLight lights[] = {
		{ vec3tofloat3(scene->light[0].Position()), vec3tofloat3(scene->light[0].Diffuse()), vec3tofloat3(scene->light[0].Forward()),  1, 0 }
		//{ make_float3( -0.5f,  0.25f, -1.0f ), make_float3( 0.2f, 0.2f, 0.25f ), make_float3( 00f, 0.0f, 0.0f ), 0, 0 },
		//{ make_float3( -0.5f,  0.0f ,  1.0f ), make_float3( 0.1f, 0.1f, 0.10f ), make_float3( 00f, 0.0f, 0.0f ), 0, 0 },
		//{ make_float3(  0.5f,  0.5f ,  0.5f ), make_float3( 0.7f, 0.7f, 0.65f ), make_float3( 00f, 0.0f, 0.0f ), 1, 0 },
	};
	//lights[1].pos *= 10.0f; 
	//lights[2].pos *= 10.0f; 
	//lights[3].pos *= 10.0f; 

	Buffer light_buffer = context->createBuffer(RT_BUFFER_INPUT);
	light_buffer->setFormat(RT_FORMAT_USER);
	light_buffer->setElementSize(sizeof(BasicLight));
	light_buffer->setSize(sizeof(lights) / sizeof(lights[0]));
	memcpy(light_buffer->map(), lights, sizeof(lights));
	light_buffer->unmap();

	context["lights"]->set(light_buffer);
}

void OptixSampler::Initialize(Scene* scn)
{
	int usage_report_level = 0;

	try
	{
		this->scene = scn;
		width = this->scene->width;
		height = this->scene->height;
		UsageReportLogger logger;
		createContext(usage_report_level, &logger);
		layout = new SeparateAccelsLayout("Trbvh", false);
		createScene(context, layout);
		const optix::Aabb aabb = layout->getSceneBBox();
		setupCamera();
		setupLights();

		//context->validate();
		//updateCamera();
		//context->launch( 0, width, height );
		//sutil::displayBufferPPM( out_file.c_str(), getOutputBuffer() );
		//destroyContext();
	}
	SUTIL_CATCH(context->get())
}

GeometryInstance OptixSampler::createParallelogram(
	const float3& anchor,
	const float3& offset1,
	const float3& offset2)
{
	Geometry parallelogram = context->createGeometry();
	parallelogram->setPrimitiveCount(1u);
	parallelogram->setIntersectionProgram(pgram_intersection);
	parallelogram->setBoundingBoxProgram(pgram_bounding_box);

	float3 normal = normalize(cross(offset1, offset2));
	float d = dot(normal, anchor);
	float4 plane = make_float4(normal, d);

	float3 v1 = offset1 / dot(offset1, offset1);
	float3 v2 = offset2 / dot(offset2, offset2);

	parallelogram["plane"]->setFloat(plane);
	parallelogram["anchor"]->setFloat(anchor);
	parallelogram["v1"]->setFloat(v1);
	parallelogram["v2"]->setFloat(v2);

	GeometryInstance gi = context->createGeometryInstance();
	gi->setGeometry(parallelogram);
	return gi;
}


void OptixSampler::updateCamera()
{
	const float aspect_ratio = static_cast<float>(width) /
		static_cast<float>(height);
	float3 camera_u, camera_v, camera_w;
	vfov = scene->camera.FieldOfView();
	sutil::calculateCameraVariables(
		camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
		camera_u, camera_v, camera_w, true);

	const Matrix4x4 frame = Matrix4x4::fromBasis(
		normalize(camera_u),
		normalize(camera_v),
		normalize(-camera_w),
		camera_lookat);
	const Matrix4x4 frame_inv = frame.inverse();
	// Apply camera rotation twice to match old SDK behavior
	const Matrix4x4 trans = frame*camera_rotate*camera_rotate*frame_inv;

	camera_eye = make_float3(trans*make_float4(camera_eye, 1.0f));
	camera_lookat = make_float3(trans*make_float4(camera_lookat, 1.0f));
	camera_up = make_float3(trans*make_float4(camera_up, 0.0f));

	sutil::calculateCameraVariables(
		camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
		camera_u, camera_v, camera_w, true);

	camera_rotate = Matrix4x4::identity();

	context["eye"]->setFloat(camera_eye);
	context["U"]->setFloat(camera_u);
	context["V"]->setFloat(camera_v);
	context["W"]->setFloat(camera_w);
}


void OptixSampler::update()
{
	static unsigned frame_count = 0;
	context["frame_number"]->setUint(frame_count);

	layout->updateGeometry();
	updateCamera();
	sampleIdx = 0;

	context->validate();
	context->launch(cam_sample_scene_entry, samplenum);
	getBufferContent(NULL, true);

	//context->launch(light_sample_scene_entry, samplenum);
	//getBufferContent(NULL, true);

	//context->launch(directlight_sample_scene_entry, samplenum);
	//getBufferContent(NULL, true);
	
	//context->launch(pathtrace_color_entry, width, height);
	//getBufferContent(getOutputBuffer(), false);
	frame_count++;
}

void OptixSampler::estimateRadiosity(float* outrad) 
{
	context->validate();
	context->launch(vpl_radiance_scene_entry, vpl_count);
	getBufferToArr("radiosity_buffer", outrad, vpl_count);
}

void OptixSampler::estimatePdf(float* outpdf)
{
	context->validate();
	context->launch(pathtrace_pdf_entry, vpl_count);
	getBufferToArr("contri_buffer", outpdf, vpl_count);
}

void OptixSampler::draw(unsigned entrypoint) {
	context->launch(entrypoint, width, height);
	getBufferContent(getOutputBuffer(), false);
}


//------------------------------------------------------------------------------
//
//  GLUT callbacks
//
//------------------------------------------------------------------------------

// event handler
#pragma region EVENT_HANDLER
void OptixSampler::glutKeyboardPress(unsigned char k, int x, int y)
{

	switch (k)
	{
	case('q'):
	case(27): // ESC
	{
		destroyContext();
		exit(0);
	}
	case('s'):
	{
		const std::string outputImage = std::string(SAMPLE_NAME) + ".ppm";
		std::cerr << "Saving current frame to '" << outputImage << "'\n";
		sutil::displayBufferPPM(outputImage.c_str(), getOutputBuffer());
		break;
	}
	case('d'):
	{
		camera_eye -= make_float3(0.0f, 0.0f, 3);
		updateCamera();
		break;
	}
	case('e'):
	{
		camera_eye += make_float3(0.0f, 0.0f, 3);
		updateCamera();
		break;
	}
	case('r'):
	{
		layout->resetGeometry();
		break;
	}
	case(' '):
	{
		layout->triggerGeometryMove();
		break;
	}
	}
}


void OptixSampler::glutMousePress(int button, int state, int x, int y)
{
	if (state == GLFW_PRESS)
	{
		mouse_button = button;
		mouse_prev_pos = make_int2(x, y);
	}
	else
	{
		// nothing
	}
}


void OptixSampler::glutMouseMotion(int x, int y)
{
	if (mouse_button == GLFW_PRESS)
	{
		const float dx = static_cast<float>(x - mouse_prev_pos.x) /
			static_cast<float>(width);
		const float dy = static_cast<float>(y - mouse_prev_pos.y) /
			static_cast<float>(height);
		const float dmax = fabsf(dx) > fabs(dy) ? dx : dy;
		const float scale = fminf(dmax, 0.9f);
		camera_eye = camera_eye + (camera_lookat - camera_eye)*scale;
	}
	else if (mouse_button == GLFW_PRESS)
	{
		const float2 from = { static_cast<float>(mouse_prev_pos.x),
			static_cast<float>(mouse_prev_pos.y) };
		const float2 to = { static_cast<float>(x),
			static_cast<float>(y) };

		const float2 a = { from.x / width, from.y / height };
		const float2 b = { to.x / width, to.y / height };

		camera_rotate = arcball.rotate(b, a);
	}
	else if (mouse_button == GLFW_PRESS)
	{
		const float2 from = { static_cast<float>(mouse_prev_pos.x),
			static_cast<float>(mouse_prev_pos.y) };
		const float2 to = { static_cast<float>(x),
			static_cast<float>(y) };
	}

	mouse_prev_pos = make_int2(x, y);
}


void OptixSampler::glutResize(int w, int h)
{
	if (w == (int)width && h == (int)height) return;

	width = w;
	height = h;
	sutil::ensureMinimumSize(width, height);

	sutil::resizeBuffer(getOutputBuffer(), width, height);

	glViewport(0, 0, width, height);

}

#pragma endregion

// Converts the buffer format to gl format
GLenum OptixSampler::glFormatFromBufferFormat(bufferPixelFormat pixel_format, RTformat buffer_format)
{
	if (buffer_format == RT_FORMAT_UNSIGNED_BYTE4)
	{
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_BGRA;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGRA;
		default:
			throw Exception("Unknown buffer pixel format");
		}
	}
	else if (buffer_format == RT_FORMAT_FLOAT4)
	{
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGRA;
		default:
			throw Exception("Unknown buffer pixel format");
		}
	}
	else if (buffer_format == RT_FORMAT_FLOAT3)
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_RGB;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGB;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGR;
		default:
			throw Exception("Unknown buffer pixel format");
		}
	else if (buffer_format == RT_FORMAT_FLOAT)
		return GL_LUMINANCE;
	else
		throw Exception("Unknown buffer format");
}



//------------------------------------------------------------------------------
//
// Main
//
//------------------------------------------------------------------------------

void OptixSampler::printUsageAndExit(const std::string& argv0)
{
	std::cerr << "\nUsage: " << argv0 << " [options]\n";
	std::cerr <<
		"App Options:\n"
		"  -h | --help               Print this usage message and exit.\n"
		"  -f | --file               Save single frame to file and exit.\n"
		"  -n | --nopbo              Disable GL interop for display buffer.\n"
		"  -m | --mesh <mesh_file>   Specify path to mesh to be loaded.\n"
		"  -r | --report <LEVEL>     Enable usage reporting and report level [1-3].\n"
		"App Keystrokes:\n"
		"  q  Quit\n"
		"  s  Save image to '" << SAMPLE_NAME << ".ppm'\n"
		<< std::endl;

	exit(1);
}