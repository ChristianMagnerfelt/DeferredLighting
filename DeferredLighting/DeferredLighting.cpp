//
// AUTHOR:	Christian Magnerfelt
// DATE:	2012.05.12
// Description: Main file for Deferred Lighting rendering process
//

#pragma once

// OpenGL
#include <GL/glew.h>
#include <GL/freeglut.h>

// Cg
#include <Cg/cg.h>
#include <Cg/cgGL.h>

// STL
#include <iostream>

// Add libraries
#pragma comment (lib , "cg.lib")
#pragma comment (lib , "cgGL.lib")
#pragma comment (lib , "freeglut.lib")
#pragma comment (lib , "glew32.lib")

///////// Global constants
inline int getDefaultScreenWidth(){return 800; } const
inline int getDefaultScreenHeight(){return 600; } const
inline const char * getWindowTitle(){return "Deferred Lighting Demo"; } const

//////// Global variables
int currentScreenWidth = ::getDefaultScreenWidth();
int currentScreenHeight = ::getDefaultScreenHeight();
GLuint fboId;
GLuint gBufferId;
GLuint gBufferDepthId;

///////// Forward declarations
void display();
void reshape(int width, int height);
void idle();
void keyboard(unsigned char key, int x , int y);
void initFrameBufferObject(int width, int height);
void releaseFrameBufferObject();
void cleanUp();
void checkGLErrors(const char * action);

int main(int argc, char * argv [])
{
	glutInit(&argc, argv);
	glutInitWindowSize(::currentScreenWidth, ::currentScreenHeight);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(::getWindowTitle());
	
	// GlewInit
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

void display()
{
	glutSwapBuffers();
}
void reshape(int width, int height)
{
	// Force same width and height on resize
	glutReshapeWindow(::currentScreenWidth,::currentScreenHeight); 
}
void idle()
{
	glutPostRedisplay();
}
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
void initFrameBufferObject(int width, int height)
{
	// Generate and bind G-Buffer
	// G-Buffer structure COLOR0:
	// RGB	: View space normals
	// A	: Specular emission
	glGenTextures(1, &gBufferId);
	glBindTexture(GL_TEXTURE_2D, gBufferId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, 0); 
	checkGLErrors("Generating G-Buffer texture RGBA");

	// G-Buffer structure DEPTH:
	// DEPTH24
	glGenTextures(1, &gBufferDepthId);
	glBindTexture(GL_TEXTURE_2D, gBufferDepthId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// 24 bit depth texture format
	glTexImage2D   (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, 
					GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	checkGLErrors("Generating G-Buffer texture DEPTH");
	
	// Generate frame buffer object
	glGenFramebuffersEXT(1, &fboId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	checkGLErrors("Generating frame buffer object");

	// Attach normal & specular buffer to frame buffer object
	glFramebufferTexture2DEXT  (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								GL_TEXTURE_2D, gBufferId, 0);
	checkGLErrors("Attach G-Buffer texture RGBA");

	// Attach depth buffer to frame buffer object
	glFramebufferTexture2DEXT  (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
								GL_TEXTURE_2D, gBufferDepthId, 0);
	checkGLErrors("Attach G-Buffer texture Depth");

	// Enable drawing for all color buffer attachments
	GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0_EXT}; 
	glDrawBuffers(1, drawbuffers);
	checkGLErrors("Enable drawing to attachments");
	void checkFramebufferStatus(); 
}
void releaseFrameBufferObject()
{
	glDeleteFramebuffersEXT(1,&fboId);
	glDeleteTextures(1,&gBufferId);
	glDeleteTextures(1,&gBufferDepthId);
}
void cleanUp()
{
	releaseFrameBufferObject();
}
//////// Error handling function definitions
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
void checkFramebufferStatus()
{
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			//GBuffer bind OK
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
			//Error
			std::cerr << "FBO Error : " << status << std::endl;
	}
}