#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QString>

#include "FrameTypes.h"

class FrameParser : public QObject
{
    Q_OBJECT

public:
    explicit FrameParser(QObject *parent = nullptr);
    ~FrameParser() override;

    QList<Frame> parse(const QByteArray &data);
    QByteArray buildFrame(CommandCode command, uint32_t sequence, const QByteArray &payload);
    void clearBuffer();

signals:
    void frameParsed(const Frame &frame);
    void errorOccurred(const QString &error);

private:
    int findHeader(const QByteArray &buffer) const;
    bool validateFrame(const QByteArray &frame) const;
    uint16_t calculateCRC16(const QByteArray &data) const;

    QByteArray m_buffer;
};

#endif // FRAMEPARSER_H
