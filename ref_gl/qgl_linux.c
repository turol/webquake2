/*
** QGL_WIN.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake2 you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/
#define QGL
#include "../ref_gl/gl_local.h"

#include <float.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL.h>


#ifdef EMSCRIPTEN


// these are here only so we can compile
// TODO: use an actual opengl loader
void glBindBuffer(GLenum target, GLuint buffer);
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
void glGenBuffers(GLsizei n, GLuint *buffers);


#endif // EMSCRIPTEN


typedef struct Vertex {
	float pos[3];
	uint32_t color;
	float tex0[2];
	float tex1[2];
} Vertex;


#define NUMMATRICES 32


typedef struct QGLState {
	Vertex *vertices;
	unsigned int numVertices;
	unsigned int usedVertices;

	GLenum primitive;

	Vertex currentVertex;

	unsigned int activeTexture;

	GLenum matrixMode;

	float mvMatrices[NUMMATRICES][16];
	float projMatrices[NUMMATRICES][16];

	int mvMatrixTop, projMatrixTop;
	bool mvMatrixDirty, projMatrixDirty;

	GLuint vbo;
} QGLState;


static QGLState *qglState = NULL;


void * qwglGetProcAddress(const char *procname)
{
	return SDL_GL_GetProcAddress(procname);
}

void ( APIENTRY * qglActiveTextureARB) ( GLenum );
void ( APIENTRY * qglClientActiveTextureARB) ( GLenum );

void ( APIENTRY * qglAlphaFunc )(GLenum func, GLclampf ref);
void ( APIENTRY * qglBindTexture )(GLenum target, GLuint texture);
void ( APIENTRY * qglBlendFunc )(GLenum sfactor, GLenum dfactor);
void ( APIENTRY * qglClear )(GLbitfield mask);
void ( APIENTRY * qglClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void ( APIENTRY * qglClearDepth )(GLclampd depth);
void ( APIENTRY * qglClearStencil )(GLint s);
void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void ( APIENTRY * qglColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglCullFace )(GLenum mode);
void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint *textures);
void ( APIENTRY * qglDepthFunc )(GLenum func);
void ( APIENTRY * qglDepthMask )(GLboolean flag);
void ( APIENTRY * qglDepthRange )(GLclampd zNear, GLclampd zFar);
void ( APIENTRY * qglDisable )(GLenum cap);
void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void ( APIENTRY * qglEnable )(GLenum cap);
void ( APIENTRY * qglFinish )(void);
void ( APIENTRY * qglFlush )(void);
void ( APIENTRY * qglFrontFace )(GLenum mode);
void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint *textures);
void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean *params);
GLenum ( APIENTRY * qglGetError )(void);
void qglGetFloatv(GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint *params);
const GLubyte * ( APIENTRY * qglGetString )(GLenum name);
void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
void ( APIENTRY * qglLineWidth )(GLfloat width);
void ( APIENTRY * qglPolygonMode )(GLenum face, GLenum mode);
void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglShadeModel )(GLenum mode);
void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
void ( APIENTRY * qglStencilMask )(GLuint mask);
void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
void ( APIENTRY * qglTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglTexEnvf )(GLenum target, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexEnvi )(GLenum target, GLenum pname, GLint param);
void ( APIENTRY * qglTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTexParameterf )(GLenum target, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexParameteri )(GLenum target, GLenum pname, GLint param);
void ( APIENTRY * qglTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

static void ( APIENTRY * dllAlphaFunc )(GLenum func, GLclampf ref);
static void ( APIENTRY * dllBegin )(GLenum mode);
static void ( APIENTRY * dllBindTexture )(GLenum target, GLuint texture);
static void ( APIENTRY * dllBlendFunc )(GLenum sfactor, GLenum dfactor);
static void ( APIENTRY * dllClear )(GLbitfield mask);
static void ( APIENTRY * dllClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static void ( APIENTRY * dllClearDepth )(GLclampd depth);
static void ( APIENTRY * dllClearStencil )(GLint s);
static void ( APIENTRY * dllColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static void ( APIENTRY * dllColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllCullFace )(GLenum mode);
static void ( APIENTRY * dllDeleteTextures )(GLsizei n, const GLuint *textures);
static void ( APIENTRY * dllDepthFunc )(GLenum func);
static void ( APIENTRY * dllDepthMask )(GLboolean flag);
static void ( APIENTRY * dllDepthRange )(GLclampd zNear, GLclampd zFar);
static void ( APIENTRY * dllDisable )(GLenum cap);
static void ( APIENTRY * dllDrawArrays )(GLenum mode, GLint first, GLsizei count);
static void ( APIENTRY * dllDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
static void ( APIENTRY * dllEnable )(GLenum cap);
static void ( APIENTRY * dllEnd )(void);
static void ( APIENTRY * dllFinish )(void);
static void ( APIENTRY * dllFlush )(void);
static void ( APIENTRY * dllFrontFace )(GLenum mode);
static void ( APIENTRY * dllFrustum )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
static void ( APIENTRY * dllGenTextures )(GLsizei n, GLuint *textures);
static void ( APIENTRY * dllGetBooleanv )(GLenum pname, GLboolean *params);
GLenum ( APIENTRY * dllGetError )(void);
static void ( APIENTRY * dllGetIntegerv )(GLenum pname, GLint *params);
const GLubyte * ( APIENTRY * dllGetString )(GLenum name);
static void ( APIENTRY * dllHint )(GLenum target, GLenum mode);
GLboolean ( APIENTRY * dllIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * dllIsTexture )(GLuint texture);
static void ( APIENTRY * dllLineWidth )(GLfloat width);
static void ( APIENTRY * dllLoadIdentity )(void);
static void ( APIENTRY * dllLoadMatrixf )(const GLfloat *m);
static void ( APIENTRY * dllMultMatrixf )(const GLfloat *m);
static void ( APIENTRY * dllOrtho )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
static void ( APIENTRY * dllPolygonMode )(GLenum face, GLenum mode);
static void ( APIENTRY * dllPolygonOffset )(GLfloat factor, GLfloat units);
static void ( APIENTRY * dllPopMatrix )(void);
static void ( APIENTRY * dllPushMatrix )(void);
static void ( APIENTRY * dllReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
GLint ( APIENTRY * dllRenderMode )(GLenum mode);
static void ( APIENTRY * dllRotated )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllRotatef )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllScaled )(GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllScalef )(GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
static void ( APIENTRY * dllShadeModel )(GLenum mode);
static void ( APIENTRY * dllStencilFunc )(GLenum func, GLint ref, GLuint mask);
static void ( APIENTRY * dllStencilMask )(GLuint mask);
static void ( APIENTRY * dllStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
static void ( APIENTRY * dllTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllTexEnvf )(GLenum target, GLenum pname, GLfloat param);
static void ( APIENTRY * dllTexEnvi )(GLenum target, GLenum pname, GLint param);
static void ( APIENTRY * dllTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllTexParameterf )(GLenum target, GLenum pname, GLfloat param);
static void ( APIENTRY * dllTexParameteri )(GLenum target, GLenum pname, GLint param);
static void ( APIENTRY * dllTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllTranslatef )(GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllViewport )(GLint x, GLint y, GLsizei width, GLsizei height);


/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.
*/
void QGL_Shutdown( void )
{
	if (qglState->vbo != 0) {
		glDeleteBuffers(1, &qglState->vbo);
		qglState->vbo = 0;
	}

	qglAlphaFunc                 = NULL;
	qglBindTexture               = NULL;
	qglBlendFunc                 = NULL;
	qglClear                     = NULL;
	qglClearColor                = NULL;
	qglClearDepth                = NULL;
	qglClearStencil              = NULL;
	qglColorMask                 = NULL;
	qglColorPointer              = NULL;
	qglCullFace                  = NULL;
	qglDeleteTextures            = NULL;
	qglDepthFunc                 = NULL;
	qglDepthMask                 = NULL;
	qglDepthRange                = NULL;
	qglDisable                   = NULL;
	qglDrawArrays                = NULL;
	qglDrawElements              = NULL;
	qglEnable                    = NULL;
	qglFinish                    = NULL;
	qglFlush                     = NULL;
	qglFrontFace                 = NULL;
	qglGenTextures               = NULL;
	qglGetBooleanv               = NULL;
	qglGetError                  = NULL;
	qglGetIntegerv               = NULL;
	qglGetString                 = NULL;
	qglHint                      = NULL;
	qglIsEnabled                 = NULL;
	qglIsTexture                 = NULL;
	qglLineWidth                 = NULL;
	qglPolygonMode               = NULL;
	qglPolygonOffset             = NULL;
	qglReadPixels                = NULL;
	qglScissor                   = NULL;
	qglShadeModel                = NULL;
	qglStencilFunc               = NULL;
	qglStencilMask               = NULL;
	qglStencilOp                 = NULL;
	qglTexCoordPointer           = NULL;
	qglTexEnvf                   = NULL;
	qglTexEnvi                   = NULL;
	qglTexImage2D                = NULL;
	qglTexParameterf             = NULL;
	qglTexParameteri             = NULL;
	qglTexSubImage2D             = NULL;
	qglVertexPointer             = NULL;
	qglViewport                  = NULL;
}

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to 
** the appropriate GL stuff.  In Windows this means doing a 
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
** 
*/
qboolean QGL_Init( const char *dllname )
{
	qglState = (QGLState *) malloc(sizeof(QGLState));
	// qglState = new QGLState;   ... oh shit, not C++. sigh ...
	memset(qglState, 0, sizeof(QGLState));
	qglState->numVertices = 1024;
	qglState->vertices = (Vertex *) malloc(qglState->numVertices * sizeof(Vertex));
	memset(qglState->vertices, 0, qglState->numVertices * sizeof(Vertex));

	qglAlphaFunc                 = dllAlphaFunc = glAlphaFunc;
	qglBindTexture               = dllBindTexture = glBindTexture;
	qglBlendFunc                 = dllBlendFunc = glBlendFunc;
	qglClear                     = dllClear = glClear;
	qglClearColor                = dllClearColor = glClearColor;
	qglClearDepth                = dllClearDepth = glClearDepth;
	qglClearStencil              = dllClearStencil = glClearStencil;
	qglColorMask                 = dllColorMask = glColorMask;
	qglColorPointer              = dllColorPointer = glColorPointer;
	qglCullFace                  = dllCullFace = glCullFace;
	qglDeleteTextures            = dllDeleteTextures = glDeleteTextures;
	qglDepthFunc                 = dllDepthFunc = glDepthFunc;
	qglDepthMask                 = dllDepthMask = glDepthMask;
	qglDepthRange                = dllDepthRange = glDepthRange;
	qglDisable                   = dllDisable = glDisable;
	qglDrawArrays                = dllDrawArrays = glDrawArrays;
	qglDrawElements              = dllDrawElements = glDrawElements;
	qglEnable                    = 	dllEnable                    = glEnable;
	qglFinish                    = 	dllFinish                    = glFinish;
	qglFlush                     = 	dllFlush                     = glFlush;
	qglFrontFace                 = 	dllFrontFace                 = glFrontFace;
	qglGenTextures               = 	dllGenTextures               = glGenTextures;
	qglGetBooleanv               = 	dllGetBooleanv               = glGetBooleanv;
	qglGetError                  = 	dllGetError                  = glGetError;
	qglGetIntegerv               = 	dllGetIntegerv               = glGetIntegerv;
	qglGetString                 = 	dllGetString                 = glGetString;
	qglHint                      = 	dllHint                      = glHint;
	qglIsEnabled                 = 	dllIsEnabled                 = glIsEnabled;
	qglIsTexture                 = 	dllIsTexture                 = glIsTexture;
	qglLineWidth                 = 	dllLineWidth                 = glLineWidth;
	qglPolygonMode               = 	dllPolygonMode               = glPolygonMode;
	qglPolygonOffset             = 	dllPolygonOffset             = glPolygonOffset;
	qglReadPixels                = 	dllReadPixels                = glReadPixels;
	qglScissor                   = 	dllScissor                   = glScissor;
	qglShadeModel                = 	dllShadeModel                = glShadeModel;
	qglStencilFunc               = 	dllStencilFunc               = glStencilFunc;
	qglStencilMask               = 	dllStencilMask               = glStencilMask;
	qglStencilOp                 = 	dllStencilOp                 = glStencilOp;
	qglTexCoordPointer           = 	dllTexCoordPointer           = glTexCoordPointer;
	qglTexEnvf                   = 	dllTexEnvf                   = glTexEnvf;
	qglTexEnvi                   = 	dllTexEnvi                   = glTexEnvi;
	qglTexImage2D                = 	dllTexImage2D                = glTexImage2D;
	qglTexParameterf             = 	dllTexParameterf             = glTexParameterf;
	qglTexParameteri             = 	dllTexParameteri             = glTexParameteri;
	qglTexSubImage2D             = 	dllTexSubImage2D             = glTexSubImage2D;
	qglVertexPointer             = 	dllVertexPointer             = glVertexPointer;
	qglViewport                  = 	dllViewport                  = glViewport;

	return true;
}

void GLimp_EnableLogging( qboolean enable )
{
		qglAlphaFunc                 = dllAlphaFunc;
		qglBindTexture               = dllBindTexture;
		qglBlendFunc                 = dllBlendFunc;
		qglClear                     = dllClear;
		qglClearColor                = dllClearColor;
		qglClearDepth                = dllClearDepth;
		qglClearStencil              = dllClearStencil;
		qglColorMask                 = dllColorMask;
		qglColorPointer              = dllColorPointer;
		qglCullFace                  = dllCullFace;
		qglDeleteTextures            = dllDeleteTextures ;
		qglDepthFunc                 = dllDepthFunc ;
		qglDepthMask                 = dllDepthMask ;
		qglDepthRange                = dllDepthRange ;
		qglDisable                   = dllDisable ;
		qglDrawArrays                = dllDrawArrays ;
		qglDrawElements              = dllDrawElements ;
		qglEnable                    = 	dllEnable                    ;
		qglFinish                    = 	dllFinish                    ;
		qglFlush                     = 	dllFlush                     ;
		qglFrontFace                 = 	dllFrontFace                 ;
		qglGenTextures               = 	dllGenTextures               ;
		qglGetBooleanv               = 	dllGetBooleanv               ;
		qglGetError                  = 	dllGetError                  ;
		qglGetIntegerv               = 	dllGetIntegerv               ;
		qglGetString                 = 	dllGetString                 ;
		qglHint                      = 	dllHint                      ;
		qglIsEnabled                 = 	dllIsEnabled                 ;
		qglIsTexture                 = 	dllIsTexture                 ;
		qglLineWidth                 = 	dllLineWidth                 ;
		qglPolygonMode               = 	dllPolygonMode               ;
		qglPolygonOffset             = 	dllPolygonOffset             ;
		qglReadPixels                = 	dllReadPixels                ;
		qglScissor                   = 	dllScissor                   ;
		qglShadeModel                = 	dllShadeModel                ;
		qglStencilFunc               = 	dllStencilFunc               ;
		qglStencilMask               = 	dllStencilMask               ;
		qglStencilOp                 = 	dllStencilOp                 ;
		qglTexCoordPointer           = 	dllTexCoordPointer           ;
		qglTexEnvf                   = 	dllTexEnvf                   ;
		qglTexEnvi                   = 	dllTexEnvi                   ;
		qglTexImage2D                = 	dllTexImage2D                ;
		qglTexParameterf             = 	dllTexParameterf             ;
		qglTexParameteri             = 	dllTexParameteri             ;
		qglTexSubImage2D             = 	dllTexSubImage2D             ;
		qglVertexPointer             = 	dllVertexPointer             ;
		qglViewport                  = 	dllViewport                  ;
}


// add a new vertex to vertices array
// resize if necessary
static void pushVertex(const Vertex *v) {
	if (qglState->numVertices == qglState->usedVertices) {
		// resize needed
		size_t oldVerticesSize = qglState->numVertices * sizeof(Vertex);

		qglState->numVertices *= 2;
		Vertex *newVertices = (Vertex *) malloc(qglState->numVertices * sizeof(Vertex));
		memset(newVertices, 0, qglState->numVertices * sizeof(Vertex));
		memcpy(newVertices, qglState->vertices, oldVerticesSize);

		free(qglState->vertices);
		qglState->vertices = newVertices;
	}

	memcpy(&qglState->vertices[qglState->usedVertices++], v, sizeof(Vertex));
}


void qglColor3f(GLfloat red, GLfloat green, GLfloat blue) {
	qglColor4f(red, green, blue, 1.0f);
}


#define CLAMP(x) if ((x) > 1.0f) { (x) = 1.0f; } else if ((x) < 0.0f) { (x) = 0.0f; }


void qglColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	CLAMP(red)
	CLAMP(green)
	CLAMP(blue)
	CLAMP(alpha)

	uint32_t r = red * 255, g = green * 255, b = blue * 255, a = alpha * 255;

	// TODO: big-endian, if anyone cares
	uint32_t c =
		  (r <<  0)
		| (g <<  8)
		| (b << 16)
		| (a << 24);
	qglState->currentVertex.color = c;
}


#undef CLAMP


void qglVertex2f(GLfloat x, GLfloat y) {
	qglVertex3f(x, y, 0.0f);
}


void qglVertex3f(GLfloat x, GLfloat y, GLfloat z) {
	qglState->currentVertex.pos[0] = x;
	qglState->currentVertex.pos[1] = y;
	qglState->currentVertex.pos[2] = z;

	pushVertex(&qglState->currentVertex);
}


void qglMTexCoord2f(GLenum tex, GLfloat s, GLfloat t) {
	if (tex == GL_TEXTURE0) {
		qglState->currentVertex.tex0[0] = s;
		qglState->currentVertex.tex0[1] = t;
	} else {
		qglState->currentVertex.tex1[0] = s;
		qglState->currentVertex.tex1[1] = t;
	}
}


void qglBegin(GLenum mode) {
	qglState->usedVertices = 0;
	qglState->primitive = mode;
}


void qglEnd(void) {
	if (qglState->vbo == 0) {
		// can't be called in QGL_Init, GL context doesn't exist there
		glGenBuffers(1, &qglState->vbo);
		glBindBuffer(GL_ARRAY_BUFFER, qglState->vbo);
	}

	glBufferData(GL_ARRAY_BUFFER, qglState->usedVertices * sizeof(Vertex), &qglState->vertices[0], GL_DYNAMIC_DRAW);

	qglVertexPointer(3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, pos));
	qglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), offsetof(Vertex, color));

	if (qglState->activeTexture == 0) {
		qglTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, tex0));

		glClientActiveTexture(GL_TEXTURE1);
		qglState->activeTexture = 1;
		qglTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, tex1));
	} else {
		qglTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, tex1));

		glClientActiveTexture(GL_TEXTURE0);
		qglState->activeTexture = 0;
		qglTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, tex0));
	}

	if (qglState->mvMatrixDirty) {
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(qglState->mvMatrices[qglState->mvMatrixTop]);
		qglState->mvMatrixDirty = false;
	}

	if (qglState->projMatrixDirty) {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(qglState->projMatrices[qglState->projMatrixTop]);
		qglState->projMatrixDirty = false;
	}

	glDrawArrays(qglState->primitive, 0, qglState->usedVertices);

	qglState->primitive = GL_NONE;
}


void qglMatrixMode(GLenum mode) {
	qglState->matrixMode = mode;
}


static void identityMatrix(float *matrix) {
	memset(matrix, 0, sizeof(float) * 16);
	matrix[0 * 4 + 0] = 1.0f;
	matrix[1 * 4 + 1] = 1.0f;
	matrix[2 * 4 + 2] = 1.0f;
	matrix[3 * 4 + 3] = 1.0f;
}


static void multMatrices(float *target, const float *left, const float *right) {
	// target and left/right must not alias
	// should probably put some __restrict__ on this
	assert(target != left);
	assert(target != right);

	target[0 * 4 + 0] = right[0 * 4 + 0] * left[0 * 4 + 0] + right[0 * 4 + 1] * left[1 * 4 + 0] + right[0 * 4 + 2] * left[2 * 4 + 0] + right[0 * 4 + 3] * left[3 * 4 + 0];
	target[0 * 4 + 1] = right[0 * 4 + 0] * left[0 * 4 + 1] + right[0 * 4 + 1] * left[1 * 4 + 1] + right[0 * 4 + 2] * left[2 * 4 + 1] + right[0 * 4 + 3] * left[3 * 4 + 1];
	target[0 * 4 + 2] = right[0 * 4 + 0] * left[0 * 4 + 2] + right[0 * 4 + 1] * left[1 * 4 + 2] + right[0 * 4 + 2] * left[2 * 4 + 2] + right[0 * 4 + 3] * left[3 * 4 + 2];
	target[0 * 4 + 3] = right[0 * 4 + 0] * left[0 * 4 + 3] + right[0 * 4 + 1] * left[1 * 4 + 3] + right[0 * 4 + 2] * left[2 * 4 + 3] + right[0 * 4 + 3] * left[3 * 4 + 3];

	target[1 * 4 + 0] = right[1 * 4 + 0] * left[0 * 4 + 0] + right[1 * 4 + 1] * left[1 * 4 + 0] + right[1 * 4 + 2] * left[2 * 4 + 0] + right[1 * 4 + 3] * left[3 * 4 + 0];
	target[1 * 4 + 1] = right[1 * 4 + 0] * left[0 * 4 + 1] + right[1 * 4 + 1] * left[1 * 4 + 1] + right[1 * 4 + 2] * left[2 * 4 + 1] + right[1 * 4 + 3] * left[3 * 4 + 1];
	target[1 * 4 + 2] = right[1 * 4 + 0] * left[0 * 4 + 2] + right[1 * 4 + 1] * left[1 * 4 + 2] + right[1 * 4 + 2] * left[2 * 4 + 2] + right[1 * 4 + 3] * left[3 * 4 + 2];
	target[1 * 4 + 3] = right[1 * 4 + 0] * left[0 * 4 + 3] + right[1 * 4 + 1] * left[1 * 4 + 3] + right[1 * 4 + 2] * left[2 * 4 + 3] + right[1 * 4 + 3] * left[3 * 4 + 3];
	
	target[2 * 4 + 0] = right[2 * 4 + 0] * left[0 * 4 + 0] + right[2 * 4 + 1] * left[1 * 4 + 0] + right[2 * 4 + 2] * left[2 * 4 + 0] + right[2 * 4 + 3] * left[3 * 4 + 0];
	target[2 * 4 + 1] = right[2 * 4 + 0] * left[0 * 4 + 1] + right[2 * 4 + 1] * left[1 * 4 + 1] + right[2 * 4 + 2] * left[2 * 4 + 1] + right[2 * 4 + 3] * left[3 * 4 + 1];
	target[2 * 4 + 2] = right[2 * 4 + 0] * left[0 * 4 + 2] + right[2 * 4 + 1] * left[1 * 4 + 2] + right[2 * 4 + 2] * left[2 * 4 + 2] + right[2 * 4 + 3] * left[3 * 4 + 2];
	target[2 * 4 + 3] = right[2 * 4 + 0] * left[0 * 4 + 3] + right[2 * 4 + 1] * left[1 * 4 + 3] + right[2 * 4 + 2] * left[2 * 4 + 3] + right[2 * 4 + 3] * left[3 * 4 + 3];

	target[3 * 4 + 0] = right[3 * 4 + 0] * left[0 * 4 + 0] + right[3 * 4 + 1] * left[1 * 4 + 0] + right[3 * 4 + 2] * left[2 * 4 + 0] + right[3 * 4 + 3] * left[3 * 4 + 0];
	target[3 * 4 + 1] = right[3 * 4 + 0] * left[0 * 4 + 1] + right[3 * 4 + 1] * left[1 * 4 + 1] + right[3 * 4 + 2] * left[2 * 4 + 1] + right[3 * 4 + 3] * left[3 * 4 + 1];
	target[3 * 4 + 2] = right[3 * 4 + 0] * left[0 * 4 + 2] + right[3 * 4 + 1] * left[1 * 4 + 2] + right[3 * 4 + 2] * left[2 * 4 + 2] + right[3 * 4 + 3] * left[3 * 4 + 2];
	target[3 * 4 + 3] = right[3 * 4 + 0] * left[0 * 4 + 3] + right[3 * 4 + 1] * left[1 * 4 + 3] + right[3 * 4 + 2] * left[2 * 4 + 3] + right[3 * 4 + 3] * left[3 * 4 + 3];
}


void qglLoadIdentity(void) {
	float idM[16];
	identityMatrix(idM);

	qglLoadMatrixf(idM);
}


void qglFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
	float mat[16];
	memset(mat, 0, sizeof(float) * 16);

	mat[0 * 4 + 0] = 2 * zNear / (right - left);

	mat[1 * 4 + 1] = 2 * zNear / (top - bottom);

	mat[2 * 4 + 0] = (right + left) / (right - left);
	mat[2 * 4 + 1] = (top + bottom) / (top - bottom);
	mat[2 * 4 + 2] = -(zFar + zNear) / (zFar - zNear);
	mat[2 * 4 + 3] = -1.0f;

	mat[3 * 4 + 2] = -(2 * zFar * zNear) / (zFar - zNear);

	// should really be MultMatrix but since we always load identity before...
	qglLoadMatrixf(mat);
}


void qglOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
	float mat[16];
	memset(mat, 0, sizeof(float) * 16);

	mat[0 * 4 + 0] = 2  / (right - left);

	mat[1 * 4 + 1] = 2  / (top - bottom);

	mat[2 * 4 + 2] = -2 / (zFar - zNear);

	mat[3 * 4 + 0] = -(right + left) / (right - left);
	mat[3 * 4 + 1] = -(top + bottom) / (top - bottom);
	mat[3 * 4 + 2] = -(zFar + zNear) / (zFar - zNear);
	mat[3 * 4 + 3] = 1.0f;

	// should really be MultMatrix but since we always load identity before...
	qglLoadMatrixf(mat);
}


void qglLoadMatrixf(const GLfloat *m) {
	float *targetMat = NULL;
	if (qglState->matrixMode == GL_MODELVIEW) {
		targetMat = qglState->mvMatrices[qglState->mvMatrixTop];
		qglState->mvMatrixDirty = true;
	} else if (qglState->matrixMode == GL_PROJECTION) {
		targetMat = qglState->projMatrices[qglState->projMatrixTop];
		qglState->projMatrixDirty = true;
	} else {
		assert(false);
	}

	memcpy(targetMat, m, sizeof(float) * 16);
}


void qglMultMatrixf(const GLfloat *m) {
	float *targetMat = NULL;
	if (qglState->matrixMode == GL_MODELVIEW) {
		targetMat = qglState->mvMatrices[qglState->mvMatrixTop];
		qglState->mvMatrixDirty = true;
	} else if (qglState->matrixMode == GL_PROJECTION) {
		targetMat = qglState->projMatrices[qglState->projMatrixTop];
		qglState->projMatrixDirty = true;
	} else {
		assert(false);
	}

	float mat[16];
	multMatrices(mat, targetMat, m);
	memcpy(targetMat, mat, sizeof(float) * 16);
}


void qglRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
	float mat[16];
	memset(mat, 0, sizeof(float) * 16);

	// someone calls this with zero vector
	// add FLT_MIN to prevent division by zero
	float norm = 1.0f / (sqrtf(x*x + y*y + z*z) + FLT_MIN);
	x *= norm;
	y *= norm;
	z *= norm;

	float c = cosf(angle * M_PI / 180.0f);
	float s = sinf(angle * M_PI / 180.0f);

	mat[0 * 4 + 0] = x * x * (1 - c) + c;
	mat[0 * 4 + 1] = y * x * (1 - c) + z * s;
	mat[0 * 4 + 2] = x * z * (1 - c) - y * s;

	mat[1 * 4 + 0] = x * y * (1 - c) - z * s;
	mat[1 * 4 + 1] = y * y * (1 - c) + c;
	mat[1 * 4 + 2] = y * z * (1 - c) + x * s;

	mat[2 * 4 + 0] = x * z * (1 - c) + y * s;
	mat[2 * 4 + 1] = y * z * (1 - c) - x * s;
	mat[2 * 4 + 2] = z * z * (1 - c) + c;

	mat[3 * 4 + 3] = 1.0f;

	qglMultMatrixf(mat);
}


void qglScalef(GLfloat x, GLfloat y, GLfloat z) {
	float mat[16];
	memset(mat, 0, sizeof(float) * 16);

	mat[0 * 4 + 0] = x;

	mat[1 * 4 + 1] = y;

	mat[2 * 4 + 2] = z;

	mat[3 * 4 + 3] = 1.0f;

	qglMultMatrixf(mat);
}


void qglTranslatef(GLfloat x, GLfloat y, GLfloat z) {
	float mat[16];
	identityMatrix(mat);

	mat[3 * 4 + 0] = x;
	mat[3 * 4 + 1] = y;
	mat[3 * 4 + 2] = z;

	qglMultMatrixf(mat);
}


void qglPopMatrix(void) {
	float *targetMat = NULL;
	if (qglState->matrixMode == GL_MODELVIEW) {
		qglState->mvMatrixTop--;

		assert(qglState->mvMatrixTop >= 0 && qglState->mvMatrixTop <= NUMMATRICES);
		targetMat = qglState->mvMatrices[qglState->mvMatrixTop];
		qglState->mvMatrixDirty = true;
	} else if (qglState->matrixMode == GL_PROJECTION) {
		qglState->projMatrixTop--;

		assert(qglState->projMatrixTop >= 0 && qglState->projMatrixTop <= NUMMATRICES);
		targetMat = qglState->projMatrices[qglState->projMatrixTop];
		qglState->projMatrixDirty = true;
	} else {
		assert(false);
	}
}


void qglPushMatrix(void) {
	if (qglState->matrixMode == GL_MODELVIEW) {
		qglState->mvMatrixTop++;

		assert(qglState->mvMatrixTop >= 0 && qglState->mvMatrixTop <= NUMMATRICES);

		memcpy(qglState->mvMatrices[qglState->mvMatrixTop], qglState->mvMatrices[qglState->mvMatrixTop - 1], sizeof(float) * 16);
	} else if (qglState->matrixMode == GL_PROJECTION) {
		qglState->projMatrixTop++;

		assert(qglState->projMatrixTop >= 0 && qglState->projMatrixTop <= NUMMATRICES);

		memcpy(qglState->projMatrices[qglState->projMatrixTop], qglState->projMatrices[qglState->projMatrixTop - 1], sizeof(float) * 16);
	} else {
		assert(false);
	}
}


void qglGetFloatv(GLenum pname, GLfloat *params) {
	assert(pname == GL_MODELVIEW_MATRIX);

	memcpy(params, qglState->mvMatrices[qglState->mvMatrixTop], sizeof(float) * 16);
}
