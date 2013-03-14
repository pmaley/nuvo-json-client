#ifndef BONJOURRESOLVER_H
#define BONJOURRESOLVER_H

#include <QObject>
#include <QHostInfo>
#include <QSocketNotifier>
#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <dns_sd.h>

#include "bonjourrecord.h"

class BonjourResolver : public QObject
{
  Q_OBJECT

public:
  BonjourResolver(QObject *parent);
  ~BonjourResolver();

  void resolveBonjourRecord(const BonjourRecord &record);

signals:
  void recordResolved(const QHostInfo &hostInfo,
                      int port);
  void error(DNSServiceErrorType error);

private slots:
  void bonjourSocketReadyRead();
  void cleanupResolve();
  void finishConnect(const QHostInfo &hostInfo);

private:
  static void DNSSD_API bonjourResolveReply(DNSServiceRef,
        DNSServiceFlags, quint32, DNSServiceErrorType,
        const char *, const char *, quint16, quint16,
        const char *, void *);

  DNSServiceRef dnssref;
  QSocketNotifier *bonjourSocket;
  int bonjourPort;
};

#endif // BONJOURRESOLVER_H
