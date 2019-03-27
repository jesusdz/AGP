#ifndef GLFUNCTIONS_H
#define GLFUNCTIONS_H

#include <QOpenGLFunctions_3_3_Core>
#include <QString>


extern QOpenGLFunctions_3_3_Core *gl;


class OpenGLErrorGuard
{
    public:

    OpenGLErrorGuard(const char *message, ...);

    ~OpenGLErrorGuard();

    static void checkGLError(const char *around, ...);

    QString message;
};


class OpenGLState
{
public:

    bool depthTest = false;
    bool depthWrite = true;
    GLenum depthFunc = GL_LESS;
    bool blending = false;
    GLenum blendFuncSrc = GL_SRC_ALPHA;
    GLenum blendFuncDst = GL_ONE_MINUS_SRC_ALPHA;

    void apply();

    static OpenGLState currentState;

    static void initialize();
    static void reset();
};


#endif // GLFUNCTIONS_H
