#include <QDebug>
#include <QTreeWidgetItem>
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

void SlotItemPanel::OnDataUpdated_() {
  qDebug() << __func__ << " called";
  auto& db = KanColleDatabase::GetInstance();
  auto obj = db.GetData();
  auto api_data = (*obj)["api_data"];
  auto mst_slotitem = api_data.toObject()["api_mst_slotitem"];

  for (auto item : mst_slotitem.toArray()) {
    auto name = item.toObject()["api_name"];
    auto category_id = item.toObject()["api_type"].toArray()[0];

    auto parent = topLevelItem(category_id.toDouble() - 1);
    new QTreeWidgetItem(parent, {name.toString(), "0"});
  }
}
