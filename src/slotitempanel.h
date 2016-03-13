#ifndef SLOTITEMPANEL_H
#define SLOTITEMPANEL_H

#include <unordered_map>

#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "kancolledatabase.h"

class SlotItemPanel : public QTreeWidget
{
public:
  SlotItemPanel(QWidget* parent = nullptr);

private slots:
  void OnDataUpdated_(const KanColleDatabase* db);
  void UpdateItemList_(const KanColleDatabase* db);

private:
  std::unordered_map<int, QTreeWidgetItem*> item_type_widgets_;
  std::unordered_map<int, QTreeWidgetItem*> item_widgets_;
};

#endif // SLOTITEMPANEL_H
