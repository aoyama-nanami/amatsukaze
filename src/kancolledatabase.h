#ifndef KANCOLLEDATABASE_H
#define KANCOLLEDATABASE_H

#include <memory>

#include <QObject>
#include <QJsonObject>

#include <boost/noncopyable.hpp>

#include "kcsapi.h"

class KanColleDatabase : public QObject, public boost::noncopyable {
  Q_OBJECT
public:
  static KanColleDatabase& GetInstance();

  std::shared_ptr<const QJsonObject> GetData() const;
  std::shared_ptr<const QJsonObject> GetSlotItem() const;

  void ProcessData(KcsApi api_name, const std::string& response);

signals:
  void SlotItemUpdated(const KanColleDatabase* db);

private:
  KanColleDatabase() = default;

  std::shared_ptr<const QJsonObject> data_;
  std::shared_ptr<const QJsonObject> slot_item_;
};

#endif // KANCOLLEDATABASE_H
