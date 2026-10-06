#include "waterfall.h"

#include <QPainter>

Waterfall::Waterfall(QQuickItem* parent) : QQuickPaintedItem(parent) {
    // Pre-computed colormap
    for (int i = 0; i < 256; ++i)
        m_lut[i] = colormap(i / 255.0f);

    resetImage(2048, m_history);

    setOpaquePainting(true);
}

void Waterfall::setSource(SdrBackend* s) {
    if (m_source == s) return;

    if (m_source) {
        disconnect(m_source, nullptr, this, nullptr);
    }

    m_source = s;

    if (m_source) {
        connect(m_source, &SdrBackend::spectrum, this, &Waterfall::addLine);
    }

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

void Waterfall::clear() {
    m_img.fill(Qt::black);

    update();
}

void Waterfall::resetImage(int bins, int rows)
{
    m_img = QImage(bins, rows, QImage::Format_RGB32);
    m_img.fill(Qt::black);
}

void Waterfall::addLine(const QList<double>& dB)
{
    if (dB.isEmpty())
        return;

    // FFT size changed (or first line): rebuild the history at the new width
    if (dB.size() != m_img.width())
        resetImage(int(dB.size()), m_history);

    const int w   = m_img.width();
    const int h   = m_img.height();
    const int bpl = m_img.bytesPerLine();

    // Scroll history down one row: rows 0..h-2 → 1..h-1 (overlapping, so memmove)
    if (h > 1)
        std::memmove(m_img.scanLine(1), m_img.scanLine(0), size_t(bpl) * (h - 1));

    // Newest spectrum goes into row 0
    const float range = std::max(1e-3f, m_maxDb - m_minDb);   // guard against min >= max

    auto* row = reinterpret_cast<QRgb*>(m_img.scanLine(0));

    for (int x = 0; x < w; ++x) {
        const float t = (dB[x] - m_minDb) / range;
        const int idx = std::clamp(int(t * 255.0f), 0, 255);
        row[x] = m_lut[idx];
    }

    update();   // schedules paint() on the next frame
}

void Waterfall::paint(QPainter* painter)
{
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