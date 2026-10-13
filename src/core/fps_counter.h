#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <QObject>
#include <QElapsedTimer>
#include <QEvent>

class FpsCounter : public QObject {
    QElapsedTimer timer;
    int frames = 0;
    double _fps = 0;
public:
    FpsCounter(QObject *parent) : QObject(parent) { timer.start(); }

    void tick() {
        ++frames;

        if (timer.elapsed() >= 1000) {
            _fps = frames * 1000.0 / timer.restart();

            frames = 0;
        }
    }

    double fps() const { return this->_fps; }
};

#endif // FPS_COUNTER_H
