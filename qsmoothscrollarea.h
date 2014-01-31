#ifndef QSMOOTHSCROLLAREA_H
#define QSMOOTHSCROLLAREA_H

#include <QScrollArea>
#include <QList>
#include <QPair>
#include <Qt>

class QTimer;

class QSmoothScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit QSmoothScrollArea(QWidget *parent = 0);

    enum SmoothMode {
        NO_SMOOTH,
        CONSTANT,
        LINEAR,
        QUADRATIC,
        COSINE
    };
    SmoothMode smoothMode();
    void setSmoothMode(SmoothMode mode);

    int fps();
    void setFps(int fps);

    // value in millisecond
    int duration();
    void setDuration(int mesc);

    double acceration();
    void setAcceration(double acceleration);

    double smallStepRatio();
    void setSmallStepRatio(double smallStepRatio);

    double bigStepRatio();
    void setBigStepRatio(double bigStepRatio);

    Qt::Modifier smallStepModifier();
    void setSmallStepModifier(Qt::Modifier smallStepModifier);

    Qt::Modifier bigStepModifier();
    void setbigStepModifier(Qt::Modifier bigStepModifier);

protected:
    virtual void wheelEvent(QWheelEvent *event);

signals:

public slots:
    void slotSmoothMove();

private:
    double subDelta(double delta, int stepsLeft);

    QTimer *smoothMoveTimer;
    QWheelEvent *lastWheelEvent;

    int m_fps;
    int m_duration;
    SmoothMode m_smoothMode;

    double m_acceleration;
    double m_smallStepRatio;
    double m_bigStepRatio;
    Qt::Modifier m_smallStepModifier;
    Qt::Modifier m_bigStepModifier;

    int stepsTotal;
    QList< QPair<double, int> > stepsLeftQueue;
};

#endif // QSMOOTHSCROLLAREA_H
