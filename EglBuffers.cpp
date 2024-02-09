#include "EglBuffers.hpp"

#include <libdrm/drm_fourcc.h>

static void MyEglError(){
    EGLint error = eglGetError();  // Get the last error
    std::string errorMessage;

    // Match the error code with predefined EGL error codes
    switch(error) {
        case EGL_SUCCESS:
            errorMessage = "The last function succeeded without error.";
            return;
            break;
        case EGL_NOT_INITIALIZED:
            errorMessage = "EGL is not initialized, or could not be initialized, for the specified EGL display connection.";
            break;
        case EGL_BAD_ACCESS:
            errorMessage = "EGL cannot access a requested resource.";
            break;
        case EGL_BAD_ALLOC:
            errorMessage = "EGL failed to allocate resources for the requested operation.";
            break;
        case EGL_BAD_ATTRIBUTE:
            errorMessage = "An unrecognized attribute or attribute value was passed in the attribute list.";
            break;
        case EGL_BAD_CONTEXT:
            errorMessage = "An EGLContext argument does not name a valid EGL rendering context.";
            break;
        case EGL_BAD_CONFIG:
            errorMessage = "An EGLConfig argument does not name a valid EGL frame buffer configuration.";
            break;
        case EGL_BAD_CURRENT_SURFACE:
            errorMessage = "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.";
            break;
        case EGL_BAD_DISPLAY:
            errorMessage = "An EGLDisplay argument does not name a valid EGL display connection.";
            break;
        case EGL_BAD_SURFACE:
            errorMessage = "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.";
            break;
        default:
            errorMessage = "Unknown EGL error.";
    }

    std::cout << "EGL ERROR: "
                    << errorMessage << " :::: " << error << std::endl;
}


int getSharedProcFd(int procid, int fd){
    int pid_fd = syscall(SYS_pidfd_open, procid, 0);
    if (pid_fd == -1) {
        perror("pidfd_open");
        return -1;
    }

    int new_fd_raw = syscall(SYS_pidfd_getfd, pid_fd, fd, 0);
    if (new_fd_raw == -1) {
        printf("%d , %d\r\n", pid_fd, fd);
        perror("pidfd_getfd");
        close(pid_fd);
        return -1;
    }

    close(pid_fd);

    return new_fd_raw;
}

static void get_colour_space_info(std::optional<libcamera::ColorSpace> const &cs, EGLint &encoding, EGLint &range)
{
	encoding = EGL_ITU_REC601_EXT;
	range = EGL_YUV_NARROW_RANGE_EXT;

	if (cs == libcamera::ColorSpace::Sycc)
		range = EGL_YUV_FULL_RANGE_EXT;
	else if (cs == libcamera::ColorSpace::Smpte170m)
		/* all good */;
	else if (cs == libcamera::ColorSpace::Rec709)
		encoding = EGL_ITU_REC709_EXT;
}

int EglBuffers::init(){
    int err = 0;

    static const int MAX_DEVICES = 4;
    EGLDeviceEXT eglDevs[MAX_DEVICES];
    EGLint numDevices;

    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
    eglQueryDevicesEXT(MAX_DEVICES, eglDevs, &numDevices);
    printf("Detected %d devices\n", numDevices);

    MyEglError();

    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
    (PFNEGLGETPLATFORMDISPLAYEXTPROC)
    eglGetProcAddress("eglGetPlatformDisplayEXT");
    eglDpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, 
                                    eglDevs[0], 0);
    
    MyEglError();

    if (eglDpy == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        err = -1;
    }

    EGLint major, minor;
    if (eglInitialize(eglDpy, &major, &minor) == EGL_FALSE) {
            fprintf(stderr, "Failed to initialize EGL display: %x\n", eglGetError());
            err = -1;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    MyEglError();

    return err;
}

void EglBuffers::makeBuffer(const SharedMemoryBuffer* context, FrameBuffer &buffer)
{
	buffer.raw.fd = getSharedProcFd(context->procid, context->fd_raw);
    buffer.raw.size = context->raw_length;
    buffer.raw.info = context->raw;

    buffer.isp.fd = getSharedProcFd(context->procid, context->fd_isp);
    buffer.isp.size = context->isp_length;
    buffer.isp.info = context->isp;

    buffer.luma.size = context->isp_length;
    buffer.luma.info = context->isp;

    // buffer.lores.fd = getSharedProcFd(context->procid,context->fd_lores);
    // buffer.lores.size = context->lores_length;
    // buffer.lores.info = context->lores;

    buffer.framerate = context->framerate;
    buffer.sequence = context->sequence;

    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;
    eglCreateImageKHR = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    if (!eglCreateImageKHR) {
        MyEglError();
    }

    PFNEGLQUERYDMABUFFORMATSEXTPROC eglQueryDmaBufFormatsEXT = nullptr;
    eglQueryDmaBufFormatsEXT = reinterpret_cast<PFNEGLQUERYDMABUFFORMATSEXTPROC>(eglGetProcAddress("eglQueryDmaBufFormatsEXT"));
    if (!eglQueryDmaBufFormatsEXT) {
        MyEglError();
    }

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
    glEGLImageTargetTexture2DOES = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    if (!glEGLImageTargetTexture2DOES) {
        MyEglError();
    }

    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = nullptr;
    eglDestroyImageKHR = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    if (!eglDestroyImageKHR) {
        MyEglError();
    }


	get_colour_space_info(buffer.raw.info.colour_space, buffer.raw.encoding, buffer.raw.range);
    get_colour_space_info(buffer.isp.info.colour_space, buffer.isp.encoding, buffer.isp.range);
    // get_colour_space_info(buffer.lores.info.colour_space, buffer.lores.encoding, buffer.lores.range);

    EGLint attribs_raw[] = {
        EGL_WIDTH, static_cast<EGLint>(buffer.raw.info.width),
        EGL_HEIGHT, static_cast<EGLint>(buffer.raw.info.height),
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_R16,
        EGL_DMA_BUF_PLANE0_FD_EXT, buffer.raw.fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(buffer.raw.info.stride),
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE
    };

	EGLint attribs_isp[] = {
		EGL_WIDTH, static_cast<EGLint>(buffer.isp.info.width),
		EGL_HEIGHT, static_cast<EGLint>(buffer.isp.info.height),
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_YUV420,
		EGL_DMA_BUF_PLANE0_FD_EXT, buffer.isp.fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(buffer.isp.info.stride),
		EGL_DMA_BUF_PLANE1_FD_EXT, buffer.isp.fd,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, static_cast<EGLint>(buffer.isp.info.stride * buffer.isp.info.height),
		EGL_DMA_BUF_PLANE1_PITCH_EXT, static_cast<EGLint>(buffer.isp.info.stride / 2),
		EGL_DMA_BUF_PLANE2_FD_EXT, buffer.isp.fd,
		EGL_DMA_BUF_PLANE2_OFFSET_EXT, static_cast<EGLint>(buffer.isp.info.stride * buffer.isp.info.height + (buffer.isp.info.stride / 2) * (buffer.isp.info.height / 2)),
		EGL_DMA_BUF_PLANE2_PITCH_EXT, static_cast<EGLint>(buffer.isp.info.stride / 2),
		EGL_YUV_COLOR_SPACE_HINT_EXT, buffer.isp.encoding,
		EGL_SAMPLE_RANGE_HINT_EXT, buffer.isp.range,
		EGL_NONE
	};

    EGLint attribs_luma[] = {
        EGL_WIDTH, static_cast<EGLint>(buffer.isp.info.width),
        EGL_HEIGHT, static_cast<EGLint>(buffer.isp.info.height),
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_R8,
        EGL_DMA_BUF_PLANE0_FD_EXT, buffer.isp.fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(buffer.isp.info.stride),
        EGL_NONE
    };

    EGLImage image_raw = eglCreateImageKHR(eglDpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs_raw);
	if (!image_raw){
        MyEglError();
		throw std::runtime_error("failed to import fd " + std::to_string(buffer.isp.fd));
    }
        
	EGLImage image_isp = eglCreateImageKHR(eglDpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs_isp);
	if (!image_isp){
        MyEglError();
		throw std::runtime_error("failed to import fd " + std::to_string(buffer.isp.fd));
    }

    EGLImage image_luma = eglCreateImageKHR(eglDpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs_luma);
    if (!image_luma){
        MyEglError();
        throw std::runtime_error("failed to import fd " + std::to_string(buffer.isp.fd));
    }

    glGenTextures(1, &buffer.raw.texture);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, buffer.raw.texture);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image_raw);

    MyEglError();

	glGenTextures(1, &buffer.isp.texture);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, buffer.isp.texture);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image_isp);

    MyEglError();

    glGenTextures(1, &buffer.luma.texture);
	glBindTexture(GL_TEXTURE_2D, buffer.luma.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image_luma);

    MyEglError();

	eglDestroyImageKHR(eglDpy, image_isp);
    eglDestroyImageKHR(eglDpy, image_raw);
    eglDestroyImageKHR(eglDpy, image_luma);
    buffer.mapped = 1;
    console->info("Buffer mapped! raw:{}, isp:{}, lores:{}", buffer.raw.fd, buffer.isp.fd, buffer.lores.fd);
}


void EglBuffers::update(){
    if(!shared.connected()){
        if(!first_time_){
            reset();
        }
        return;
    }

    if(first_time_)
        first_time_ = false;
        
    current_index_ = context->fd_raw;
    FrameBuffer &buffer = buffers_[current_index_];
	if (buffer.mapped == -1 && current_index_ != -1){
        makeBuffer(context, buffer);
    }
		
    if(last_fd_ != current_index_){
        newFrame_ = true;
        console->info("FD: {}, FN: {}, FR: {}, SEQ: {}", current_index_, context->frame, context->metadata.focus, context->metadata.exposure_time);
    }

    last_fd_ = current_index_;
}