#ifndef KCSAPI_H
#define KCSAPI_H

#include <exception>
#include <memory>
#include <unordered_map>
#include <vector>

#include <QJsonObject>
#include <QJsonValue>

enum class KcsApi {
  NONE, START2, SLOT_ITEM, PORT
};

class BadJsonValue : public std::exception {
public:
  virtual const char* what() const override;
};

struct SlotItem {
  int id;
  int slot_item_id;
  int locked;
  int level;  // 改修レベル
  int alv;  // 艦載機熟練度

  static SlotItem FromJsonValue(const QJsonValue& value);
};

using SlotItemList = std::vector<SlotItem>;

struct Ship {
  int id;
  int ship_id;
  int lv;
  std::vector<int> slot;

  static Ship FromJsonValue(const QJsonValue& value);
};

using ShipList = std::vector<Ship>;

struct MstShip {
  int id;
  QString name;
};

struct MstSlotItem {
  int id;
  QString name;
  std::vector<int> type;
};

struct MstData {
  std::unordered_map<int, MstShip> mst_ship;
  std::unordered_map<int, MstSlotItem> mst_slot_item;

  static std::shared_ptr<const MstData> FromJsonValue(const QJsonValue& value);
};

#endif // KCSAPI_H
