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

  std::shared_ptr<const MstData> GetData() const;
  std::shared_ptr<const ShipList> GetShip() const;
  std::shared_ptr<const SlotItemList> GetSlotItem() const;

  void ProcessData(KcsApi api_name, const std::string& response);

signals:
  void ShipUpdated(const KanColleDatabase* db);
  void SlotItemUpdated(const KanColleDatabase* db);
  void DataUpdated(const KanColleDatabase* db);

private:
  KanColleDatabase() = default;

  std::shared_ptr<const MstData> data_;
  std::shared_ptr<const ShipList> ship_;
  std::shared_ptr<const SlotItemList> slot_item_;
};

#endif // KANCOLLEDATABASE_H
