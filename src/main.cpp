#include <QGuiApplication>
#include <QQmlApplicationEngine>
// #include <thread>

// #include "fftw3.h"

int main(int argc, char *argv[])
{
    // fftwf_init_threads();
    // fftwf_make_planner_thread_safe();
    // fftwf_plan_with_nthreads(int(std::thread::hardware_concurrency()));

    int rc = 0;
    {
        QGuiApplication app(argc, argv);

        QQmlApplicationEngine engine;
        QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);
        engine.loadFromModule("sdr_qt", "Main");

        rc = QGuiApplication::exec();
    }

    // fftwf_cleanup_threads();

    return rc;
}