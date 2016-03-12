#ifndef SLOTITEMPANEL_H
#define SLOTITEMPANEL_H

#include <QTreeWidget>

#include "kancolledatabase.h"

class SlotItemPanel : public QTreeWidget
{
public:
  SlotItemPanel(QWidget* parent = nullptr);

private slots:
  void OnDataUpdated_();
};

#endif // SLOTITEMPANEL_H
