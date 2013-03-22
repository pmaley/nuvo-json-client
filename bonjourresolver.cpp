#include <QtCore/QSocketNotifier>
#include <QtNetwork/QHostInfo>

#include "bonjourrecord.h"
#include "bonjourresolver.h"

BonjourResolver::BonjourResolver(QObject *parent)
    : QObject(parent), dnssref(0), bonjourSocket(0), bonjourPort(-1)
{
}

BonjourResolver::~BonjourResolver()
{
    cleanupResolve();
}

void BonjourResolver::cleanupResolve()
{
    if (dnssref) {
        DNSServiceRefDeallocate(dnssref);
        dnssref = 0;
        delete bonjourSocket;
        bonjourPort = -1;
    }
}

void BonjourResolver::resolveBonjourRecord(const BonjourRecord &record)
{
    if (dnssref) {
        qWarning("resolve in process, aborting");
        return;
    }
    DNSServiceErrorType err = DNSServiceResolve(&dnssref, 0, 0,
                                                record.serviceName.toUtf8().constData(),
                                                record.registeredType.toUtf8().constData(),
                                                record.replyDomain.toUtf8().constData(),
                                                (DNSServiceResolveReply)bonjourResolveReply, this);
    if (err != kDNSServiceErr_NoError) {
        emit error(err);
    } else {
        int sockfd = DNSServiceRefSockFD(dnssref);
        if (sockfd == -1) {
            emit error(kDNSServiceErr_Invalid);
        } else {
            bonjourSocket = new QSocketNotifier(sockfd, QSocketNotifier::Read, this);
            connect(bonjourSocket, SIGNAL(activated(int)), this, SLOT(bonjourSocketReadyRead()));
        }
    }
}

void BonjourResolver::bonjourSocketReadyRead()
{
    DNSServiceErrorType err = DNSServiceProcessResult(dnssref);
    if (err != kDNSServiceErr_NoError)
        emit error(err);
}


void BonjourResolver::bonjourResolveReply(DNSServiceRef, DNSServiceFlags ,
                                    quint32 , DNSServiceErrorType errorCode,
                                    const char *, const char *hosttarget, quint16 port,
                                    quint16 , const char *, void *context)
{
    BonjourResolver *serviceResolver = static_cast<BonjourResolver *>(context);
    if (errorCode != kDNSServiceErr_NoError) {
        emit serviceResolver->error(errorCode);
        return;
    }
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        {
            port =  0 | ((port & 0x00ff) << 8) | ((port & 0xff00) >> 8);
        }
#endif
    serviceResolver->bonjourPort = port;
    QHostInfo::lookupHost(QString::fromUtf8(hosttarget),
                          serviceResolver, SLOT(finishConnect(const QHostInfo &)));
}

void BonjourResolver::finishConnect(const QHostInfo &hostInfo)
{
    emit bonjourRecordResolved(hostInfo, bonjourPort);
    QMetaObject::invokeMethod(this, "cleanupResolve", Qt::QueuedConnection);
}
