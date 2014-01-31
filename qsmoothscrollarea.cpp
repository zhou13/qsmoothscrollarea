#include <QWheelEvent>
#include <QApplication>
#include <QScrollBar>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QQueue>
#include <qmath.h>

#include "qsmoothscrollarea.h"

QSmoothScrollArea::QSmoothScrollArea(QWidget *parent) :
    QScrollArea(parent)
{
    lastWheelEvent = 0;
    smoothMoveTimer = new QTimer(this);
    connect(smoothMoveTimer, SIGNAL(timeout()), this, SLOT(slotSmoothMove()));

    m_fps = 60;
    m_duration = 400;
    m_smoothMode = COSINE;
    m_acceleration = 2.5;

    m_smallStepModifier = Qt::SHIFT;
    m_smallStepRatio = 1.0 / 5.0;
    m_bigStepModifier = Qt::ALT;
    m_bigStepRatio = 5.0;
}

int QSmoothScrollArea::fps()
{
    return m_fps;
}

void QSmoothScrollArea::setFps(int fps)
{
    m_fps = fps;
}

int QSmoothScrollArea::duration()
{
    return m_duration;
}

void QSmoothScrollArea::setDuration(int mesc)
{
    m_duration = mesc;
}

QSmoothScrollArea::SmoothMode QSmoothScrollArea::smoothMode()
{
    return m_smoothMode;
}

void QSmoothScrollArea::setSmoothMode(QSmoothScrollArea::SmoothMode mode)
{
    m_smoothMode = mode;
}

double QSmoothScrollArea::acceration()
{
    return m_acceleration;
}

void QSmoothScrollArea::setAcceration(double acceleration)
{
    m_acceleration = acceleration;
}

double QSmoothScrollArea::smallStepRatio()
{
    return m_smallStepRatio;
}

void QSmoothScrollArea::setSmallStepRatio(double smallStepRatio)
{
    m_smallStepRatio = smallStepRatio;
}

double QSmoothScrollArea::bigStepRatio()
{
    return m_bigStepRatio;
}

void QSmoothScrollArea::setBigStepRatio(double bigStepRatio)
{
    m_bigStepRatio = bigStepRatio;
}

Qt::Modifier QSmoothScrollArea::smallStepModifier()
{
    return m_smallStepModifier;
}

void QSmoothScrollArea::setSmallStepModifier(
        Qt::Modifier smallStepModifier)
{
    m_smallStepModifier = smallStepModifier;
}

Qt::Modifier QSmoothScrollArea::bigStepModifier()
{
    return m_bigStepModifier;
}

void QSmoothScrollArea::setbigStepModifier(
        Qt::Modifier bigStepModifier)
{
    m_bigStepModifier = bigStepModifier;
}

void QSmoothScrollArea::wheelEvent(QWheelEvent *e)
{
    if (m_smoothMode == NO_SMOOTH) {
        QScrollArea::wheelEvent(e);
        return;
    }


    // According to my experiment, a normal person is able to scroll his wheel
    // at the speed about 36 times per second in average.  Here we use a
    // conservative value 30: a user can achieve the maximum acceration when he
    // scrools his wheel at 30 times / second.
    static QQueue<qint64> scrollStamps;
    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    scrollStamps.enqueue(now);
    while (now - scrollStamps.front() > 500)
        scrollStamps.dequeue();
    double accerationRatio = qMin(scrollStamps.size() / 15.0, 1.0);

    if (!lastWheelEvent)
        lastWheelEvent = new QWheelEvent(*e);
    else
        *lastWheelEvent = *e;

    stepsTotal = m_fps * m_duration / 1000;
    double multiplier = 1.0;
    if (QApplication::keyboardModifiers() & smallStepModifier())
        multiplier *= smallStepRatio();
    if (QApplication::keyboardModifiers() & bigStepModifier())
        multiplier *= bigStepRatio();
    double delta = e->delta() * multiplier;
    if (acceration() > 0)
        delta += delta * acceration() * accerationRatio;

    stepsLeftQueue.push_back(qMakePair(delta, stepsTotal));
    smoothMoveTimer->start(1000 / m_fps);
}

void QSmoothScrollArea::slotSmoothMove()
{
    double totalDelta = 0;

    for (QList< QPair<double, int> >::Iterator it = stepsLeftQueue.begin();
         it != stepsLeftQueue.end(); ++it)
    {
        totalDelta += subDelta(it->first, it->second);
        --(it->second);
    }
    while (!stepsLeftQueue.empty() && stepsLeftQueue.begin()->second == 0)
        stepsLeftQueue.pop_front();

    Qt::Orientation orientation = lastWheelEvent->orientation();
    // By default, when you press ALT, QT will scroll horizontally.  But if we
    // have defined the use of ALT key, we ignore this setting since horizontal
    // scroll is not so useful in okular
    if ((bigStepModifier() & Qt::ALT) || (smallStepModifier() & Qt::ALT))
        orientation = Qt::Vertical;

    QWheelEvent e(
                lastWheelEvent->pos(),
                lastWheelEvent->globalPos(),
                qRound(totalDelta),
                lastWheelEvent->buttons(),
                0,
                orientation
    );
    if (e.orientation() == Qt::Horizontal)
        QApplication::sendEvent(horizontalScrollBar(), &e);
    else
        QApplication::sendEvent(verticalScrollBar(), &e);

    if (stepsLeftQueue.empty()) {
        smoothMoveTimer->stop();
    }
}

double QSmoothScrollArea::subDelta(double delta, int stepsLeft)
{
    Q_ASSERT(m_smoothMode != NO_SMOOTH);

    double m = stepsTotal / 2.0;
    double x = abs(stepsTotal - stepsLeft - m);

    // some mathmatical integral result.
    switch (m_smoothMode) {
    case NO_SMOOTH:
        return 0;
        break;

    case CONSTANT:
        return double(delta) / stepsTotal;
        break;

    case LINEAR:
        return 2.0*delta/stepsTotal * (m - x) / m;
        break;

    case QUADRATIC:
        return 3.0/4.0/m * (1.0 - x*x/m/m) * delta;
        break;

    case COSINE:
        return (cos(x * M_PI / m) + 1.0) / (2.0*m) * delta;
        break;
    }

    return 0;
}
