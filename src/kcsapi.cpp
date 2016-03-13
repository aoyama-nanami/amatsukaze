#include <QDebug>
#include <QJsonArray>

#include "kcsapi.h"

namespace {
  int GetInt(const QJsonObject& object, const char* key) {
    auto value = object[key];
    if (!value.isDouble()) throw BadJsonValue();
    return value.toDouble();
  }

  int GetOptionalInt(const QJsonObject& object, const char* key,
                          int default_value = 0) {
    try {
      return GetInt(object, key);
    } catch (...) {
      return default_value;
    }
  }

  std::vector<int> GetIntArray(const QJsonObject& object, const char* key) {
    auto arr = object[key];
    if (!arr.isArray()) throw BadJsonValue();
    std::vector<int> ret;

    for (auto& x : arr.toArray()) {
      if (!x.isDouble()) throw BadJsonValue();
      ret.push_back(x.toDouble());
    }

    return ret;
  }

  QString GetString(const QJsonObject& object, const char* key) {
    auto value = object[key];
    if (!value.isString()) throw BadJsonValue();
    return value.toString();
  }
}

const char* BadJsonValue::what() const {
  return "json conversion failed";
}

SlotItem SlotItem::FromJsonValue(const QJsonValue& value) {
  SlotItem item;
  auto obj = value.toObject();
  item.id = GetInt(obj, "api_id");
  item.slot_item_id = GetInt(obj, "api_slotitem_id");
  item.locked = GetInt(obj, "api_locked");
  item.level = GetInt(obj, "api_level");
  item.alv = GetOptionalInt(obj, "api_alv", 0);
  return item;
}

Ship Ship::FromJsonValue(const QJsonValue& value) {
  Ship ship;
  auto obj = value.toObject();
  ship.id = GetInt(obj, "api_id");
  ship.ship_id = GetInt(obj, "api_ship_id");
  ship.lv = GetInt(obj, "api_lv");
  ship.slot = GetIntArray(obj, "api_slot");
  return ship;
}

std::shared_ptr<const MstData> MstData::FromJsonValue(const QJsonValue& value) {
  auto mst_data = std::make_shared<MstData>();
  auto obj = value.toObject();
  if (!obj["api_mst_ship"].isArray()) throw BadJsonValue();
  if (!obj["api_mst_slotitem"].isArray()) throw BadJsonValue();

  for (auto& x : obj["api_mst_ship"].toArray()) {
    if (!x.isObject()) throw BadJsonValue();
    auto x_obj = x.toObject();
    MstShip ship;
    ship.id = GetInt(x_obj, "api_id");
    ship.name = GetString(x_obj, "api_name");
    mst_data->mst_ship.emplace(ship.id, ship);
  }

  for (auto& x : obj["api_mst_slotitem"].toArray()) {
    if (!x.isObject()) throw BadJsonValue();
    auto x_obj = x.toObject();
    MstSlotItem slot_item;
    slot_item.id = GetInt(x_obj, "api_id");
    slot_item.name = GetString(x_obj, "api_name");
    slot_item.type = GetIntArray(x_obj, "api_type");
    mst_data->mst_slot_item.emplace(slot_item.id, slot_item);
  }

  return std::const_pointer_cast<const MstData>(mst_data);
}
