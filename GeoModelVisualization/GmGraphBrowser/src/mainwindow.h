#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GeoModelKernel/GeoPhysVol.h"
QT_BEGIN_NAMESPACE

class QTreeWidgetItem;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:


  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void setTopLevel(PVConstLink root,int i);
  void setHeaderLabel(const char *label, int i);

private:

    using PMap=std::map<QTreeWidgetItem *,const GeoVPhysVol *>;

    Ui::MainWindow *ui;
    std::vector<PVConstLink>    root{2};
    std::vector<PMap>           pMap{2};
    void selectItem(QTreeWidgetItem *item, int column, int twId);
							      
private slots:

  void selectItem1(QTreeWidgetItem *item, int column) {
    selectItem(item,column,0);
  }
  void selectItem2(QTreeWidgetItem *item, int column) {
    selectItem(item,column,1);
  }

};
#endif // MAINWINDOW_H
