#include "domain/RangeGate.h"
#include "domain/WindProfile.h"
#include "services/DeviceService.h"

#include <QCoreApplication>
#include <QTimer>

#include <iostream>

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    const QString host = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("127.0.0.1");
    const int port = argc > 2 ? QString::fromLocal8Bit(argv[2]).toInt() : 5500;
    DeviceService service;
    int result = 3;

    QObject::connect(&service, &DeviceService::errorOccurred, [](const QString &message) {
        std::cerr << message.toStdString() << '\n';
    });
    QObject::connect(&service, &DeviceService::windProfileUpdated,
                     [&](WindProfile *profile) {
        if (!profile || profile->retrievalMethod() != WindRetrievalMethod::FiveBeamLeastSquares) return;
        if (profile->beamStates().size() != 5 || profile->rangeGates().size() != 30
            || profile->validGateCount() < 25 || profile->confidence() < 80.0) {
            result = 2;
            application.quit();
            return;
        }
        const RangeGate *gate = profile->rangeGates().first();
        std::cout << "scan=" << profile->sourceScanId()
                  << " u=" << gate->eastwardWindMps()
                  << " v=" << gate->northwardWindMps()
                  << " w=" << gate->upwardWindMps()
                  << " residual=" << gate->retrievalResidualMps()
                  << " confidence=" << profile->confidence()
                  << " valid=" << profile->validGateCount() << "/" << profile->gateCount() << '\n';
        result = 0;
        application.quit();
    });
    QTimer::singleShot(6000, &application, &QCoreApplication::quit);
    if (!service.connectDevice(host, port)) return 1;
    application.exec();
    return result;
}
