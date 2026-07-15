#include "ui/widgets/WindVectorFieldWidget.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QImage>
#include <QPixmap>
#include <QtMath>

#include <cmath>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WindVectorFieldWidget widget;
    widget.resize(1280, 520);

    QVector<WindVectorFieldWidget::ProfileLevel> levels;
    levels.reserve(30);
    for (int gate = 0; gate < 30; ++gate) {
        const double height = (gate + 1) * 10.0;
        const double eastward = 4.0 + 0.015 * height;
        const double northward = 2.5 + 0.9 * qSin(height / 75.0);
        const double upward = 0.35 * qSin(height / 42.0);
        QVector<double> radial;
        radial.reserve(5);
        const double azimuths[] = {0.0, 45.0, 135.0, 225.0, 315.0};
        const double elevations[] = {90.0, 75.0, 75.0, 75.0, 75.0};
        for (int beam = 0; beam < 5; ++beam) {
            const double azimuth = qDegreesToRadians(azimuths[beam]);
            const double elevation = qDegreesToRadians(elevations[beam]);
            radial.append(eastward * qCos(elevation) * qSin(azimuth)
                          + northward * qCos(elevation) * qCos(azimuth)
                          + upward * qSin(elevation));
        }
        levels.append({height, eastward, northward, upward,
                       gate % 8 == 0 ? 42.0 : 86.0, radial});
    }
    QElapsedTimer renderTimer;
    renderTimer.start();
    widget.setWindProfile(levels, QStringLiteral("离屏渲染校验 / 五波束 WLS"));
    widget.show();
    app.processEvents();

    const QPixmap pixmap = widget.grab();
    const qint64 renderElapsedMs = renderTimer.elapsed();
    const QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
    int coloredPixels = 0;
    for (int y = 0; y < image.height(); y += 3) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); x += 3) {
            const QColor color(line[x]);
            if (color.red() > 45 && color.green() > 45 && color.blue() > 45
                && qAbs(color.red() - color.blue()) > 18) {
                ++coloredPixels;
            }
        }
    }

    const QString output = QDir(QCoreApplication::applicationDirPath())
                               .absoluteFilePath(QStringLiteral("wind-curtain-render-test.png"));
    if (!pixmap.save(output) || coloredPixels < 800 || renderElapsedMs > 800) {
        std::cerr << "Wind curtain render validation failed: coloredPixels=" << coloredPixels
                  << ", renderMs=" << renderElapsedMs << '\n';
        return 1;
    }
    std::cout << "Wind curtain render saved to " << output.toStdString()
              << ", coloredPixels=" << coloredPixels << '\n';
    std::cout << "Wind curtain first render=" << renderElapsedMs << " ms\n";
    return 0;
}
