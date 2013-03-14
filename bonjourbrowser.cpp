#include "bonjourbrowser.h"


BonjourBrowser::BonjourBrowser(QObject *parent) :
    QObject(parent)
{
}
BonjourBrowser::~BonjourBrowser()
{
}

void BonjourBrowser::browseForServiceType(
      const QString &serviceType)
{
  DNSServiceErrorType err = DNSServiceBrowse(&dnssref, 0,
        0, serviceType.toUtf8().constData(), 0,
        bonjourBrowseReply, this);
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

void BonjourBrowser::bonjourBrowseReply(DNSServiceRef,
      DNSServiceFlags flags, quint32,
      DNSServiceErrorType errorCode,
      const char *serviceName, const char *regType,
      const char *replyDomain, void *context)
{
  BonjourBrowser *browser =
        static_cast<BonjourBrowser *>(context);
  if (errorCode != kDNSServiceErr_NoError) {
    emit browser->error(errorCode);
  } else {
    BonjourRecord record(serviceName, regType,
                         replyDomain);
    if (flags & kDNSServiceFlagsAdd) {
      if (!browser->bonjourRecords.contains(record))
        browser->bonjourRecords.append(record);
    } else {
      browser->bonjourRecords.removeAll(record);
    }
    if (!(flags & kDNSServiceFlagsMoreComing)) {
      emit browser->currentBonjourRecordsChanged(
                                 browser->bonjourRecords);
    }
  }
}

void BonjourBrowser::bonjourSocketReadyRead()
{
  DNSServiceErrorType err =
        DNSServiceProcessResult(dnssref);
  if (err != kDNSServiceErr_NoError)
    emit error(err);
}
