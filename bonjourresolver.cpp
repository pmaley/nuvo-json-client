#include "bonjourresolver.h"

BonjourResolver::BonjourResolver(QObject *parent) :
    QObject(parent)
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
void BonjourResolver::bonjourSocketReadyRead()
{
    DNSServiceErrorType err = DNSServiceProcessResult(dnssref);
    if (err != kDNSServiceErr_NoError)
        emit error(err);
}

void BonjourResolver::resolveBonjourRecord(const BonjourRecord &record)
{
  if (dnssref) {
    qWarning("Resolve already in process");
    return;
  }

  DNSServiceErrorType err = DNSSD_API::DNSServiceResolve(&dnssref, 0,
        0, record.serviceName.toUtf8().constData(),
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
      bonjourSocket = new QSocketNotifier(sockfd,
                             QSocketNotifier::Read, this);
      connect(bonjourSocket, SIGNAL(activated(int)),
              this, SLOT(bonjourSocketReadyRead()));
    }
  }
}

void BonjourResolver::bonjourResolveReply(DNSServiceRef,
      DNSServiceFlags, quint32,
      DNSServiceErrorType errorCode, const char *,
      const char *hostTarget, quint16 port, quint16,
      const char *, void *context)
{
  BonjourResolver *resolver =
        static_cast<BonjourResolver *>(context);
  if (errorCode != kDNSServiceErr_NoError) {
    emit resolver->error(errorCode);
    return;
  }

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
  port = ((port & 0x00ff) << 8) | ((port & 0xff00) >> 8);
#endif
  resolver->bonjourPort = port;
  QHostInfo::lookupHost(QString::fromUtf8(hostTarget),
        resolver, SLOT(finishConnect(const QHostInfo &)));
}

void BonjourResolver::finishConnect(
      const QHostInfo &hostInfo)
{
  emit recordResolved(hostInfo, bonjourPort);
  QMetaObject::invokeMethod(this, "cleanupResolve",
                            Qt::QueuedConnection);
}
