#ifndef MYCUSTOMWIDGET_H
#define MYCUSTOMWIDGET_H

#include <QWidget>

class MyCustomWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyCustomWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:

public slots:

private:

    void paintEvent(QPaintEvent *event) override;
};

#endif // MYCUSTOMWIDGET_H
