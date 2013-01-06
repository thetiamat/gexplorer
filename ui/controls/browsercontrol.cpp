#include <QtCore/QDebug>
#include <QtCore/QDir>

#include <QtGui/QContextMenuEvent>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QMenu>

#include <QtWebKit/QWebElement>

#include <QtWebKitWidgets/QWebFrame>

#include "application.h"

#include "data/galleryitemdata.h"
#include "data/gallerydata.h"

#include "handlers/galleryhandler.h"

#include "ui/common/browsernetworkaccessmanager.h"

#include "ui/dialogs/downloaddialog.h"
#include "ui/dialogs/searchdialog.h"

#include "ui/controls/browserviewcontrol.h"
#include "ui/controls/titlecontrol.h"

#include "ui/controls/browsercontrol.h"
#include "ui_browsercontrol.h"

BrowserControl::BrowserControl(QWidget* parent): QWidget(parent), gallery(NULL), web(NULL), title(NULL), ui(new Ui::BrowserControl)
{
    ui->setupUi(this);

    title = new TitleControl(this);

    web = new BrowserViewControl(this);
    web->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    web->setContextMenuPolicy(Qt::NoContextMenu);
    //web->installEventFilter(this);
    connect(web, SIGNAL(onTopHref()), this, SLOT(topHrefEvent()));

    createMenuAndActions();
    updateButtons();

    QLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(title);
    layout->addWidget(web);
    setLayout(layout);
}

BrowserControl::~BrowserControl()
{
    delete ui;
}

TitleControl* BrowserControl::getTitle()
{
    return title;
}

void BrowserControl::goTo(GalleryData* gallery)
{
    this->gallery = gallery;
    web->load(QUrl(gallery->getSource()));
    updateButtons();
}

void BrowserControl::goTo(const QString& url)
{
    this->gallery = NULL;
    web->load(QUrl(url));
    updateButtons();
}

/*bool BrowserControl::eventFilter(QObject* object, QEvent* event)
{
    bool isHandled = false;

    if(gallery && event->type() == QEvent::MouseButtonPress) {
        isHandled = true;
    }
    if(!isHandled) {
        return QWidget::eventFilter(object, event);
    }
    return true;

    return QWidget::eventFilter(object, event);
}*/

void BrowserControl::contextMenuEvent(QContextMenuEvent* event)
{
    topHref = web->getTopHref();

    menuX = event->x() - web->pos().x();
    menuY = event->y() - web->pos().y();

    mMenu->exec(event->globalPos());
}

void BrowserControl::addDeletedGalleryEvent()
{
    GalleryData* data = new GalleryData(GalleryData::DeletedGallery);
    const QUrl& url = web->url();

    data->setSource(url.toString());
    data->setPath("");

    GalleryHandler::getInstance()->addWebGallery(*data);

    delete data;
}

void BrowserControl::addGalleryEvent()
{
    GalleryData* data = new GalleryData(GalleryData::WebGallery);
    const QUrl& url = web->url();

    data->setSource(url.toString());

    QString urlPath;
    QString urlExtension;
    QString topHref = web->getTopHref(menuX, menuY);
    //qDebug() << web->getTopHref();

    parseUrl(topHref, urlPath, urlExtension);
    //qDebug() << urlPath << ", " << urlExtension;

    SearchDialog* search = new SearchDialog(this, url.resolved(QUrl(topHref)).toString(), urlPath, urlExtension);
    search->show();
    search->exec();
    if(search->result() == QDialog::Accepted) {
        urlExtension = search->getExtension();
        urlPath = search->getPath();

        QWebElement element = web->page()->mainFrame()->documentElement();
        searchSameUrls(element, urlPath, urlExtension, data);
    }
    delete search;

    if(Application::getInstance()->getExtensionByName(urlExtension) != NULL) {
        if(urlPath.isEmpty()) {
            urlPath = "./";
        }

        QString p = url.host() + url.path();
        QString u = url.resolved(QUrl(urlPath)).toString();

        data->setPath(p);
        foreach(GalleryItemData* item, data->getItems()) {
            item->setUrl(u + item->getFileName());
            item->setPath(QDir("").path());
        }

        DownloadDialog* download = new DownloadDialog(this, *data);
        download->show();
        download->exec();
        if(download->result() == QDialog::Accepted) {
            data->setPath(download->getPath());
            GalleryHandler::getInstance()->addWebGallery(*data);
        }
        delete download;
    }
    delete data;
}

void BrowserControl::addStreamGalleryEvent()
{
    GalleryData* data = new GalleryData(GalleryData::WebGallery);
    const QUrl& url = web->url();

    data->setSource(url.toString());

    QString p = url.host() + url.path();
    data->setPath(p);

    foreach(const BrowserNetworkAccessManager::StreamData& d, web->getStreamData()) {
        QString u = url.resolved(QUrl(d.url)).toString(QUrl::RemoveQuery | QUrl::RemoveFragment);
        QFileInfo f(u);

        GalleryItemData* item = new GalleryItemData(data, *Application::getInstance()->getExtensionByName(d.ext));

        item->setFileName(f.fileName());
        item->setPath(QDir("").path());
        item->setUrl(d.url);

        //qDebug() << f.fileName();
   }

    DownloadDialog* download = new DownloadDialog(this, *data);
    download->show();
    download->exec();
    if(download->result() == QDialog::Accepted) {
        data->setPath(download->getPath());
        GalleryHandler::getInstance()->addWebGallery(*data);
    }
    delete download;
    delete data;
}

void BrowserControl::addItemsEvent()
{
    GalleryData* data = gallery->clone(false);
    const QUrl& url = web->url();

    QString urlPath;
    QString urlExtension;
    QString topHref = web->getTopHref(menuX, menuY);

    //qDebug() << topHref;

    parseUrl(topHref, urlPath, urlExtension);

    SearchDialog* search = new SearchDialog(this, url.resolved(QUrl(topHref)).toString(), urlPath, urlExtension);
    search->show();
    search->exec();
    if(search->result() == QDialog::Accepted) {
        urlExtension = search->getExtension();
        urlPath = search->getPath();

        QWebElement element = web->page()->mainFrame()->documentElement();
        searchSameUrls(element, urlPath, urlExtension, data);
    }
    delete search;

    if(Application::getInstance()->getExtensionByName(urlExtension) != NULL) {
        if(urlPath.isEmpty()) {
            urlPath = "./";
        }

        QString u = url.resolved(QUrl(urlPath)).toString();
        foreach(GalleryItemData* item, data->getItems()) {
            item->setUrl(u + item->getFileName());
            item->setPath(QDir("").path());
        }

        DownloadDialog* download = new DownloadDialog(this, *data);
        download->show();
        download->exec();

        if(download->result() == QDialog::Accepted) {
            int i = 0;
            while(i < data->getItems().count()) {
                GalleryItemData* di =  data->getItems()[i];

                bool isFind = false;
                foreach(GalleryItemData* gi, gallery->getItems()) {
                    if(!QString::compare(gi->getUrl(), di->getUrl(), Qt::CaseInsensitive)) {
                        isFind = true;
                        break;
                    }
                }

                if(isFind) {
                    delete di;
                } else {
                    i++;
                }
            }
            GalleryHandler::getInstance()->addToWebGallery(*data);
        }
        delete download;
    }
    delete data;
}

void BrowserControl::topHrefEvent()
{
    if(mMenu->isHidden()) {
        updateButtons();
    }
}

void BrowserControl::openEvent()
{
    goTo(topHref);
}

void BrowserControl::backEvent()
{
    QStack<QString>& history = web->getHistory();
    history.pop();

    goTo(history.top());
}

void BrowserControl::goEvent()
{
    goTo(lineEdit->text());
}

void BrowserControl::searchSameUrls(QWebElement& element, const QString& path, const QString& ext, GalleryData* data)
{
    foreach(QWebElement e, element.findAll("[href]")) {
        QString url = e.attribute("href");
        if(url.startsWith(path, Qt::CaseInsensitive) && url.endsWith(ext, Qt::CaseInsensitive)) {
            QString midle = url.mid(path.length(), url.length() - path.length() - ext.length());
            if(midle.contains("/")) {
                continue;
            }
            QString file = url.right(url.length() - path.length());
            if(!isContainsFile(data, file)) {
                GalleryItemData* item = new GalleryItemData(data, *Application::getInstance()->getExtensionByName(ext));
                item->setFileName(file);
            }
        }
    }
}

bool BrowserControl::isContainsFile(GalleryData* data, const QString& file)
{
    foreach(GalleryItemData* item, data->getItems()) {
        if(!file.compare(item->getFileName(), Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

void BrowserControl::parseUrl(const QString& url, QString& path, QString& ext)
{
    path = "";
    ext = ".";

    if(!url.isEmpty()) {
        QStringList list = url.split("/");
        QStringList listExt = list[list.count() - 1].split(".");
        if(listExt.count() > 1) {
            ext = list[list.count() - 1].right(list[list.count() - 1].length() - listExt[0].length());
        }
        path = url.left(url.length() - list[list.count() - 1].length());
    }
}

void BrowserControl::createMenuAndActions()
{
    title->getToolBar()->setVisible(true);

    mMenu = new QMenu(this);

    lineEdit = new QLineEdit(this);
    lineEdit->setMaximumHeight(30);
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    title->getToolBar()->addWidget(lineEdit);

    aGo = new QAction("&Go", this);
    aGo->setStatusTip("Go to url");
    aGo->setIcon(QIcon(":/res/resources/view.png"));
    title->getToolBar()->addAction(aGo);
    connect(aGo, SIGNAL(triggered()), this, SLOT(goEvent()));

    aBack = new QAction("&Back", this);
    aBack->setStatusTip("Back to url");
    aBack->setIcon(QIcon(":/res/resources/left.png"));
    title->getToolBar()->addAction(aBack);
    connect(aBack, SIGNAL(triggered()), this, SLOT(backEvent()));

    aOpen = new QAction("&Open", this);
    aOpen->setStatusTip("Open to url");
    aOpen->setIcon(QIcon(":/res/resources/gallery.png"));
    mMenu->addAction(aOpen);
    connect(aOpen, SIGNAL(triggered()), this, SLOT(openEvent()));

    mMenu->addSeparator();

    aAdd = new QAction("&Add gallery", this);
    aAdd->setStatusTip("Add gallery");
    aAdd->setIcon(QIcon(":/res/resources/gallery.png"));
    mMenu->addAction(aAdd);
    connect(aAdd, SIGNAL(triggered()), this, SLOT(addGalleryEvent()));

    aAddStream = new QAction("&Add stream gallery", this);
    aAddStream->setStatusTip("Add stream gallery");
    aAddStream->setIcon(QIcon(":/res/resources/gallery.png"));
    mMenu->addAction(aAddStream);
    connect(aAddStream, SIGNAL(triggered()), this, SLOT(addStreamGalleryEvent()));

    aAddItem = new QAction("&Add to gallery", this);
    aAddItem->setStatusTip("Add to gallery items");
    aAddItem->setIcon(QIcon(":/res/resources/add.png"));
    mMenu->addAction(aAddItem);
    connect(aAddItem, SIGNAL(triggered()), this, SLOT(addItemsEvent()));

    aAddDeleted = new QAction("&Add deleted", this);
    aAddDeleted->setStatusTip("Add gallery as Deleted");
    aAddDeleted->setIcon(QIcon(":/res/resources/gallery_delete.png"));
    title->getToolBar()->addAction(aAddDeleted);
    mMenu->addAction(aAddDeleted);
    connect(aAddDeleted, SIGNAL(triggered()), this, SLOT(addDeletedGalleryEvent()));
}

void BrowserControl::updateButtons()
{
    if(web->getHistory().count() > 1) {
        aBack->setEnabled(true);
    } else {
        aBack->setEnabled(false);
    }

    if(gallery) {
        aAddDeleted->setEnabled(false);

        bool isValid = false;

        QString urlPath;
        QString urlExtension;
        parseUrl(web->getTopHref(), urlPath, urlExtension);
        if(Application::getInstance()->getExtensionByName(urlExtension) != NULL) {
            isValid = true;
        }

        aAdd->setEnabled(false);
        aAddStream->setEnabled(false);
        if(isValid) {
            aAddItem->setEnabled(true);
        } else {
            aAddItem->setEnabled(false);
        }
    } else {
        aAdd->setEnabled(true);
        aAddStream->setEnabled(true);
        aAddItem->setEnabled(false);
        aAddDeleted->setEnabled(true);
    }
}
