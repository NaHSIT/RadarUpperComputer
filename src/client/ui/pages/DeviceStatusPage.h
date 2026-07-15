#ifndef DEVICESTATUSPAGE_H
#define DEVICESTATUSPAGE_H

#include <QWidget>

class BeamPage;
class DataQualityPage;
class DeviceHealthPage;
class SpectrumPage;

class DeviceStatusPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceStatusPage(QWidget *parent = nullptr);
    DeviceHealthPage *healthPage() const { return m_healthPage; }
    DataQualityPage *qualityPage() const { return m_qualityPage; }
    BeamPage *beamPage() const { return m_beamPage; }
    SpectrumPage *spectrumPage() const { return m_spectrumPage; }

private:
    DeviceHealthPage *m_healthPage;
    DataQualityPage *m_qualityPage;
    BeamPage *m_beamPage;
    SpectrumPage *m_spectrumPage;
};

#endif // DEVICESTATUSPAGE_H
