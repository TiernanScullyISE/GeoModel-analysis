#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include <QTreeWidget>
#include <typeinfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeWidget1->setColumnCount(3);
    ui->treeWidget2->setColumnCount(3);
    connect(ui->treeWidget1,&QTreeWidget::itemDoubleClicked,this,&MainWindow::selectItem1);
    connect(ui->treeWidget2,&QTreeWidget::itemDoubleClicked,this,&MainWindow::selectItem2);
    for (QTreeWidget * tw:{ui->treeWidget1,ui->treeWidget2}){
        for (int i=0;i<3;i++) tw->setColumnWidth(i,300);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setHeaderLabel(const char *label, int i) {
  QTreeWidget *tw[2]={ui->treeWidget1,ui->treeWidget2};
  tw[i]->setHeaderLabels({label,"Info","Share count"});
  
}

void MainWindow::setTopLevel(PVConstLink world, int i) {

  QTreeWidget *tw[2]={ui->treeWidget1,ui->treeWidget2};
  if (!world) {
    tw[i]->hide();
    return;
  }
  root[i]=world;

    QList<QTreeWidgetItem *> items;
    GeoVolumeCursor v(root[i]);
    while (!v.atEnd()){
        QStringList volInfo;
        volInfo.push_back(QString(v.getVolume()->getLogVol()->getName().c_str()));
        volInfo.push_back(QString(v.getVolume()->getLogVol()->getMaterial()->getName().c_str()));
        QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), volInfo);
        items.append(newItem);
        pMap[i][newItem]=v.getVolume().get();
        v.next();
    }
    tw[i]->insertTopLevelItems(0, items);
}


void MainWindow::selectItem(QTreeWidgetItem *item, int column, int twId){
  if (item->childCount()!=0) return;
  const GeoVPhysVol *vol=pMap[twId][item];
  if (vol) {
    QList<QTreeWidgetItem *> items;
    for (unsigned int g=0;g<vol->getNChildNodes();g++) {
      const GeoGraphNode * const * graphNode=vol->getChildNode(g);
      const GeoFullPhysVol *fullPhysVol = dynamic_cast<const GeoFullPhysVol *> (*graphNode);
      const GeoPhysVol *physVol = dynamic_cast<const GeoPhysVol *> (*graphNode);
      const GeoTransform *transform = dynamic_cast<const GeoTransform *> (*graphNode);
      const GeoAlignableTransform *alignableTransform = dynamic_cast<const GeoAlignableTransform *> (*graphNode);
      const GeoNameTag *nameTag = dynamic_cast<const GeoNameTag *> (*graphNode);
      const GeoIdentifierTag *identifierTag = dynamic_cast<const GeoIdentifierTag *> (*graphNode);
      const GeoSerialTransformer *serialTransformer = dynamic_cast<const GeoSerialTransformer *> (*graphNode);
      const GeoSerialIdentifier *serialIdentifier = dynamic_cast<const GeoSerialIdentifier *> (*graphNode);
      const GeoSerialDenominator *serialDenominator = dynamic_cast<const GeoSerialDenominator *> (*graphNode);

      QBrush b;
      b.setStyle(Qt::SolidPattern);
      QStringList info;

      int         refCount=(*graphNode)->refCount();
      std::string refCountString=refCount<2 ? "" : std::to_string( refCount);
      QString     refCountQString=refCountString.c_str();
      
      if (fullPhysVol) {
	info.push_back(QString(fullPhysVol->getLogVol()->getName().c_str()));
	info.push_back(QString(fullPhysVol->getLogVol()->getMaterial()->getName().c_str()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	b.setColor("darkGreen");
	newItem->setForeground(0,b);
	newItem->setForeground(1,b);
	newItem->setForeground(2,b);
	if (fullPhysVol->getNChildVols()==0) newItem->setDisabled(true);
	pMap[twId][newItem]=fullPhysVol;
      }
      else if (physVol) {
	info.push_back(QString(physVol->getLogVol()->getName().c_str()));
	info.push_back(QString(physVol->getLogVol()->getMaterial()->getName().c_str()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	if (physVol->getNChildVols()==0) newItem->setDisabled(true);
	pMap[twId][newItem]=physVol;
      }
      else if (serialTransformer) {
	info.push_back(QString("SerialTransformer"));
	info.push_back(QString((std::to_string(serialTransformer->getNCopies())+" Children").c_str()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	b.setColor("darkMagenta");
	newItem->setForeground(0,b);
	newItem->setForeground(1,b);
	newItem->setForeground(2,b);
      }
      else if (alignableTransform) {
	info.push_back(QString("AlignableTransform"));
	info.push_back(QString(""));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	b.setColor("red");
	newItem->setForeground(0,b);
	newItem->setForeground(1,b);
	newItem->setForeground(2,b);
	newItem->setDisabled(true);
      }
      else if (transform) {
	info.push_back(QString("Transform"));
	info.push_back(QString(""));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	b.setColor("blue");
	newItem->setForeground(0,b);
	newItem->setForeground(1,b);
	newItem->setForeground(2,b);
	newItem->setDisabled(true);
      }
      else if (identifierTag) {
	info.push_back(QString("IDTag:"));
	info.push_back(QString(std::to_string(identifierTag->getIdentifier()).c_str()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	QFont font=newItem->font(0);
	font.setItalic(true);
	newItem->setFont(0,font);
	newItem->setFont(1,font);
	newItem->setFont(2,font);
	newItem->setDisabled(true);
      }
      else if (nameTag) {
	info.push_back(QString("NameTag:"));
	info.push_back(QString(nameTag->getName().c_str()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	QFont font=newItem->font(0);
	font.setItalic(true);
	newItem->setFont(0,font);
	newItem->setFont(1,font);
	newItem->setFont(2,font);
	newItem->setDisabled(true);
      }
      else if (serialDenominator) {
	info.push_back(QString("NameTag:"));
	info.push_back(QString(serialDenominator->getBaseName().c_str()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	QFont font=newItem->font(0);
	font.setItalic(true);
	newItem->setFont(0,font);
	newItem->setFont(1,font);
	newItem->setFont(2,font);
	newItem->setDisabled(true);
      }
      else if (serialIdentifier) {
	info.push_back(QString("SerialIdentifier:"));
	info.push_back(QString(std::to_string(serialIdentifier->getBaseId()).c_str()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	QFont font=newItem->font(0);
	font.setItalic(true);
	newItem->setFont(0,font);
	newItem->setFont(1,font);
	newItem->setFont(2,font);
	newItem->setDisabled(true);
      }
      else {
	QStringList info;
	
	info.push_back(QString("Unresolved type"));
	info.push_back(QString(typeid(graphNode).name()));
	info.push_back(refCountQString);
	QTreeWidgetItem *newItem= new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), info);
	items.append(newItem);
	newItem->setDisabled(true);
      }
    }
    item->addChildren(items);
  }
}




