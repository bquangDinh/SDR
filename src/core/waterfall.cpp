#include "waterfall.h"

#include <QPainter>

void Waterfall::reconnectSource()
{
    QObject::disconnect(m_conn);       // drop only our spectrum connection (no-op if none)

    if (!m_source)
        return;

    m_conn = _useNarrow
                 ? connect(m_source, &SdrBackend::narrowSpectrum, this, &Waterfall::addLine)
                 : connect(m_source, &SdrBackend::spectrum,       this, &Waterfall::addLine);
}

Waterfall::Waterfall(QQuickItem* parent) : QQuickPaintedItem(parent) {
    // Pre-computed colormap
    for (int i = 0; i < 256; ++i)
        m_lut[i] = colormap(i / 255.0f);

    resetImage(2048, m_history);

    setOpaquePainting(true);
}

void Waterfall::setSource(SdrBackend* s) {
    if (m_source == s) return;

    m_source = s;

    reconnectSource();

    emit sourceChanged();
}

void Waterfall::setMinDb(float v)
{
    if (qFuzzyCompare(m_minDb, v))
        return;

    m_minDb = v;

    emit minDbChanged();
}

void Waterfall::setMaxDb(float v)
{
    if (qFuzzyCompare(m_maxDb, v))
        return;

    m_maxDb = v;

    emit maxDbChanged();
}

void Waterfall::setHistory(int rows) {
    rows = std::max(1, rows);

    if (rows == m_history) return;

    m_history = rows;

    resetImage(m_img.width(), m_history);

    emit historyChanged();

    update();
}

void Waterfall::setDirection(QString direction) {
    _direction = direction;

    emit directionChanged();
}

// void Waterfall::setFromBin(quint32 fromBin) {
//     _fromBin = fromBin;

//     emit fromBinChanged();
// }

// void Waterfall::setToBin(quint32 toBin) {
//     _toBin = toBin;

//     emit toBinChanged();
// }

void Waterfall::setUseNarrow(bool useNarrow) {
    if (_useNarrow == useNarrow) return;

    _useNarrow = useNarrow;

    reconnectSource();

    emit useNarrowChanged();
}

void Waterfall::clear() {
    m_img.fill(Qt::black);

    update();
}

void Waterfall::resetImage(int w, int h)
{
    m_img = QImage(w, h, QImage::Format_RGB32);
    m_img.fill(m_lut[0]);
}

void Waterfall::addLine(const QList<double>& dB)
{
    if (dB.isEmpty())
        return;

    const bool horizontal = (_direction == "horizontal");
    const int  bins       = int(dB.size());

    // int band = bins;

    // qDebug() << "Narrow " << _useNarrow << " from " << _fromBin << " to " << _toBin;

    // if (_useNarrow) {
    //     if ((_toBin <= _fromBin) || _toBin >= bins || _fromBin >= bins) {
    //         qDebug() << "Bins range is not valid for narrow";
    //     } else {
    //         band = _toBin - _fromBin;
    //     }
    // }

    // vertical:   bins wide × history tall  (one spectrum per row)
    // horizontal: history wide × bins tall  (one spectrum per column)
    const int wantW = horizontal ? m_history : bins;
    const int wantH = horizontal ? bins      : m_history;

    // FFT size or direction changed (or first line): rebuild the history
    if (m_img.width() != wantW || m_img.height() != wantH)
        resetImage(wantW, wantH);

    const int w = m_img.width();
    const int h = m_img.height();

    const float range = std::max(1e-3f, m_maxDb - m_minDb);   // guard against min >= max
    auto color = [&](double v) {
        const float t = (float(v) - m_minDb) / range;
        return m_lut[std::clamp(int(t * 255.0f), 0, 255)];
    };

    if (!horizontal) {
        // Scroll down one row: rows 0..h-2 → 1..h-1 (overlapping, so memmove)
        if (h > 1)
            std::memmove(m_img.scanLine(1), m_img.scanLine(0),
                         size_t(m_img.bytesPerLine()) * (h - 1));

        // Newest spectrum goes into row 0
        auto* row = reinterpret_cast<QRgb*>(m_img.scanLine(0));
        for (int x = 0; x < w; ++x) {
            row[x] = color(dB[x]);
        }
    } else {
        // Scroll right one column: columns 0..w-2 → 1..w-1, one row at a time
        for (int y = 0; y < h; ++y) {
            auto* row = reinterpret_cast<QRgb*>(m_img.scanLine(y));
            if (w > 1)
                std::memmove(row + 1, row, sizeof(QRgb) * size_t(w - 1));

            // Newest spectrum goes into column 0; flipped so high frequencies are at the top
            row[0] = color(dB[y]);
        }
    }

    update();   // schedules paint() on the next frame
}

void Waterfall::paint(QPainter* painter)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Called on the render thread while the GUI thread is blocked,
    // so reading m_img here is safe without a mutex.
    painter->drawImage(boundingRect(), m_img);
}

// ---------- colormap: black → blue → yellow → red ----------

QRgb Waterfall::colormap(float t)
{
    if (t < 0.33f) {
        const float u = t / 0.33f;
        return qRgb(0, 0, int(255 * u));
    }
    if (t < 0.66f) {
        const float u = (t - 0.33f) / 0.33f;
        return qRgb(int(255 * u), int(255 * u), int(255 * (1 - u)));
    }
    const float u = (t - 0.66f) / 0.34f;
    return qRgb(255, int(255 * (1 - u)), 0);
}