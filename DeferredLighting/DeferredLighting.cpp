 /*!
 *	\brief		Deferred Lighting example
 *	\details	Renders a simple example model using the multi-pass deferred lighting renderer
 *	\file		DeferredLighting.cpp
 *	\author		Christian Magnerfelt
 *	\version	0.1
 *	\date		2012.08.01
 *	\copyright	GNU Public License
 */

#pragma once

#include <GL/glew.h>				 //!< OpenGL EXT support
#include <GL/freeglut.h>			 //!<  Window management library
#include <Cg/cg.h>					 //!<  CG Run-Time Library API
#include <Cg/cgGL.h>				 //!<  CGGL Run-Time Library API
#include <iostream>					 //!<  Standard IO interface

// Required libraries
#pragma comment (lib , "cg.lib")
#pragma comment (lib , "cgGL.lib")
#pragma comment (lib , "freeglut.lib")
#pragma comment (lib , "glew32.lib")


inline int getDefaultScreenWidth(){return 800; } const
inline int getDefaultScreenHeight(){return 600; } const
inline const char * getWindowTitle(){return "Deferred Lighting Demo"; } const

int currentScreenWidth = ::getDefaultScreenWidth();
int currentScreenHeight = ::getDefaultScreenHeight();

GLuint fboId;								//!< Id to our frame buffer object ( G-Buffer )
GLuint gBufferId;							//!< ID of the G-Buffer part that contains normals and specular exponent at each pixel
GLuint gBufferDepthId;						//!< ID of the G-Buffer part that contains the depth at each pixel

CGcontext cgContext;
CGprofile cgVertexProfile;
CGprofile cgFragmentProfile;

// Our vertex and fragment programs used for this demo
CGprogram cgVertexProgram;
CGprogram cgFragmentProgram;

/*!
 * Forward declarations
 */
void display();
void reshape(int width, int height);
void idle();
void keyboard(unsigned char key, int x , int y);
void initFrameBufferObject(int width, int height);
void releaseFrameBufferObject();
void shaderSetup();
void cleanUp();
void checkFramebufferStatus();
void checkGLErrors(const char * action);
void checkForCgError(const char * situation);

/*!
 * \brief Main enetry point for the program
 * \details Initializes window and FBO. Enters glut main loop when initialization is complete.
 * \return Return 0 after glut main loop has been been succesfully terminated
 */
int main(int argc, char * argv [])
{
	glutInit(&argc, argv);
	glutInitWindowSize(::currentScreenWidth, ::currentScreenHeight);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(::getWindowTitle());
	
	// Init extentions
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr)
	{
		std::cout << glewGetErrorString(glewErr) << std::endl;
	}
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	initFrameBufferObject(::currentScreenWidth,::currentScreenHeight);
	glutMainLoop();
	releaseFrameBufferObject();
	return 0;
}

/*!
 * \brief Display callback
 */
void display()
{
	glutSwapBuffers();
}

/*!
 * \brief Reshape callback
 * \details Forces same width and height on resize
 * \param width Not used, The new window width 
 * \param height Not used, The new window height
 */
void reshape(int width, int height)
{
	glutReshapeWindow(::currentScreenWidth,::currentScreenHeight); 
}

/*!
 * \brief Idle callback
 */
void idle()
{
	glutPostRedisplay();
}

/*!
 * \brief Keyboard callback
 * \param key The id of the pressed key
 * \param x Mouse x-position
 * \param y Mouse y-position
 */
void keyboard(unsigned char key, int x , int y)
{
	switch(key)
	{
	case (27) :		// Exit when ESC is pressed
		cleanUp();
		glutLeaveMainLoop();
		break;
	default:
		break;
	}
}

/*!
 * \brief Initializes the frame buffer object in video memory
 * \details The G-Buffer represented by the frame buffer object, consists of two textures. The first texture uses RGBA format 
 * which is used to store the normals and specular emission. The RGB part holds the normals and the A part
 * holds the spuclar emission. The second texture holds the depth of the scene. The texture format is depth using
 * 24 bits. Note that texture will be upscaled to 32 bits in order to preserve the conistence wihtin the frame buffer object
 * \param width In pixels, of the frame buffer object
 * \param height In pixels, of the frame buffer object
 */
void initFrameBufferObject(int width, int height)
{
	// Generate and bind G-Buffer
	glGenTextures(1, &gBufferId);
	glBindTexture(GL_TEXTURE_2D, gBufferId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, 0); 
	checkGLErrors("Generating G-Buffer texture RGBA");

	glGenTextures(1, &gBufferDepthId);
	glBindTexture(GL_TEXTURE_2D, gBufferDepthId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D   (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, 
					GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	checkGLErrors("Generating G-Buffer texture DEPTH");
	
	// Generate frame buffer object
	glGenFramebuffersEXT(1, &fboId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	checkGLErrors("Generating frame buffer object");

	// Attach G-Buffer to FBO
	glFramebufferTexture2DEXT  (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								GL_TEXTURE_2D, gBufferId, 0);
	checkGLErrors("Attach G-Buffer texture RGBA");

	glFramebufferTexture2DEXT  (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
								GL_TEXTURE_2D, gBufferDepthId, 0);
	checkGLErrors("Attach G-Buffer texture Depth");

	// Enable drawing for all color buffer attachments
	GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0_EXT}; 
	glDrawBuffers(1, drawbuffers);
	checkGLErrors("Enable drawing to attachments");
	// Check FBO status
	void checkFramebufferStatus();
}

/*!
 * \brief Frame buffer object clean up
 */
void releaseFrameBufferObject()
{
	glDeleteFramebuffersEXT(1,&fboId);
	glDeleteTextures(1,&gBufferId);
	glDeleteTextures(1,&gBufferDepthId);
}

/*!
 * \brief 
 */
void shaderSetup()
{
	cgContext = cgCreateContext();
	checkForCgError("Creating Context");
	cgGLSetDebugMode(CG_FALSE);

	// CG parameters will update immediately
	cgSetParameterSettingMode(cgContext, CG_IMMEDIATE_PARAMETER_SETTING);

	// Set up vertex profile & program
	cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);					
	cgGLSetOptimalOptions(cgVertexProfile);
	checkForCgError("Selecting Vertex Profile");

	cgVertexProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, "DeferredLightingShaders.cg", 
											  cgVertexProfile, "blinnPhongVTF", NULL);																
	checkForCgError("Creating Vertex Program");

	cgGLLoadProgram(cgVertexProgram);
	checkForCgError("Loading Vertex Program");

	// Set up fragment profile & program
	cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);					
	cgGLSetOptimalOptions(cgFragmentProfile);
	checkForCgError("Selecting Fragment Profile");

	cgFragmentProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, "DeferredLightingShaders.cg",											
												cgVertexProfile, "blinnPhongFTB", NULL);																
	checkForCgError("Creating Fragment Program");

	cgGLLoadProgram(cgFragmentProgram);
	checkForCgError("Loading Fragment Program");
}

/*!
 * \brief 
 */
void cleanUp()
{
	releaseFrameBufferObject();
}

/*!
 * \brief Checks for OpenGL errors
 * \param action OpenGL action that provoked the error
 */
void checkGLErrors(const char * action)
{
	GLenum status = glGetError();
	switch(status)
	{
		case GL_NO_ERROR:
			// OK  
			break;
		case GL_INVALID_ENUM:
			std::cerr << action 
				<< " : An unacceptable value is specified for an enumerated argument" 
				<< std::endl;
			break;
		case GL_INVALID_OPERATION:
			std::cerr << action 
				<< " : The specified operation is not allowed in the current state" 
				<< std::endl;
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << action << " : Stack Overflow" << std::endl;
			break;
		default:
			std::cerr << action << " : GL error " << status << std::endl;
	}

}

/*!
 * \brief Checks that frame buffer object status is OK
 */
void checkFramebufferStatus()
{
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			// Frame buffer status OK
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			std::cerr << "FBO Error : Framebuffer configuration not supported" 
				<< std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			std::cerr << "FBO Error : Incomplete attachment" << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			std::cerr << "FBO Error : Incomplete missing attachment" << std::endl;
			break;
		default:
			std::cerr << "FBO Error : " << status << std::endl;
	}
}

/*!
 * \brief Check for errors in Cg runtime
 * \param situation The context of the error
 */
void checkForCgError(const char * situation)
{
	CGerror error;
	const char *string = cgGetLastErrorString(&error);

	if (error != CG_NO_ERROR) 
	{
		std::cerr << ::GetTitleBarInfo << " : " << situation << " : " << string << std::endl;
		if (error == CG_COMPILER_ERROR) 
		{
			std::cerr << cgGetLastListing(cgContext) << std::endl;
		}
		system("pause");
  }
}