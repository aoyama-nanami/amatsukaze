#include <QDebug>
#include <QJsonArray>

#include "slotitempanel.h"

namespace {
  const char* ITEM_CATEGORY[] = {
    u8"砲",
    u8"魚雷",
    u8"艦載機",
    u8"機銃・特殊弾(対空系)",
    u8"偵察機・電探(索敵系)",
    u8"強化",
    u8"対潜装備",
    u8"大発動艇・探照灯",
    u8"簡易輸送部材",
    u8"艦艇修理施設",
    u8"照明弾",
    u8"司令部施設",
    u8"航空要員",
    u8"高射装置",
    u8"対地装備",
    u8"水上艦要員",
    u8"大型飛行艇",
    u8"戦闘糧食",
    u8"補給物資"
  };
}

SlotItemPanel::SlotItemPanel(QWidget* parent) : QTreeWidget(parent) {
  connect(&KanColleDatabase::GetInstance(), &KanColleDatabase::DataUpdated,
          this, &SlotItemPanel::OnDataUpdated_);
  setColumnCount(2);
  setSortingEnabled(false);

  for (auto name : ITEM_CATEGORY) {
    auto tree_item = new QTreeWidgetItem(this, {name, ""});
    tree_item->setFirstColumnSpanned(true);
  }
}

void SlotItemPanel::OnDataUpdated_(const KanColleDatabase* db) {
  qDebug() << __func__ << " called";
  auto mst_data = db->GetData();
  auto& mst_slot_item = mst_data->mst_slot_item;

  for (auto& p : mst_slot_item) {
    auto& item = p.second;
    int category_id = item.type[0];
    int slot_id = item.id;

    auto parent = topLevelItem(category_id - 1);
    auto tree_item = new QTreeWidgetItem(parent, {item.name, "0"});
    tree_item->setHidden(true);
    item_type_widgets_[slot_id] = tree_item;
  }

  disconnect(db, &KanColleDatabase::DataUpdated,
             this, &SlotItemPanel::OnDataUpdated_);
  connect(&KanColleDatabase::GetInstance(), &KanColleDatabase::SlotItemUpdated,
          this, &SlotItemPanel::UpdateItemList_);
  connect(&KanColleDatabase::GetInstance(), &KanColleDatabase::ShipUpdated,
          this, &SlotItemPanel::UpdateItemList_);
}

void SlotItemPanel::UpdateItemList_(const KanColleDatabase* db) {
  qDebug() << __func__ << " called";
  auto slot_item = db->GetSlotItem();
  if (slot_item == nullptr) return;

  decltype(item_widgets_) new_widgets;
  for (auto& item : *slot_item) {
    auto iter = item_widgets_.find(item.id);
    if (iter == item_widgets_.end()) {
      auto parent_widget = item_type_widgets_.find(item.slot_item_id);
      if (item_type_widgets_.count(item.slot_item_id) == 0) {
        qFatal("slot_id %d not found", item.slot_item_id);
      }
      auto widget = new QTreeWidgetItem(parent_widget->second,
                                        {QString::number(item.id), ""});
      new_widgets.insert({item.id, widget});
    } else {
      new_widgets.insert({item.id, iter->second});
      item_widgets_.erase(iter);
    }
  }

  for (auto& p : item_widgets_) {
    p.second->parent()->removeChild(p.second);
    delete p.second;
  }

  for (auto& p : item_type_widgets_) {
    int n = p.second->childCount();
    p.second->setText(1, QString::number(n));
    p.second->setHidden(n == 0);
  }

  item_widgets_.swap(new_widgets);

  auto mst_data = db->GetData();
  auto ship = db->GetShip();
  if (mst_data == nullptr || ship == nullptr) return;
  for (auto& s : *ship) {
    auto ship_name = mst_data->mst_ship.find(s.ship_id)->second.name;
    auto ship_text = QString("%1 (Lv%2)").arg(ship_name).arg(s.lv);
    for (auto slot : s.slot) {
      if (slot >= 0) {
        item_widgets_[slot]->setText(1, ship_text);
      }
    }
  }
}
