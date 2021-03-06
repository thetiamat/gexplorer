#define DB_FILE_NAME "db.sqlite"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QVariant>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "application.h"

#include "data/customgalleryitemdata.h"
#include "data/customgallerydata.h"
#include "data/galleryitemdata.h"
#include "data/extensiondata.h"
#include "data/gallerydata.h"
#include "data/aliasdata.h"
#include "data/keydata.h"
#include "data/urldata.h"

#include "handlers/customgalleryhandler.h"
#include "handlers/galleryhandler.h"

#include "handlers/dbhandler.h"

DbHandler* DbHandler::handler = NULL;

DbHandler* DbHandler::createInstance(const QString& path)
{
    if(!handler) {
        DbHandler* tempHandler = new DbHandler();
        //tempHandler->db = QSqlDatabase::database("QSQLITE");
        tempHandler->db = QSqlDatabase::addDatabase("QSQLITE");
        tempHandler->db.setDatabaseName(path + QDir::separator() + DB_FILE_NAME);
        bool res = tempHandler->db.open();

        QSqlQuery query;
        if(res) {
            res = query.exec("PRAGMA foreign_keys = ON");
        }
        QSqlDatabase::database().transaction();

        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS statuses (id INTEGER PRIMARY KEY, name VARCHAR(256) NOT NULL)");
        }
        if(res) {
            res = query.exec("SELECT id FROM statuses");
        }
        if(res && !query.next()) {
            if(res) {
                res = query.exec("INSERT INTO statuses (id, name) VALUES (1, 'New')");
            }
            if(res) {
                res = query.exec("INSERT INTO statuses (id, name) VALUES (2, 'Download')");
            }
            if(res) {
                res = query.exec("INSERT INTO statuses (id, name) VALUES (3, 'Complete')");
            }
            if(res) {
                res = query.exec("INSERT INTO statuses (id, name) VALUES (4, 'Error')");
            }
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS gallery_types (id INTEGER PRIMARY KEY, name VARCHAR(256) NOT NULL)");
        }
        if(res) {
            res = query.exec("SELECT id FROM gallery_types");
        }
        if(res && !query.next()) {
            if(res) {
                res = query.exec("INSERT INTO gallery_types (id, name) VALUES (1, 'Web')");
            }
            if(res) {
                res = query.exec("INSERT INTO gallery_types (id, name) VALUES (2, 'File')");
            }
            if(res) {
                res = query.exec("INSERT INTO gallery_types (id, name) VALUES (3, 'Deleted')");
            }
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS extension_types (id INTEGER PRIMARY KEY, name VARCHAR(256) NOT NULL)");
        }
        if(res) {
            res = query.exec("SELECT id FROM extension_types");
        }
        if(res && !query.next()) {
            if(res) {
                res = query.exec("INSERT INTO extension_types (id, name) VALUES (1, 'Image')");
            }
            if(res) {
                res = query.exec("INSERT INTO extension_types (id, name) VALUES (2, 'Video')");
            }
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS extensions (id INTEGER PRIMARY KEY, name VARCHAR(256) NOT NULL, type INTEGER REFERENCES extension_types(id))");
        }
        if(res) {
            res = query.exec("SELECT id FROM extensions");
        }
        if(res && !query.next()) {
            if(res) {
                res = query.exec("INSERT INTO extensions (id, name, type) VALUES (1, '.jpg', 1)");
            }
            if(res) {
                res = query.exec("INSERT INTO extensions (id, name, type) VALUES (2, '.png', 1)");
            }
            if(res) {
                res = query.exec("INSERT INTO extensions (id, name, type) VALUES (3, '.wmv', 2)");
            }
            if(res) {
                res = query.exec("INSERT INTO extensions (id, name, type) VALUES (4, '.avi', 2)");
            }
            if(res) {
                res = query.exec("INSERT INTO extensions (id, name, type) VALUES (5, '.mpg', 2)");
            }
            if(res) {
                res = query.exec("INSERT INTO extensions (id, name, type) VALUES (6, '.flv', 2)");
            }
        }
        if(res) {
            res = query.exec("SELECT id FROM extensions WHERE id=7");
        }
        if(res && !query.next()) {
            res = query.exec("INSERT INTO extensions (id, name, type) VALUES (7, '.tif', 1)");
        }
        if(res) {
            res = query.exec("SELECT id FROM extensions WHERE id=8");
        }
        if(res && !query.next()) {
            res = query.exec("INSERT INTO extensions (id, name, type) VALUES (8, '.jpeg', 1)");
        }
        if(res) {
            res = query.exec("SELECT id FROM extensions WHERE id=9");
        }
        if(res && !query.next()) {
            res = query.exec("INSERT INTO extensions (id, name, type) VALUES (9, '.swf', 2)");
        }
        if(res) {
            res = query.exec("SELECT id FROM extensions WHERE id=10");
        }
        if(res && !query.next()) {
            res = query.exec("INSERT INTO extensions (id, name, type) VALUES (10, '.gif', 1)");
        }

        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS urls (id INTEGER PRIMARY KEY, source VARCHAR(1024) NOT NULL UNIQUE, name VARCHAR(1024) NOT NULL)");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS aliases (id INTEGER PRIMARY KEY, alias VARCHAR(1024) NOT NULL UNIQUE)");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS galleries (id INTEGER PRIMARY KEY, source VARCHAR(1024) NOT NULL UNIQUE, path VARCHAR(1024) NOT NULL, type INTEGER REFERENCES gallery_types(id))");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS items (id INTEGER PRIMARY KEY, url VARCHAR(1024) NOT NULL UNIQUE, path VARCHAR(1024) NOT NULL, filename VARCHAR(256) NOT NULL, extension_id INTEGER REFERENCES extensions(id), status_id INTEGER REFERENCES statuses(id), gallery_id INTEGER REFERENCES galleries(id) ON UPDATE CASCADE ON DELETE CASCADE)");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS item_to_item (id INTEGER REFERENCES items(id) ON UPDATE CASCADE ON DELETE CASCADE, item_id INTEGER REFERENCES items(id) ON UPDATE CASCADE ON DELETE RESTRICT, angle INTEGER DEFAULT 0, PRIMARY KEY(id))");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS custom_galleries (id INTEGER PRIMARY KEY, name VARCHAR(256) NOT NULL UNIQUE)");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS custom_items (id INTEGER PRIMARY KEY, custom_gallery_id INTEGER REFERENCES custom_galleries(id) ON UPDATE CASCADE ON DELETE CASCADE, item_id INTEGER REFERENCES items(id) ON UPDATE CASCADE ON DELETE CASCADE, name VARCHAR(256) NOT NULL, angle INTEGER DEFAULT 0, custom_item_id INTEGER NULL REFERENCES custom_items(id) ON DELETE SET NULL, UNIQUE(custom_gallery_id, item_id))");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS keys (id INTEGER PRIMARY KEY, name VARCHAR(256) NOT NULL UNIQUE)");
        }
        if(res) {
            res = query.exec("CREATE TABLE IF NOT EXISTS keys_to_custom_galleries (key_id INTEGER REFERENCES keys(id) ON UPDATE CASCADE ON DELETE CASCADE, custom_gallery_id INTEGER REFERENCES custom_galleries(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY(key_id, custom_gallery_id))");
        }
        checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
        if(res) {
            int r = QSqlDatabase::database().commit();
            checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
        } else {
            int r = QSqlDatabase::database().rollback();
            checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
        }
        if(res) {
            handler = tempHandler;
        } else {
            delete tempHandler;
        }
    }
    return handler;
}

DbHandler* DbHandler::getInstance()
{
    return handler;
}

DbHandler::~DbHandler()
{
    handler = NULL;
    if(db.isOpen()) {
        db.close();
    }
}

DbHandler::DbHandler() {
}

bool DbHandler::getGalleries(QList<GalleryData*>& galleries)
{
    bool res = true;
    QSqlQuery query;
    res = query.exec("SELECT * FROM galleries");
    while(query.next()) {
        GalleryData* gallery = new GalleryData((GalleryData::GalleryTypeEnum)query.value(3).toInt());
        gallery->setId(query.value(0).toInt());
        gallery->setSource(query.value(1).toString());
        gallery->setPath(query.value(2).toString());
        galleries.append(gallery);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

struct TempGalleryData
{
public:
    TempGalleryData(GalleryItemData* item, int gal_id, int item_id)
    {
        this->item = item;
        this->gal_id = gal_id;
        this->item_id = item_id;
    }

    GalleryItemData* item;
    int gal_id;
    int item_id;
};

bool DbHandler::getGalleryItems(Application* app, GalleryHandler* handler)
{
    QList<TempGalleryData> td;
    bool res = true;
    QSqlQuery query;

    res = query.exec("SELECT i.id, i.url, i.path, i.filename, i.extension_id, i.status_id, i.gallery_id, ii.item_id, ii.angle, g.id FROM items AS i LEFT JOIN item_to_item AS ii ON i.id = ii.id LEFT JOIN items AS i1 ON ii.item_id = i1.id LEFT JOIN galleries AS g ON i1.gallery_id = g.id");
    while(query.next()) {
        GalleryData* gallery = handler->getGalleryById(query.value(6).toInt());
        GalleryItemData* item = new GalleryItemData(gallery, *app->getExtensionById(query.value(4).toInt()));
        item->setId(query.value(0).toInt());
        item->setUrl(query.value(1).toString());
        item->setPath(query.value(2).toString());
        item->setFileName(query.value(3).toString());
        item->setStatus((GalleryItemData::GalleryItemStatusEnum)query.value(5).toInt());
        if(!query.value(7).isNull()) {
            td.append(TempGalleryData(item, query.value(9).toInt(), query.value(7).toInt()));
            item->setReferenceAngle(query.value(8).toInt());
        }
    }
    if(res) {
        foreach(const TempGalleryData& i, td) {
            i.item->setReference(handler->getGalleryItemById(i.gal_id, i.item_id));
        }
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::getCustomGalleries(QList<CustomGalleryData*>& customGalleries)
{
    bool res = true;
    QSqlQuery query;
    res = query.exec("SELECT * FROM custom_galleries");
    while(query.next()) {
        CustomGalleryData* gallery = new CustomGalleryData();
        gallery->setId(query.value(0).toInt());
        gallery->setName(query.value(1).toString());
        customGalleries.append(gallery);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::getCustomGalleryItems(CustomGalleryHandler* handler) {
    bool res = true;
    QSqlQuery query;
    res = query.exec("SELECT ci.id, ci.custom_gallery_id, ci.item_id, ci.name, ci.angle, ci.custom_item_id, g.id FROM custom_items AS ci LEFT JOIN items AS i ON ci.item_id = i.id LEFT JOIN galleries AS g ON i.gallery_id = g.id");
    while(query.next()) {
        CustomGalleryData* gallery = handler->getCustomGalleryById(query.value(1).toInt());
        GalleryItemData* item = GalleryHandler::getInstance()->getGalleryItemById(query.value(6).toInt(), query.value(2).toInt());
        CustomGalleryItemData* customItem = new CustomGalleryItemData(*gallery, *item);
        customItem->setId(query.value(0).toInt());
        customItem->setName(query.value(3).toString());
        customItem->setAngle(query.value(4).toInt());
        customItem->setCustomId(query.value(5).toInt());
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::getExtensions(QList<ExtensionData*>& extensions)
{
    bool res = true;
    QSqlQuery query;
    res = query.exec("SELECT * FROM extensions");
    while(query.next()) {
        ExtensionData* extension = new ExtensionData(query.value(0).toInt(), query.value(1).toString(), (ExtensionData::ExtensionTypeEnum)query.value(2).toInt());
        extensions.append(extension);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::getKeys(QList<KeyData*>& keys)
{
    bool res = true;
    QSqlQuery query;
    res = query.exec("SELECT * FROM keys");
    while(query.next()) {
        KeyData* key = new KeyData();
        key->setId(query.value(0).toInt());
        key->setName(query.value(1).toString());
        keys.append(key);
    }
    if(res) {
        foreach(KeyData* key, keys) {
            query.prepare("SELECT custom_gallery_id FROM keys_to_custom_galleries WHERE key_id = :key_id");
            query.bindValue(":key_id", key->getId());
            res = query.exec();
            while(query.next()) {
                CustomGalleryData* gallery = CustomGalleryHandler::getInstance()->getCustomGalleryById(query.value(0).toInt());
                key->galleries.append(gallery);
            }
            if(!res) {
                break;
            }
        }
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::getUrls(QList<UrlData*>& urls)
{
    bool res = true;
    QSqlQuery query;
    res = query.exec("SELECT * FROM urls");
    while(query.next()) {
        UrlData* url = new UrlData();
        url->setId(query.value(0).toInt());
        url->setSource(query.value(1).toString());
        url->setName(query.value(2).toString());
        urls.append(url);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::getAliases(QList<AliasData*>& aliases)
{
    bool res = true;
    QSqlQuery query;
    res = query.exec("SELECT * FROM aliases");
    while(query.next()) {
        AliasData* alias = new AliasData();
        alias->setId(query.value(0).toInt());
        alias->setAlias(query.value(1).toString());
        aliases.append(alias);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::addToKey(const KeyData& key, const CustomGalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("INSERT INTO keys_to_custom_galleries (custom_gallery_id, key_id) VALUES (:custom_gallery_id, :key_id)");
    query.bindValue(":custom_gallery_id", value.getId());
    query.bindValue(":key_id", key.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::addToCustomGallery(QList<CustomGalleryItemData*>& items)
{
    bool res = true;
    QSqlQuery query;

    QSqlDatabase::database().transaction();
    foreach(CustomGalleryItemData* item, items) {
        query.prepare("INSERT INTO custom_items (id, custom_gallery_id, item_id, name) VALUES (NULL, :custom_gallery_id, :item_id, :name)");
        query.bindValue(":custom_gallery_id", item->getCustomGallery()->getId());
        query.bindValue(":item_id", item->getItem().getId());
        query.bindValue(":name", item->getName());
        res = query.exec();
        if(res) {
            item->setId(query.lastInsertId().toInt());
        } else {
            break;
        }
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    if(res) {
        int r = QSqlDatabase::database().commit();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    } else {
        int r = QSqlDatabase::database().rollback();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    }
    return res;
}

bool DbHandler::addCustomGallery(CustomGalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    QSqlDatabase::database().transaction();
    query.prepare("INSERT INTO custom_galleries (id, name) VALUES (NULL, :name)");
    query.bindValue(":name", value.getName());
    res = query.exec();
    int id = 0;
    if(res) {
        id = query.lastInsertId().toInt();
    }
    if(res) {
        foreach (CustomGalleryItemData* item, value.getItems()) {
            query.prepare("INSERT INTO custom_items (id, custom_gallery_id, item_id, name) VALUES (NULL, :custom_gallery_id, :item_id, :name)");
            query.bindValue(":custom_gallery_id", id);
            query.bindValue(":item_id", item->getItem().getId());
            query.bindValue(":name", item->getName());
            res = query.exec();
            if(res) {
                item->setId(query.lastInsertId().toInt());
            } else {
                break;
            }
        }
    }
    if(res) {
        value.setId(id);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    if(res) {
        int r = QSqlDatabase::database().commit();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    } else {
        int r = QSqlDatabase::database().rollback();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    }
    return res;
}

bool DbHandler::addToGallery(GalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    QSqlDatabase::database().transaction();
    foreach (GalleryItemData* item, value.getItems()) {
        query.prepare("INSERT INTO items (id, url, path, filename, extension_id, status_id, gallery_id) VALUES (NULL, :url, :path, :filename, :extension_id, :status_id, :gallery_id)");
        query.bindValue(":url", item->getUrl());
        query.bindValue(":path", item->getPath());
        query.bindValue(":filename", item->getFileName());
        query.bindValue(":extension_id", item->getExtension().getId());
        query.bindValue(":status_id", item->getStatus());
        query.bindValue(":gallery_id", value.getId());
        res = query.exec();
        if(res) {
            item->setId(query.lastInsertId().toInt());
        } else {
            break;
        }
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    if(res) {
        int r = QSqlDatabase::database().commit();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    } else {
        int r = QSqlDatabase::database().rollback();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    }
    return res;
}

bool DbHandler::addGallery(GalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    QSqlDatabase::database().transaction();
    query.prepare("INSERT INTO galleries (id, source, path, type) VALUES (NULL, :source, :path, :type)");
    query.bindValue(":source", value.getSource());
    query.bindValue(":path", value.getPath());
    query.bindValue(":type", value.getType());
    res = query.exec();
    if(res) {
        value.setId(query.lastInsertId().toInt());
        foreach (GalleryItemData* item, value.getItems()) {
            query.prepare("INSERT INTO items (id, url, path, filename, extension_id, status_id, gallery_id) VALUES (NULL, :url, :path, :filename, :extension_id, :status_id, :gallery_id)");
            query.bindValue(":url", item->getUrl());
            query.bindValue(":path", item->getPath());
            query.bindValue(":filename", item->getFileName());
            query.bindValue(":extension_id", item->getExtension().getId());
            query.bindValue(":status_id", item->getStatus());
            query.bindValue(":gallery_id", value.getId());
            res = query.exec();
            if(res) {
                item->setId(query.lastInsertId().toInt());
            } else {
                //break;
            }
        }
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    if(res) {
        int r = QSqlDatabase::database().commit();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    } else {
        int r = QSqlDatabase::database().rollback();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    }
    return res;
}

bool DbHandler::addKey(KeyData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("INSERT INTO keys (id, name) VALUES (NULL, :name)");
    query.bindValue(":name", value.getName());
    res = query.exec();
    int id = 0;
    if(res) {
        id = query.lastInsertId().toInt();
    }
    if(res) {
        foreach(CustomGalleryData* gallery, value.getGalleries()) {
            query.prepare("INSERT INTO keys_to_custom_galleries (custom_gallery_id, key_id) VALUES (:custom_gallery_id, :key_id)");
            query.bindValue(":custom_gallery_id", gallery->getId());
            query.bindValue(":key_id", id);
            res = query.exec();
            if(!res) {
                break;
            }
        }
    }
    if(res) {
        value.setId(id);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::addUrl(UrlData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("INSERT INTO urls (id, source, name) VALUES (NULL, :source, :name)");
    query.bindValue(":source", value.getName());
    query.bindValue(":name", value.getSource());
    res = query.exec();
    int id = 0;
    if(res) {
        id = query.lastInsertId().toInt();
    }
    if(res) {
        value.setId(id);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::addAlias(AliasData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("INSERT INTO aliases (id, alias) VALUES (NULL, :alias)");
    query.bindValue(":alias", value.getAlias());
    res = query.exec();
    int id = 0;
    if(res) {
        id = query.lastInsertId().toInt();
    }
    if(res) {
        value.setId(id);
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delFromKey(const KeyData& key, const CustomGalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM keys_to_custom_galleries WHERE custom_gallery_id = :custom_gallery_id AND key_id = :key_id");
    query.bindValue(":custom_gallery_id", value.getId());
    query.bindValue(":key_id", key.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delFromCustomGallery(const CustomGalleryItemData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM custom_items WHERE id = :id");
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delCustomGallery(const CustomGalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM custom_galleries WHERE id = :id");
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delFromGallery(const GalleryItemData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM items WHERE id = :id");
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delGallery(const GalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM galleries WHERE id = :id");
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delKey(const KeyData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM keys WHERE id = :id");
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delUrl(const UrlData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM urls WHERE id = :id");
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::delAlias(const AliasData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("DELETE FROM aliases WHERE id = :id");
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updCustomGalleryItemName(const CustomGalleryItemData& item, const QString& name)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE custom_items SET name = :name WHERE id = :id");
    query.bindValue(":id", item.getId());
    query.bindValue(":name", name);
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updCustomGalleryItemNames(QList<CustomGalleryItemName>& names)
{
    bool res = true;
    QSqlQuery query;

    QSqlDatabase::database().transaction();
    foreach(const CustomGalleryItemName& item, names) {
        if(res) {
            query.prepare("UPDATE custom_items SET name = :name WHERE id = :id");
            query.bindValue(":id", item.item->getId());
            query.bindValue(":name", item.name);
            res = query.exec();
            if(res) {
            } else {
                break;
            }
            foreach(CustomGalleryItemData* i, item.item->getChildren()) {
                query.prepare("UPDATE custom_items SET name = :name WHERE id = :id");
                query.bindValue(":id", i->getId());
                query.bindValue(":name", item.name);
                res = query.exec();
                if(res) {
                } else {
                    break;
                }
            }
        } else {
            break;
        }
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    if(res) {
        int r = QSqlDatabase::database().commit();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    } else {
        int r = QSqlDatabase::database().rollback();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    }
    return res;
}

bool DbHandler::updCustomGalleryItemUnite(const CustomGalleryItemData& item, int customId)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE custom_items SET custom_item_id = :custom_item_id WHERE id = :id");
    query.bindValue(":id", item.getId());
    query.bindValue(":custom_item_id", customId);
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updCustomGalleryItemSplit(const CustomGalleryItemData& item)
{
    bool res = true;
    QSqlQuery query;

    QSqlDatabase::database().transaction();
    foreach(CustomGalleryItemData* i, item.getChildren()) {
        if(res) {
            query.prepare("UPDATE custom_items SET custom_item_id = NULL WHERE id = :id");
            query.bindValue(":id", i->getId());
            res = query.exec();
        } else {
            break;
        }
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    if(res) {
        int r = QSqlDatabase::database().commit();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    } else {
        int r = QSqlDatabase::database().rollback();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    }
    return res;
}

bool DbHandler::updCustomGalleryItemAngle(const CustomGalleryItemData& item, int angle)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE custom_items SET angle = :angle WHERE id = :id");
    query.bindValue(":id", item.getId());
    query.bindValue(":angle", angle);
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updCustomGallery(const CustomGalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE custom_galleries SET name = :name WHERE id = :id");
    query.bindValue(":name", value.getName());
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updGalleryItemReference(const GalleryItemData& item, const GalleryItemData& reference, int angle)
{
    bool res = true;
    QSqlQuery query;
    QSqlDatabase::database().transaction();
    query.prepare("DELETE FROM item_to_item WHERE id=:id");
    query.bindValue(":id", item.getId());
    res = query.exec();
    if(res) {
        query.prepare("INSERT INTO item_to_item (id, item_id, angle) VALUES (:id, :item_id, :angle)");
        query.bindValue(":id", item.getId());
        query.bindValue(":item_id", reference.getId());
        query.bindValue(":angle", angle);
        res = query.exec();
    }
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    if(res) {
        int r = QSqlDatabase::database().commit();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    } else {
        int r = QSqlDatabase::database().rollback();
        checkResAndWriteError(r, QSqlDatabase::database().lastError().text());
    }
    return res;
}

bool DbHandler::updGalleryItemStatus(const GalleryItemData& item)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE items SET status_id = :status_id WHERE id = :id");
    query.bindValue(":id", item.getId());
    query.bindValue(":status_id", item.getStatus());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updGallerySource(const GalleryData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE galleries SET source = :source WHERE id = :id");
    query.bindValue(":source", value.getSource());
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updKey(const KeyData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE keys SET name = :name WHERE id = :id");
    query.bindValue(":name", value.getName());
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updUrl(const UrlData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE urls SET name = :name WHERE id = :id");
    query.bindValue(":name", value.getName());
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

bool DbHandler::updAlias(const AliasData& value)
{
    bool res = true;
    QSqlQuery query;
    query.prepare("UPDATE aliases SET alias = :alias WHERE id = :id");
    query.bindValue(":alias", value.getAlias());
    query.bindValue(":id", value.getId());
    res = query.exec();
    checkResAndWriteError(res, query.lastError().text(), query.lastQuery());
    return res;
}

void DbHandler::checkResAndWriteError(bool res, const QString& error, const QString& text)
{
    if(!res) {
        qDebug() << error;
        qDebug() << text;
    }
}
