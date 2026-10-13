#ifndef WATERFALL_H
#define WATERFALL_H

#include <QQuickPaintedItem>
#include <QImage>
#include <QtQml/qqmlregistration.h>
#include <QVector>
#include <QPointer>

#include "core/sdr_backend.h"
#include "core/fps_counter.h"

struct Stop { float r, g, b; };

class Waterfall : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SdrBackend* source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float minDb READ minDb WRITE setMinDb NOTIFY minDbChanged)
    Q_PROPERTY(float maxDb READ maxDb WRITE setMaxDb NOTIFY maxDbChanged)
    Q_PROPERTY(int history READ history WRITE setHistory NOTIFY historyChanged)
    Q_PROPERTY(QString direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(bool useNarrow READ useNarrow WRITE setUseNarrow NOTIFY useNarrowChanged)

public:
    explicit Waterfall(QQuickItem* parent = nullptr);

    SdrBackend* source() const { return m_source; }

    void setSource(SdrBackend* s);

    float minDb() const { return m_minDb; }

    void setMinDb(float v);

    float maxDb() const { return m_maxDb; }

    void setMaxDb(float v);

    int history() const { return m_history; }

    void setHistory(int rows);

    QString direction() const { return _direction; }

    void setDirection(QString direction);

    // quint32 fromBin() const { return _fromBin; }

    // void setFromBin(quint32 fromBin);

    // quint32 toBin() const { return _toBin; }

    // void setToBin(quint32 toBin);

    bool useNarrow() const { return _useNarrow; }

    void setUseNarrow(bool useNarrow);

    Q_INVOKABLE void clear();

public slots:
    void addLine(const QList<double>& dB);

signals:
    void sourceChanged();
    void minDbChanged();
    void maxDbChanged();
    void historyChanged();
    void directionChanged();
    // void fromBinChanged();
    // void toBinChanged();
    void useNarrowChanged();
protected:
    void paint(QPainter* painter) override;
private:
    static QRgb colormap(float t);

    void resetImage(int bins, int rows);

    FpsCounter* fpsCounter = nullptr;
    QPointer<SdrBackend> m_source;
    QImage m_img;
    QRgb m_lut[256];
    QString _direction = "vertical";
    // quint32 _fromBin = 0;
    // quint32 _toBin = 0;
    bool _useNarrow = false;

    float m_minDb = -100.0f;
    float m_maxDb = -40.0f;
    int m_history = 512;

    void reconnectSource();

    QMetaObject::Connection m_conn;   // the one connection we own

    static constexpr Stop kStops[] = {
                                       {  0,   0,   0}, {  0,   0,  96}, {  0,  64, 192}, {  0, 192, 192}, { 64, 224,  64},
                                       {255, 255,   0}, {255, 128,   0}, {255,   0,   0}, {255, 255, 255},
                                       };
};

#endif // WATERFALL_H
