#ifndef BONJOURRESOLVER_H
#define BONJOURRESOLVER_H

#include <QtCore/QObject>


#include <dns_sd.h>

class QSocketNotifier;
class QHostInfo;
class BonjourRecord;

class BonjourResolver : public QObject
{
    Q_OBJECT
public:
    BonjourResolver(QObject *parent);
    ~BonjourResolver();

    void resolveBonjourRecord(const BonjourRecord &record);

signals:
    void bonjourRecordResolved(const QHostInfo &hostInfo, int port);
    void error(DNSServiceErrorType error);

private slots:
    void bonjourSocketReadyRead();
    void cleanupResolve();
    void finishConnect(const QHostInfo &hostInfo);

private:
    static void DNSSD_API bonjourResolveReply(DNSServiceRef sdRef, DNSServiceFlags flags,
                                    quint32 interfaceIndex, DNSServiceErrorType errorCode,
                                    const char *fullName, const char *hosttarget, quint16 port,
                                    quint16 txtLen, const char *txtRecord, void *context);
    DNSServiceRef dnssref;
    QSocketNotifier *bonjourSocket;
    int bonjourPort;
};

#endif // BONJOURRESOLVER_H
