#ifndef WATERFALL_H
#define WATERFALL_H

#include <QQuickPaintedItem>
#include <QImage>
#include <QtQml/qqmlregistration.h>
#include <QVector>
#include <QPointer>

#include "core/sdr_backend.h"

class Waterfall : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SdrBackend* source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(float minDb READ minDb WRITE setMinDb NOTIFY minDbChanged)
    Q_PROPERTY(float maxDb READ maxDb WRITE setMaxDb NOTIFY maxDbChanged)
    Q_PROPERTY(int history READ history WRITE setHistory NOTIFY historyChanged)
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

    Q_INVOKABLE void clear();

public slots:
    void addLine(const QList<double>& dB);

signals:
    void sourceChanged();
    void minDbChanged();
    void maxDbChanged();
    void historyChanged();
protected:
    void paint(QPainter* painter) override;
private:
    static QRgb colormap(float t);

    void resetImage(int bins, int rows);

    QPointer<SdrBackend> m_source;
    QImage m_img;
    QRgb m_lut[256];
    float m_minDb = -100.0f;
    float m_maxDb = -40.0f;
    int m_history = 512;
};

#endif // WATERFALL_H
