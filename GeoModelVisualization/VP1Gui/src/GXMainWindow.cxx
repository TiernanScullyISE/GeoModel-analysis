/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         //
//  Implementation of class GXMainWindow                  //
//                                                         //
//  Author: Thomas Kittelmann <Thomas.Kittelmann@cern.ch>  //
//                                                         //
//  Initial version: April 2007                            //
//
//  Updates:
//  - Riccardo.Maria.Bianchi@cern.ch
//                                                         //
/////////////////////////////////////////////////////////////

#include "VP1Gui/GXMainWindow.h"

#include "VP1Gui/VP1ChannelManager.h"
#include "VP1Gui/VP1TabManager.h"
#include "VP1Gui/GXExecutionScheduler.h"
#include "VP1Gui/VP1PluginDialog.h"
#include "VP1Gui/VP1DockWidget.h"


#include "VP1Base/IVP1ChannelWidget.h"
#include "VP1Base/IVP1System.h"
#include "VP1Base/VP1Msg.h"
#include "VP1Base/VP1Settings.h"
#include "VP1Base/VP1QtUtils.h"


#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QComboBox>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QProgressBar>
#include <QStyleFactory>
#include <QDateTime>
#include <QMutex>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtGui>
#include <QSettings>
#include <QScrollBar>


#include <cassert>
#include <iostream>

//_________________________________________________________________________________
GXMainWindow::GXMainWindow(GXExecutionScheduler*sched,QWidget * parent)
: QMainWindow(parent),
  m_mustquit(false),
  m_dummyemptycontroller(new QWidget(0)),
  m_scheduler(sched),
  m_settingsfile(QDir::homePath()+QDir::separator()+".atlasvp1"),
  m_userRequestedExit(false),
  m_mutex(new QMutex())
{
	setupUi(this); // this sets up the GUI
	setupStatusBar();
	//

	m_channelmanager = new VP1ChannelManager(m_scheduler,this);
	m_tabmanager = new VP1TabManager(this,tabWidget_central,m_channelmanager);
	stackedWidget_customcontrols->addWidget(m_dummyemptycontroller);

	//Final touches to instructions page:

	//  Remove any margins added by the layouts in the stacked
	//  widget. This really ought to be the default for stacked widgets!
	for (int i=0;i<stackedWidget_central->count();++i)
		stackedWidget_central->widget(i)->layout()->setContentsMargins(0,0,0,0);

	frame_instructions->setStyleSheet("QFrame#frame_instructions { "+VP1DockWidget::highlightStyle()+" } ");
	frame_instructions->setFrameShape(QFrame::StyledPanel);
	//   textBrowser_intro1->setStyleSheet("QTextBrowser#textBrowser_intro1 { background-color: rgba(0, 0, 0, 0%) } ");
	//   textBrowser_intro2->setStyleSheet("QTextBrowser#textBrowser_intro2 { background-color: rgba(0, 0, 0, 0%) } ");
	//connect(pushButton_quicksetup_geometrystudies,SIGNAL(clicked()),this,SLOT(quickSetupTriggered()));

	//Default application font:
	m_defaultfont = QApplication::font();
	m_defaultfont_pointsize = m_defaultfont.pointSizeF();
	if (m_defaultfont_pointsize<0.0)
		m_defaultfont_pixelsize = m_defaultfont.pixelSize();
	else
		m_defaultfont_pixelsize = -1;

	//Tabs:
	connect(m_tabmanager,SIGNAL(tabListChanged(QStringList)),this,SLOT(tabListChanged(QStringList)));
	connect(tabWidget_central,SIGNAL(currentChanged(int)),this,SLOT(updateCentralStackWidget()));
	updateCentralStackWidget();

	//Channels:

	connect(pushButton_saveAllChannels,SIGNAL(clicked()),this,SLOT(saveAllCurrentChannels()));
	connect(pushButton_makeEventDisplay,SIGNAL(clicked()),this,SLOT(makeAllChannelsEventDisplay()));

	connect(pushButton_3D,SIGNAL(clicked()),this,SLOT(launch3DstereoEditor()));

	connect(pushButton_channelfullscreen,SIGNAL(clicked()),m_tabmanager,SLOT(showCurrentChannelFullScreen()));
	connect(pushButton_channelinformation,SIGNAL(clicked()),this,SLOT(request_channelInformation()));
	connect(pushButton_printchannel,SIGNAL(clicked()),this,SLOT(request_printChannel()));
	connect(pushButton_savesnapshotchannel,SIGNAL(clicked()),this,SLOT(request_saveChannelSnapshot()));
	connect(m_tabmanager,SIGNAL(selectedChannelChanged(IVP1ChannelWidget*)),this,SLOT(selectedChannelChanged(IVP1ChannelWidget*)));

	//Menu:
	//Quick start:
	connect(action_quicklaunch_Geometry_studies,SIGNAL(triggered(bool)),this,SLOT(quickSetupTriggered()));
	connect(action_exit_VP1,SIGNAL(triggered(bool)),this,SLOT(close()));

	//Configuration
	m_menu_loadConfFile = menuConfiguration->addMenu ( "&Load tab configuration from file" );
	m_menu_loadConfFile->setStatusTip("Select .vp1 config file to load");
	connect(m_menu_loadConfFile,SIGNAL(aboutToShow()),this,SLOT(showMenu_loadConfFile()));
	menuConfiguration->addSeparator();
	//plugins:
	m_menu_loadPlugin = menuConfiguration->addMenu ( "&Load plugin" );
	m_menu_loadPlugin->setStatusTip("Select plugin to load");
	//m_action_infoAboutLoadedPlugins = menuPlugins->addAction ( "&Info about loaded plugins" );
	//   m_action_infoAboutLoadedPlugins->setStatusTip("Get information about the presently loaded plugins");
	//   m_action_infoAboutLoadedPlugins->setEnabled(false);
	connect(m_menu_loadPlugin,SIGNAL(aboutToShow()),this,SLOT(showMenu_loadPlugin()));
	//Style
	m_menu_changeStyle = menu_Style->addMenu ( "&Style" );
	QActionGroup * styleGroup = new QActionGroup(this);
	QAction * laststyleact(0);
	bool foundplastique=false;
	QSettings s(m_settingsfile,QSettings::IniFormat);
	QString defaultstyle=s.value("style/defaultstyle", "Fusion").toString();
	foreach (QString style, QStyleFactory::keys() ) {
		QAction * act = m_menu_changeStyle->addAction(style);
		act->setStatusTip("Change application style to "+style);
		connect(act,SIGNAL(triggered(bool)),this,SLOT(changeStyleActionTriggered()));
		act->setCheckable(true);
		styleGroup->addAction(act);
		laststyleact=act;
		if (act->text()==defaultstyle) {
			act->setChecked(true);
			QApplication::setStyle(QStyleFactory::create(act->text()));
			foundplastique=true;
		}
	}
	if (!foundplastique) {
		laststyleact->setChecked(true);
		QApplication::setStyle(QStyleFactory::create(laststyleact->text()));
	}
	//Fonts:
	int savedgoal = 0;
	if (m_defaultfont_pointsize<0.0)
		savedgoal = s.value("font/pixelsize_diff", 0).toInt();
	else
		savedgoal = s.value("font/pointsize_relativepercent", 0).toInt();

	m_menu_changeFontSize = menu_Style->addMenu ( "&Font size" );
	QList<int> fontoptions;
	if (m_defaultfont_pointsize<0.0)
		fontoptions <<15<<10<<+5<<+3<<+2<<+1<<0<<-1<<-2<<-3<<-5<<-10<<-20<<-30;
	else
		fontoptions <<150<<100<<50<<30<<20<<10<<5<<0<<-5<<-10<<-20<<-30<<-50;

	QActionGroup * fontGroup = new QActionGroup(this);
	bool foundsavedgoal(false);
	QAction * normalfontact(0);
	foreach (int fontopt,fontoptions) {
		if (m_defaultfont_pixelsize>0&&m_defaultfont_pixelsize+fontopt<=0)
			continue;
		QString text = (fontopt==0?"normal": (fontopt>0?"+":"")+QString::number(fontopt)+(m_defaultfont_pointsize < 0.0? " pixels" : "%"));
		QAction * act = m_menu_changeFontSize->addAction(text);
		act->setStatusTip("Change overall font size of application to "+text);
		act->setData(fontopt);
		act->setCheckable(true);
		fontGroup->addAction(act);
		connect(act,SIGNAL(triggered(bool)),this,SLOT(changeFontSizeActionTriggered()));
		if (fontopt==0)
			normalfontact=act;
		if (savedgoal==fontopt) {
			act->setChecked(true);
			changeFontSize(fontopt);
			foundsavedgoal=true;
		}
	}
	if (!foundsavedgoal) {
		normalfontact->setChecked(true);
	}

	//
	m_actionSave_current_tabs = menuConfiguration->addAction ( "&Save current tab configuration to file" );
	m_actionSave_current_tabs->setStatusTip("Save current tab/channel layout to .vp1 config file");
	menuConfiguration->addSeparator();
	m_actionAdd_empty_tab = menuConfiguration->addAction ( "&Add empty tab" );
	m_actionAdd_empty_tab->setStatusTip("Add empty tab to the current tab list");
	connect(m_actionAdd_empty_tab,SIGNAL(triggered(bool)),this,SLOT(request_addEmptyTab()));
	connect(m_actionSave_current_tabs,SIGNAL(triggered(bool)),this,SLOT(request_saveasConfig()));


	// Help menu
	QMenu* menu_help = new QMenu(menubar);
	menu_help->setObjectName("menu_help");
	menu_help->setTitle("&Help");
	menubar->addAction(menu_help->menuAction());
	m_action_openGMSite = menu_help->addAction("GeoModel &Web Site");
	menu_help->addSeparator();
	m_action_openAbout = menu_help->addAction("&About GeoModelExplorer");

	QTimer::singleShot(0, this, SLOT(postInitUpdates()));
	m_currentsaveimagepath = VP1Settings::defaultFileSelectDirectory();
	m_currentloadpluginpath = VP1Settings::defaultFileSelectDirectory();

	connect(m_action_openGMSite,SIGNAL(triggered(bool)),this,SLOT(help_openGMWebSite()));
	connect(m_action_openAbout,SIGNAL(triggered(bool)),this,SLOT(help_openAbout()));

  // FIXME: enabling menubar again. It's part of a quickfix, described here: https://its.cern.ch/jira/browse/ATLASVPONE-120
  #ifndef BUILDVP1LIGHT
  	menubar->setEnabled(false);
  #endif
	
  // JFB DISABLES:
  pushButton_makeEventDisplay->hide();
  pushButton_3D->hide();
  pushButton_saveAllChannels->hide();
  menuConfiguration->hide();
  menuConfiguration->setVisible(false);
  menuConfiguration->setEnabled(false);
  menuConfiguration->setTitle("");

}

//_________________________________________________________________________________
void GXMainWindow::launch3DstereoEditor()
{
	VP1Msg::messageDebug("GXMainWindow::launch3DstereoEditor()");
	m_tabmanager->launchStereoEditorCurrentTab();

}

//_________________________________________________________________________________
void GXMainWindow::help_openGMWebSite() {
	/*
	 * Open the online help from a web url location
	 * with the default system web browser
	 */
	QDesktopServices::openUrl(QUrl("http://cern.ch/geomodel"));
	return;
}


//_________________________________________________________________________________
void GXMainWindow::help_openAbout() {
  /*
   * open the online help with the internal web browser
   */
  QMessageBox::information(0, "About GeoModelExplorer", "The GeoModelExplorer is part of the GeoModel tool suite"
			   " developed within the ATLAS experiment by Riccardo Bianchi,"
			   "  Joe Boudreau, Andrea Dell'acqua, Thomas Kittelmann, "
			   " and Vakho Tsulaia",
			   QMessageBox::Ok,QMessageBox::Ok);
  return;
}


//_________________________________________________________________________________
void GXMainWindow::postInitUpdates(){

	//Make sure that the splitters give maximum space for the tab area.

	int sum1 = splitter_tabwidget_messagebox->sizes().at(0) + splitter_tabwidget_messagebox->sizes().at(1);
	QList<int> sizes1; sizes1 << sum1 << 0;
	splitter_tabwidget_messagebox->setSizes ( sizes1 );
	int bottom = 1;
	while (splitter_tabwidget_messagebox->sizes().at(1)==0&&bottom<50000) {
		QList<int> sizes2; sizes2 << sum1-bottom << bottom;
		splitter_tabwidget_messagebox->setSizes ( sizes2 );
		++bottom;
	}

	int sum2 = splitter_leftright->sizes().at(0) + splitter_leftright->sizes().at(1);
	int left = 300;
	QList<int> sizes3; sizes3 << left << sum2-left;
	splitter_leftright->setSizes ( sizes3 );
	while (splitter_leftright->sizes().at(0)==0&&left<50000) {
		QList<int> sizes4; sizes4 << left << sum2-left;
		splitter_leftright->setSizes ( sizes4 );
		++left;
	}

	int prefwidths = std::max<int>(left,widget_controlsContainer->sizeHint().width());
	prefwidths = std::max<int>(prefwidths,groupBox_channelcontrols->sizeHint().width());
	//	prefwidths = std::max<int>(prefwidths,groupBox_cruise->sizeHint().width());
	//	prefwidths = std::max<int>(prefwidths,groupBox_event->sizeHint().width());
	prefwidths = std::max<int>(prefwidths,stackedWidget_customcontrols->sizeHint().width());
	widget_controlsContainer->setMaximumWidth(50+prefwidths);
	//   int h1(textBrowser_intro1->viewport()->sizeHint().height());
	//   textBrowser_intro1->setMaximumHeight(h1+2);
	updateEventControls();
}

//_________________________________________________________________________________
void GXMainWindow::setupStatusBar()  {
	progressbar = new QProgressBar();
	progressbar->setMinimum(0);
	progressbar->reset();
	progressbar->setOrientation(Qt::Horizontal);
	statusBar()->addPermanentWidget(progressbar);
	m_statusbarlabel = new QLabel();
	//Turn off ugly box around items in statusbar:
	statusBar()->setStyleSheet("QStatusBar::item { border-width: 0 }");
	statusBar()->addPermanentWidget(m_statusbarlabel);
	progressbar->hide();
}

//_________________________________________________________________________________
GXMainWindow::~GXMainWindow()
{
	VP1Msg::messageDebug("deleting the tab manager");
	delete m_tabmanager;
	VP1Msg::messageDebug("deleting the channel manager");
	delete m_channelmanager;

	delete m_mutex;

}

//_________________________________________________________________________________
bool GXMainWindow::mustQuit() const {
	return m_mustquit;
}


//_________________________________________________________________________________
void GXMainWindow::loadPluginFile(QString filename)
{
	VP1Msg::messageDebug("loadPluginFile()");

	QString err = m_channelmanager->loadPluginFile(filename);
	if (!err.isEmpty()) {
		QMessageBox::critical(0, "Error - could not load plugin file: "+filename,
				"Could not load plugin file: "
				+filename+"\n\nReason: "+err,QMessageBox::Ok,QMessageBox::Ok);
		VP1Msg::message("Could not load plugin file: "+filename+"\n\nReason: "+err);
	}
	//Fixme: Error message here is hardcoded to be the same as in loadPluginFile method!!
}

//_________________________________________________________________________________
void GXMainWindow::request_addEmptyTab() {
	bool ok;
	QString newtabname = QInputDialog::getText( 0, "New Tab Name","New tab name:",
			QLineEdit::Normal, m_tabmanager->suggestNewTabName("My Tab"), &ok );
	if (!ok||newtabname.isEmpty())
		return;
	m_tabmanager->addNewTab(newtabname);
}

//_________________________________________________________________________________
void GXMainWindow::tabListChanged(QStringList l) {
	updateCentralStackWidget();
	if (l.count()) {
		m_actionSave_current_tabs->setEnabled(true);
		//		groupBox_cruise->setEnabled(true);
	} else {
		m_actionSave_current_tabs->setEnabled(false);
		//groupBox_cruise->setEnabled(false);
	}
	if (l.count()>1) {
	  //		radioButton_cruise_tab->setEnabled(true);
	  //		radioButton_cruise_both->setEnabled(true);
	} else {
	  //if (radioButton_cruise_tab->isChecked()||radioButton_cruise_both->isChecked())
		  //			radioButton_cruise_event->setChecked(true);
		  //radioButton_cruise_tab->setEnabled(false);
		  //radioButton_cruise_both->setEnabled(false);
	}
}

//_________________________________________________________________________________
void GXMainWindow::addChannelIconsToComboBox(QComboBox* cb, const bool& isbasenames) {
	int n= cb->count();
	for (int i = 0; i<n; ++i) {
		QString icontext = m_channelmanager->getIconLocation(cb->itemText(i), isbasenames);
		if (!icontext.isEmpty())
			cb->setItemIcon ( i, QIcon(icontext) );
	}
}

//_________________________________________________________________________________
void GXMainWindow::selectedChannelChanged(IVP1ChannelWidget* cw)
{
	VP1Msg::messageDebug("GXMainWindow::selectedChannelChanged()");

	//Controls box:
	if (cw) {
		groupBox_channelcontrols->setTitle("Controls: "+cw->unique_name());
		groupBox_channelcontrols->setEnabled(true);
		QWidget* controller = m_channelmanager->getController(cw);
		if (!controller) {
			stackedWidget_customcontrols->setCurrentWidget(m_dummyemptycontroller);
		} else {
			if (stackedWidget_customcontrols->indexOf(controller)==-1)
				stackedWidget_customcontrols->addWidget(controller);
			stackedWidget_customcontrols->setCurrentWidget(controller);
		}
	} else {
		groupBox_channelcontrols->setTitle("Controls: no channel selected");
		groupBox_channelcontrols->setEnabled(false);
		stackedWidget_customcontrols->setCurrentWidget(m_dummyemptycontroller);
	}

	// FIXME: enabling menubar again. It's part of a quickfix, described here: https://its.cern.ch/jira/browse/ATLASVPONE-120
	menubar->setEnabled(true);

  // FIXME: enabling menubar again. It's part of a quickfix, described here: https://its.cern.ch/jira/browse/ATLASVPONE-120
  menubar->setEnabled(true);
}

//_________________________________________________________________________________
void GXMainWindow::request_saveasConfig() {

	QString filename = QFileDialog::getSaveFileName(this, "Select configuration file to save",
			(m_currentconfigfile.isEmpty()?VP1Settings::defaultFileSelectDirectory():m_currentconfigfile),
			"VP1 Configuration files (*.vp1)",0,QFileDialog::DontResolveSymlinks);
	if(filename.isEmpty())
		return;

	if (!filename.endsWith(".vp1"))
		filename += ".vp1";

	//m_tabmanager->saveConfigurationToFile(filename,false/*Since the filedialog already asks*/);
	m_currentconfigfile=filename;
}

//_________________________________________________________________________________
void GXMainWindow::request_saveConfig()
{
	if (m_currentconfigfile.isEmpty()) {
		request_saveasConfig();
		return;
	}
	//m_tabmanager->saveConfigurationToFile(m_currentconfigfile,false);
}

//_________________________________________________________________________________
void GXMainWindow::request_loadConfig()
{
	QString filename = QFileDialog::getOpenFileName(this, "Select configuration file to load",
			(m_currentconfigfile.isEmpty()?VP1Settings::defaultFileSelectDirectory():m_currentconfigfile),
			"VP1 configuration files (*.vp1)",0,QFileDialog::DontResolveSymlinks);
	if(filename.isEmpty())
		return;
	//m_tabmanager->loadConfigurationFromFile(filename,availablePluginFiles());
	m_currentconfigfile=filename;
}

//_________________________________________________________________________________
void GXMainWindow::request_loadPlugin()
{
/*
#ifdef __APPLE__
	QString sharedlibsuffix = "dylib";
#else
*/
	QString sharedlibsuffix = "so";
//#endif

  qDebug() << "GXMainWindow::request_loadPlugin()"<<m_currentloadpluginpath;

	QString filename = QFileDialog::getOpenFileName(this, "Select plugin file to load",
			m_currentloadpluginpath,
			"VP1 plugin files (*VP1*."+sharedlibsuffix+")",0,QFileDialog::DontResolveSymlinks);
	if(filename.isEmpty())
		return;
	m_currentloadpluginpath = QFileInfo(filename).dir().absolutePath();
	loadPluginFile(filename);
}

//_________________________________________________________________________________
QMap<QString,QString> GXMainWindow::availableFiles(const QString& extension,
		const QString& pathvar,//LD_LIBRARY_PATH or DATAPATH
		const QString& instareasubdir,
		const QString& extradirenvvar,
		bool currentdir ) const
{

    qDebug() << "GXMainWindow::availableFiles()";
 	qDebug() << "extension:" << extension << "pathvar:" << pathvar << "instareasubdir:" << instareasubdir << "extradirenvvar:" << extradirenvvar << "currentdir:" << currentdir;


	//Add directories from extradirenvvar (e.g. $GXPLUGINPATH)
    qDebug() << "GXPLUGINPATH: " << QString(::getenv(extradirenvvar.toStdString().c_str()));
	QStringList vp1pluginpath = extradirenvvar.isEmpty() ? QStringList() : QString(::getenv(extradirenvvar.toStdString().c_str())).split(":",Qt::SkipEmptyParts);
    if(VP1Msg::debug()){
      qDebug() << "extradirenvvar:" << extradirenvvar;
      qDebug()  << "vp1pluginpath A :" << vp1pluginpath;
  	}

	//Currentdir:
	if (currentdir) {
		vp1pluginpath<<QDir::currentPath();
		if (QDir::currentPath()!=VP1Settings::defaultFileSelectDirectory())
			vp1pluginpath<<VP1Settings::defaultFileSelectDirectory();
	}
    qDebug()  << "vp1pluginpath B :" << vp1pluginpath;
    
	//Add directories from pathvar (looking in subdir instareasubdir):
	QString varStr = QString(::getenv(pathvar.toStdString().c_str()));
	//VP1Msg::messageDebug("Add directories from pathvar... " + pathvar + " - " + varStr);
	QString path = QString(::getenv(pathvar.toStdString().c_str()));
	if (!path.isEmpty()) {
		//!instareasubdir.isEmpty()&&
		QStringList tmp = path.split(":",Qt::SkipEmptyParts);//This 'tmp' is for SLC3 compilation.
		foreach (QString dir,tmp) {
			vp1pluginpath << ( instareasubdir.isEmpty() ? dir : dir+QDir::separator()+QDir::separator()+instareasubdir );
		}
	}
    qDebug()  << "vp1pluginpath C :" << vp1pluginpath;

	//Remove all nonexisting directories:
	foreach (QString plugindir, vp1pluginpath) {
		QFileInfo fi(plugindir);
		if (!fi.exists()||!fi.isDir()) {
			vp1pluginpath.removeAll(plugindir);
		}
	}
    qDebug()  << "vp1pluginpath D :" << vp1pluginpath;

	//Find all files with required extension in the directories (in case of duplicates - the ones appearing first are used):
	QMap<QString,QString> plugins2fullpath;
	foreach (QString plugindir, vp1pluginpath) {
        qDebug() << "plugindir: " << plugindir;
		QStringList plugins = QDir(plugindir).entryList((QStringList()<<("*"+extension)),QDir::CaseSensitive | QDir::Files | QDir::Readable,QDir::Name);
		foreach (QString plugin, plugins) {
            qDebug() << "plugin: " << plugin;
			plugin = QFileInfo(plugin).fileName();
			if (!plugins2fullpath.contains(plugin)) {
				QString fullpath = plugindir+QDir::separator()+plugin;
				plugins2fullpath[plugin]=fullpath;
			}
		}
	}
	return plugins2fullpath;
}

//_________________________________________________________________________________
QMap<QString,QString> GXMainWindow::availablePluginFiles() const
{
VP1Msg::messageDebug("GXMainWindow::availablePluginFiles()");

//#ifdef __APPLE__
//	QString sharedlibsuffix = "dylib";
//#else
	QString sharedlibsuffix = "so";
//#endif
	return availableFiles( "."+sharedlibsuffix, "LD_LIBRARY_PATH", "gxplugins", "GXPLUGINPATH" );

}

//_________________________________________________________________________________
void GXMainWindow::pluginDialogClosed() {
	if (!m_plugindialog)
		return;

	int res = m_plugindialog->result();
	QString filename = m_plugindialog->unloadfile();
	disconnect(m_plugindialog,SIGNAL(finished(int)),this,SLOT(pluginDialogClosed()));
	m_plugindialog->deleteLater();
	m_plugindialog = 0;

	if (res!=QDialog::Accepted||filename.isEmpty())
		return;

	//How many channels would be affected by such unloading?
	QStringList bns = m_channelmanager->channelsInPluginFile(filename);
	int naffected(0);
	foreach (QString bn, bns)
	naffected += m_channelmanager->nActive(bn);

	foreach (QString bn, bns) {
		while(m_channelmanager->basename2UniqueNames(bn).count()>0)
			m_tabmanager->removeChannel(m_channelmanager->basename2UniqueNames(bn).value(0));
	}

	m_currentunloadpluginfiles << filename;
	QTimer::singleShot(0, this, SLOT(unloadPlugin_continue()));

}

//_________________________________________________________________________________
void GXMainWindow::unloadPlugin_continue()
{
	foreach (QString filename, m_currentunloadpluginfiles) {
		bool success = m_channelmanager->unloadPluginFile(filename);
		if (!success)
			QMessageBox::critical(0, "Error - problems unloading plugin file: "+filename,
					"Problems encountered while attempting to unload plugin file: "+filename,QMessageBox::Ok,QMessageBox::Ok);
	}
	m_currentunloadpluginfiles.clear();
}




//_________________________________________________________________________________
void GXMainWindow::closeEvent(QCloseEvent * event)
{
	VP1Msg::messageDebug("GXMainWindow::closeEvent()");

	hide();

	VP1Msg::messageDebug("calling tabmanager->setSelectedDockWidget(0)...");
	m_tabmanager->setSelectedDockWidget(0);
	VP1Msg::messageDebug("tabmanager->setSelectedDockWidget(0) called.");

	m_mustquit=true; // this will inform VP1Alg that we want to quit VP1 (then we'll quit the Athena algorithm)
	VP1Msg::messageDebug("calling qApp->quit()...");
	qApp->quit();
}


//_________________________________________________________________________________
void GXMainWindow::addToMessageBox( const QString& m, const QString& extrastyleopts,
		const QString& title, const QString& titleextrastyleopts )
{

	if (title.isEmpty())
		std::cout<<VP1Msg::prefix_msg()<<": "<<m.toStdString()<<std::endl;
	else
		std::cout<<VP1Msg::prefix_msg()<<": "<<title.toStdString() << ": " <<m.toStdString()<<std::endl;


	QString titlepart = ( title.isEmpty() ? "" : ( titleextrastyleopts.isEmpty() ? title
			: "<span style=\"font-style:italic;"+titleextrastyleopts+";\">["+title+"]</span> "));

	textBrowser_channelmessages->append(titlepart
			+ ( extrastyleopts.isEmpty() ? m
					: "<span style=\"font-style:italic;"+extrastyleopts+";\">"+m+"</span>"));
	textBrowser_channelmessages->verticalScrollBar()->setSliderPosition(textBrowser_channelmessages->verticalScrollBar()->maximum());
}

//_________________________________________________________________________________
void GXMainWindow::systemAddToMessageBox( const QString& m )
{
	IVP1System*sys = static_cast<IVP1System*>(sender());
	if (!sys) {
		addToMessageBox("GXMainWindow::systemAddToMessageBox Error: Only prints system messages!");
		return;
	}
	if (!sys->channel()) {
		addToMessageBox("GXMainWindow::systemAddToMessageBox Error: System does not know its channel!");
		return;
	}
	addToMessageBox( m, "color:#000000",sys->channel()->unique_name()+"/"+sys->name(),"color:#0000ff" );
}

//_________________________________________________________________________________
void GXMainWindow::channelAddToMessageBox( const QString& m )
{
	IVP1ChannelWidget*cw = static_cast<IVP1ChannelWidget*>(sender());
	if (!cw) {
		addToMessageBox("GXMainWindow::channelAddToMessageBox Error: Only prints channelwidget messages!");
		return;
	}
	addToMessageBox(m,"color:#000000", cw->unique_name(),"color:#0000ff");
}

//_________________________________________________________________________________
void GXMainWindow::helperAddToMessageBox( const QString& m )
{
	addToMessageBox(m);
}

//_________________________________________________________________________________
void GXMainWindow::request_channelInformation() {
	if(!m_tabmanager->selectedChannelWidget())
		return;

	QString out = "Information about channel: "+m_tabmanager->selectedChannelWidget()->name() + "\n\n";
	out += "Contact: "+m_tabmanager->selectedChannelWidget()->contact_info()+"\n";
	out += "Information: "+m_tabmanager->selectedChannelWidget()->information()+"\n";
	out += "Systems:\n\n";
	std::set<IVP1System *>::iterator itsys, itsysE = m_tabmanager->selectedChannelWidget()->systems().end();
	for (itsys = m_tabmanager->selectedChannelWidget()->systems().begin();itsys!=itsysE;++itsys) {
		out += "  ==> System "+(*itsys)->name()+"\n";
		out += "      Contact: "+(*itsys)->contact_info()+"\n";
		out += "      Information: "+(*itsys)->information()+"\n";
		out += "\n";
	}

	QMessageBox::information(0, "Information about channel: "+m_tabmanager->selectedChannelWidget()->name(),Qt::convertFromPlainText(out),QMessageBox::Ok,QMessageBox::Ok);
}

//_________________________________________________________________________________
void GXMainWindow::makeAllChannelsEventDisplay()
{
	VP1Msg::messageVerbose("GXMainWindow::makeAllChannelsEventDisplay()");

	QList<QPixmap> list;
	QStringList listNames;
	QList<unsigned long long> listRunEventNumberTimestamp;

	getAllChannelsIntoSnapshots(list, listNames);

}




//_________________________________________________________________________________
void GXMainWindow::getAllChannelsIntoSnapshots(QList<QPixmap>& list, QStringList& listNames)
{
	VP1Msg::messageDebug("GXMainWindow::getAllChannelsIntoSnapshots()");

//	int nTabs = m_tabmanager->nTabs();
	QList<IVP1ChannelWidget*> allTabs = m_tabmanager->allChannels();

	if (allTabs.isEmpty()) {
		VP1Msg::message("WARNING - No tabs to save.");
		return;
	}

	int nT = 0;

	// loop over all tabs/channels
	foreach(IVP1ChannelWidget* widg, allTabs) {

		// increase tab number
		++nT;

		// get channel name (e.g. Geometry, 3DCocktail)
//		QString channelname = widg->unique_name().toLower();
		QString channelname = m_tabmanager->channelToTab(widg);
		channelname.replace(' ','_');
		VP1Msg::messageDebug("tab: " + channelname);

//		// get channel info, only for debug
//		QString info = widg->information();
//		VP1Msg::messageDebug("channel info: " + info);


		QPixmap snap = getSingleChannelCustomSnapshot(widg);

		list << snap;
		listNames << channelname;

	}

	VP1Msg::messageVerbose(QString::number(nT)+" tabs/channels saved in the QList.");

}

//_________________________________________________________________________________
QPixmap GXMainWindow::getSingleChannelCustomSnapshot(IVP1ChannelWidget* tab, int width)
{
	VP1Msg::messageDebug("GXMainWindow::getSingleChannelCustomSnapshot()");

	std::cout << "tab: " << tab << std::endl;

	// save the anti-aliasing status, set by the user
	bool antialias_original = tab->isAntiAliasing();

	// for the final event display images we want the anti-aliasing turned ON,
	tab->setAntiAliasing(true);

	// get the snapshot
	QPixmap snap;
	if (width)
		snap = tab->getSnapshot(true, width); // 'true' means 'transparent background'
	else
		snap = tab->getSnapshot(true); // 'true' means 'transparent background'

	if (snap.isNull()) {
		VP1Msg::message("ERROR! - No snapshot produced!");
		return QPixmap();
	}

	// go back to the original anti-aliasing status, set by the user for the tab
	tab->setAntiAliasing(antialias_original);

	return snap;
}
//_________________________________________________________________________________
QPixmap GXMainWindow::getSingleChannelCustomSnapshot(QString tabName, int width)
{
	QList<IVP1ChannelWidget*> allTabs = m_tabmanager->allChannels();

	if (allTabs.isEmpty()) {
		VP1Msg::message("WARNING - No tabs to get snapshots from.");
	}

	// loop over all tabs/channels
	foreach(IVP1ChannelWidget* widg, allTabs) {

		// get channel name (e.g. Geometry, 3DCocktail)
		QString channelname = m_tabmanager->channelToTab(widg);
		channelname.replace(' ','_');

		if (channelname == tabName) {
			VP1Msg::messageDebug("found tab: " + channelname);

			if (width)
				return getSingleChannelCustomSnapshot(widg, width);
			else
				return getSingleChannelCustomSnapshot(widg);

		}
	}
	return QPixmap();
}

//_________________________________________________________________________________
void GXMainWindow::saveAllCurrentChannels()
{
	VP1Msg::messageDebug("GXMainWindow::saveAllCurrentChannels()");

	int nTabs = m_tabmanager->nTabs();

	VP1Msg::messageDebug("# of tabs: " + QString::number(nTabs));

	QList<IVP1ChannelWidget*> allTabs = m_tabmanager->allChannels();

	if (allTabs.isEmpty()) {
		VP1Msg::message("WARNING - No tabs to save.");
		return;
	}


//	QString guess;
//	QString chnlname = m_tabmanager->selectedChannelWidget()->name().toLower();
//	chnlname.replace(' ','_');

	QString base=m_currentsaveimagepath+QDir::separator()+"gmex";


	// check for existing files
	int i(2);
	while (QFile::exists(base+".png")) {
		base = base+"_"+QString::number(i++);
	}

	// let the user choose the base filename
	QString filename = QFileDialog::getSaveFileName(0, "Select target image file", base,
			"Image (*.png *.bmp)",
			0,QFileDialog::DontResolveSymlinks);
	if(filename.isEmpty()) {
		VP1Msg::message("WARNING - No filename selected.");
		return;
	}

	m_currentsaveimagepath = QFileInfo(filename).dir().absolutePath ();


	QStringList tab_save_files;

	int nT = 0;
	foreach(IVP1ChannelWidget* widg, allTabs) {

		// increase tab number
		++nT;

		// get channel name (e.g. Geometry, 3DCocktail)
		QString channelname = widg->name().toLower();
		channelname.replace(' ','_');
		VP1Msg::messageDebug("tab: " + channelname);

//		// get channel info, only for debug
//		QString info = widg->information();
//		VP1Msg::messageDebug("channel info: " + info);

		QPixmap snap = widg->getSnapshot();
		if (snap.isNull()) {
			VP1Msg::message("ERROR! - No snapshot produced!");
			return;
		}

		QString tabfilename = filename + "_" + QString::number(nT) + "_" + channelname + ".png";

		if (!(tabfilename.endsWith(".png",Qt::CaseInsensitive)||tabfilename.endsWith(".bmp",Qt::CaseInsensitive)))
			tabfilename += ".png";


		snap.save(tabfilename);

		tab_save_files << tabfilename;
	}

	VP1Msg::message("Tabs saved as: " + tab_save_files.join(" - "));


    QMessageBox::information(this, tr("Snapshots saved."),
                         "All tabs have been saved as snapshots in the following files:\n\n"+tab_save_files.join("\n"),
	                     QMessageBox::Ok,
	                     QMessageBox::Ok);


//	// TODO: test to create a multilayered PSD or TIFF file
//	// but doesn't work with the ImageMagick version shipped with SLC5...
//	// So I give it up now...
//	QProcess *proc = new QProcess();
//	QString program = "convert";
//	QStringList arguments;
//	arguments = tab_save_files;
//	arguments << " " + m_currentsaveimagepath + QDir::separator() + "out.psd";
//	VP1Msg::messageDebug("running: " + program + " " + arguments.join(" "));
//
//	// start the process
//	proc->start(program, arguments );
//
//	if (!proc->waitForStarted())
//		return;
//
//	if (!proc->waitForFinished())
//		return;

}

//_________________________________________________________________________________
/*
 * save a snaphot of the currently selected tab.
 *
 * nsnap is an optional parameter: it's an extra label being added
 * to the output filename.
 */
QString GXMainWindow::request_saveChannelSnapshot(QString xLabel)
{

	VP1Msg::messageDebug("GXMainWindow::request_saveChannelSnapshot()");

	if(!m_tabmanager->selectedChannelWidget()) {
		return QString();
	}

	QString guess;
	QString chnlname = m_tabmanager->selectedChannelWidget()->name().toLower();
	chnlname.replace(' ','_');

	QString base=m_currentsaveimagepath+QDir::separator()+"gmex_"+chnlname;


	// check for existing files
	guess=base;
	int i(2);
	while (QFile::exists(guess+".png")) {
		guess=base+"_"+QString::number(i++);
	}
	guess+=".png";


	// adding the extra label xLabel, if provided
	if (!xLabel.isEmpty()) {
		guess += "_" + xLabel;
	}
        QString filename;
	{
	  QFileDialog fileDialog(0);
	  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	  fileDialog.setNameFilter("Image (*.png *.bmp)");
	  fileDialog.selectFile(guess);
	  
	  if (fileDialog.exec()) {
	    filename=fileDialog.selectedFiles()[0];
	  }
	  else {
	    return QString();
	  }
	  raise();
	  qApp->sync();
	}
	//	QString filename = QFileDialog::getSaveFileName(0, "Select target image file", guess,
	//			"Image (*.png *.bmp)",
	//			0,QFileDialog::DontResolveSymlinks);
	if(filename.isEmpty())
	  return QString();
	
	m_currentsaveimagepath = QFileInfo(filename).dir().absolutePath ();

	if (!(filename.endsWith(".png",Qt::CaseInsensitive)||filename.endsWith(".bmp",Qt::CaseInsensitive)))
		filename += ".png";
	
	VP1Msg::messageVerbose("calling snapshot");
	QPixmap pm = m_tabmanager->selectedChannelWidget()->getSnapshot();
	
	if (pm.isNull())
		return QString();

	pm.save(filename);
	return filename;

}

//_________________________________________________________________________________
void GXMainWindow::request_printChannel() {
	if(!m_tabmanager->selectedChannelWidget())
		return;

	//The following will paint the widget onto a paper and bring up the print dialog:
	QPrinter printer;
	QPrintDialog dialog(&printer, this);
	if (dialog.exec() == QDialog::Accepted) {
		QPixmap pm = m_tabmanager->selectedChannelWidget()->getSnapshot();
		if (pm.isNull())
			return;
		QPainter painter;
		painter.begin(&printer);
		painter.drawPixmap(0,0,pm);
		painter.end();
	}
}


//_________________________________________________________________________________
void GXMainWindow::loadConfigurationFromFile(QString file) {
  //m_tabmanager->loadConfigurationFromFile(file,availablePluginFiles());
}

//_________________________________________________________________________________
void GXMainWindow::replaceConfigurationFile(QString file)
{
	VP1Msg::messageDebug("GXMainWindow::replaceConfigurationFile() : " + file);
	m_tabmanager->removeAllTabs();
	//	m_tabmanager->loadConfigurationFromFile(file,availablePluginFiles());
}



//_________________________________________________________________________________
void GXMainWindow::showMenu_loadPlugin()
{
	m_menu_loadPlugin->clear();

	QMap<QString,QString> plugins2fullpath = availablePluginFiles();

	if (plugins2fullpath.empty()) {
		m_menu_loadPlugin->addAction("No plugins found")->setEnabled(false);
		return;
	}

	QStringList pluglist(plugins2fullpath.keys());
	pluglist.sort();

	QStringList currentpluginfiles = m_channelmanager->currentPluginFiles();
	foreach(QString plug,pluglist) {
		QAction * act = m_menu_loadPlugin->addAction(plug);
		assert(plugins2fullpath.contains(plug));
		QString fullpath = plugins2fullpath[plug];
		if (currentpluginfiles.contains(fullpath)) {
			act->setEnabled(false);
		} else {
			act->setData(fullpath);
			act->setStatusTip(fullpath);
			connect(act,SIGNAL(triggered(bool)),this,SLOT(showMenu_loadPluginItemSelected()));
		}
	}

	m_menu_loadPlugin->addSeparator();
	QAction * act_browse = m_menu_loadPlugin->addAction("&Browse...");
	act_browse->setStatusTip("Browse filesystem for VP1 plugin files");
	connect(act_browse,SIGNAL(triggered(bool)),this,SLOT(request_loadPlugin()));

}

//_________________________________________________________________________________
void GXMainWindow::showMenu_loadPluginItemSelected()
{
	QAction * act = static_cast<QAction*>(sender());
	assert(act);
	if (!act)
		return;
	loadPluginFile(act->data().toString());
}


//_________________________________________________________________________________
void GXMainWindow::showMenu_loadConfFile()
{
	m_menu_loadConfFile->clear();

	QMap<QString,QString> conffile2fullpath =  availableFiles( ".vp1", "DATAPATH", "", "VP1CONFIGFILEPATH", true );

	#ifndef BUILDVP1LIGHT
		if (conffile2fullpath.empty()) {
			m_menu_loadConfFile->addAction("No .vp1 config files found")->setEnabled(false);
			return;
		}

	QStringList filelist(conffile2fullpath.keys());
	filelist.sort();

	foreach(QString file,filelist) {
		QAction * act = m_menu_loadConfFile->addAction(file);
		assert(conffile2fullpath.contains(file));
		QString fullpath = conffile2fullpath[file];
		act->setData(fullpath);
		act->setStatusTip(fullpath);
		connect(act,SIGNAL(triggered(bool)),this,SLOT(showMenu_loadConfFileItemSelected()));
	}
	#endif

	m_menu_loadConfFile->addSeparator();
	QAction * act_browse = m_menu_loadConfFile->addAction("&Browse...");
	act_browse->setStatusTip("Browse filesystem for .vp1 config file");
	connect(act_browse,SIGNAL(triggered(bool)),this,SLOT(request_loadConfig()));

}


//_________________________________________________________________________________
void GXMainWindow::showMenu_loadConfFileItemSelected()
{
	QAction * act = static_cast<QAction*>(sender());
	assert(act);
	if (!act)
		return;
	loadConfigurationFromFile(act->data().toString());
}

//_________________________________________________________________________________
void GXMainWindow::changeStyleActionTriggered()
{
	QAction * act = static_cast<QAction*>(sender());
	assert(act);
	if (!act)
		return;
	assert(QStyleFactory::keys().contains(act->text()));
	if (!QStyleFactory::keys().contains(act->text()))
		return;
	QApplication::setStyle(QStyleFactory::create(act->text()));
	QSettings s(m_settingsfile,QSettings::IniFormat);
	s.setValue("style/defaultstyle",act->text());

}

//_________________________________________________________________________________
void GXMainWindow::changeFontSizeActionTriggered()
{
	QAction * act = static_cast<QAction*>(sender());
	assert(act);
	if (!act)
		return;
	bool ok;
	int goal = act->data().toInt(&ok);
	assert(ok);
	if (!ok)
		return;
	changeFontSize(goal);
}

//_________________________________________________________________________________
void GXMainWindow::changeFontSize(int goal)
{
	QFont newfont = m_defaultfont;
	QSettings s(m_settingsfile,QSettings::IniFormat);
	if (m_defaultfont_pointsize<0.0) {
		//pixels
		assert(m_defaultfont_pixelsize+goal>0);
		if (m_defaultfont_pixelsize+goal<=0)
			return;
		if (m_defaultfont_pixelsize+goal>200)
			return;
		newfont.setPixelSize(m_defaultfont_pixelsize+goal);
		s.setValue("font/pixelsize_diff",goal);
	} else {
		//pointsize
		double fact = (goal+100.0)/100.0;
		if (fact<0.01||fact>50)
			return;
		double newpointsize=m_defaultfont_pointsize*fact;
		if (newpointsize<0)
			return;
		newfont.setPointSizeF(newpointsize);
		s.setValue("font/pointsize_relativepercent",goal);
	}
	QApplication::setFont(newfont);
}


//_________________________________________________________________________________
void GXMainWindow::updateCentralStackWidget()
{
	QWidget * targetpage = tabWidget_central->count() ? page_tabwidget : page_instructions;
	if (stackedWidget_central->currentWidget() != targetpage)
		stackedWidget_central->setCurrentWidget(targetpage);
}

//_________________________________________________________________________________
void GXMainWindow::quickSetupTriggered()
{
  VP1Msg::messageVerbose("GXMainWindow::quickSetupTriggered()");

  QSettings settings("ATLAS", "gmex");

  QString plugfile, channelname, tabname;

  {
#ifdef BUILDVP1LIGHT
    if(settings.value("db/dbByEnv").toString().isEmpty()){
      VP1GeoDBSelection dbSelection;
      dbSelection.exec();
      if(!dbSelection.result())
        return;
    }
#endif // BUILDVP1LIGHT

    plugfile="libGXGeometryPlugin.so";
    channelname="Geometry";
    tabname = "Geometry";
  } 

/*
#ifdef __APPLE__
  if (plugfile.endsWith(".so"))
    plugfile = plugfile.left(plugfile.count()-3)+".dylib";
#endif
*/

	//Check that the plugin is available:
	QMap<QString,QString> plugins2fullpath = availablePluginFiles();
	if(VP1Msg::debug()){
		qDebug() << "plugins2fullpath: " << plugins2fullpath;
	}

	if (!plugins2fullpath.contains(plugfile)) {
		QMessageBox::critical(0, "Error - could not locate plugin file: "+plugfile,
				"Could not locate the plugin file: "
				+plugfile,QMessageBox::Ok,QMessageBox::Ok);
		return;
	}
	QString plugfile_fullpath = plugins2fullpath[plugfile];


	//Load plugin
	if (!m_channelmanager->currentPluginFiles().contains(plugfile_fullpath)) {
		QString err = m_channelmanager->loadPluginFile(plugfile_fullpath);
		if (!err.isEmpty()||!m_channelmanager->currentPluginFiles().contains(plugfile_fullpath)) {
			QMessageBox::critical(0, "Error - could not load plugin file: "+plugfile_fullpath,//Fixme: Error message here is hardcoded to be the same as in loadPluginFile method!!
					"Could not load the plugin file: "
					+plugfile_fullpath+"\n\nReason: "+err,QMessageBox::Ok,QMessageBox::Ok);
			return;
		}
	}


	//Check that plugin contains necessary channel:
	if (!m_channelmanager->channelsInPluginFile(plugfile_fullpath).contains(channelname)) {
		QMessageBox::critical(0, "Error - did not find necessary channel: "+channelname,
				"Could not find channel: "+channelname+" in loaded plugin "+plugfile_fullpath,
				QMessageBox::Ok,QMessageBox::Ok);
		return;
	}

	bool save = updatesEnabled();
	setUpdatesEnabled(false);


	//Add tab:
	QString newtabname = m_tabmanager->suggestNewTabName(tabname);
	m_tabmanager->addNewTab(newtabname);
	if (!m_tabmanager->hasTab(newtabname)) {
		QMessageBox::critical(0, "Error - could not create tab: "+newtabname,
				"Could not create tab: "+newtabname,
				QMessageBox::Ok,QMessageBox::Ok);
		setUpdatesEnabled(save);
		return;
	}


	//Finally, add channel:

	if (!m_tabmanager->addChannelToTab( channelname, newtabname )) {
		QMessageBox::critical(0, "Error - problems launching channel: "+channelname,
				"Problems launching channel: "+channelname,
				QMessageBox::Ok,QMessageBox::Ok);
		setUpdatesEnabled(save);
		return;
	}


	m_tabmanager->showTab(newtabname);


	setUpdatesEnabled(save);
}

//_________________________________________________________________________________
void GXMainWindow::updateEventControls()
{
  //	pushButton_nextevent->setEnabled(okToProceedToNextEvent());
	#if defined BUILDVP1LIGHT
		pushButton_previousevent->setEnabled(okToProceedToNextEvent());
	#endif
}


//________________________________________________________________________________
QStringList GXMainWindow::userRequestedFiles()
{
	QStringList returnval(m_userRequestedFiles);
	m_userRequestedFiles.clear();
	return returnval;
}

