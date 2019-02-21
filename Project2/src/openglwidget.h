#ifndef MYCUSTOMWIDGET_H
#define MYCUSTOMWIDGET_H

#include <QOpenGLWidget>

class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

signals:

public slots:
};

#endif // MYCUSTOMWIDGET_H
