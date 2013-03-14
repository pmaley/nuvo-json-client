#ifndef BONJOURBROWSER_H
#define BONJOURBROWSER_H

#include <QObject>
#include <QSocketNotifier>

#include "bonjourrecord.h"
#include <dns_sd.h>

class BonjourBrowser : public QObject
{
  Q_OBJECT

public:
  BonjourBrowser(QObject *parent = 0);
  ~BonjourBrowser();

  void browseForServiceType(const QString &serviceType);
  QList<BonjourRecord> currentRecords() const
    { return bonjourRecords; }
  QString serviceType() const { return browsingType; }

signals:
  void currentBonjourRecordsChanged(
        const QList<BonjourRecord> &list);
  void error(DNSServiceErrorType err);

private slots:
  void bonjourSocketReadyRead();

private:
  static void DNSSD_API bonjourBrowseReply(DNSServiceRef,
        DNSServiceFlags, quint32, DNSServiceErrorType,
        const char *, const char *, const char *, void *);

  DNSServiceRef dnssref;
  QSocketNotifier *bonjourSocket;
  QList<BonjourRecord> bonjourRecords;
  QString browsingType;
};

#endif // BONJOURBROWSER_H
