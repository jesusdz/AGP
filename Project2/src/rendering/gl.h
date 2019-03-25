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

#endif // GLFUNCTIONS_H
