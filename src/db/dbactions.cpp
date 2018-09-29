/***
Buho  Copyright (C) 2018  Camilo Higuita
This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
This is free software, and you are welcome to redistribute it
under certain conditions; type `show c' for details.

 This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/

#include "dbactions.h"
#include <QJsonDocument>
#include <QVariantMap>
#include <QUuid>
#include <QDateTime>
#include "linker.h"

DBActions::DBActions(QObject *parent) : DB(parent)
{
    qDebug() << "Getting collectionDB info from: " << OWL::CollectionDBPath;

    qDebug()<< "Starting DBActions";
    this->tag =  Tagging::getInstance(OWL::App, OWL::version, "org.kde.buho", OWL::comment);

}

DBActions::~DBActions() {}

OWL::DB_LIST DBActions::getDBData(const QString &queryTxt)
{
    OWL::DB_LIST mapList;

    auto query = this->getQuery(queryTxt);

    if(query.exec())
    {
        while(query.next())
        {
            OWL::DB data;
            for(auto key : OWL::KEYMAP.keys())
                if(query.record().indexOf(OWL::KEYMAP[key])>-1)
                    data.insert(key, query.value(OWL::KEYMAP[key]).toString());

            mapList<< data;
        }

    }else qDebug()<< query.lastError()<< query.lastQuery();

    return mapList;
}

QVariantList DBActions::get(const QString &queryTxt)
{
    QVariantList mapList;

    auto query = this->getQuery(queryTxt);

    if(query.exec())
    {
        while(query.next())
        {
            QVariantMap data;
            for(auto key : OWL::KEYMAP.keys())
                if(query.record().indexOf(OWL::KEYMAP[key])>-1)
                    data[OWL::KEYMAP[key]] = query.value(OWL::KEYMAP[key]).toString();
            mapList<< data;

        }

    }else qDebug()<< query.lastError()<< query.lastQuery();

    return mapList;
}

bool DBActions::insertNote(const QVariantMap &note)
{
    qDebug()<<"TAGS"<< note[OWL::KEYMAP[OWL::KEY::TAG]].toStringList();

    auto title = note[OWL::KEYMAP[OWL::KEY::TITLE]].toString();
    auto body = note[OWL::KEYMAP[OWL::KEY::BODY]].toString();
    auto color = note[OWL::KEYMAP[OWL::KEY::COLOR]].toString();
    auto pin = note[OWL::KEYMAP[OWL::KEY::PIN]].toInt();
    auto fav = note[OWL::KEYMAP[OWL::KEY::FAV]].toInt();
    auto tags = note[OWL::KEYMAP[OWL::KEY::TAG]].toStringList();

    auto id = QUuid::createUuid().toString();

    QVariantMap note_map =
    {
        {OWL::KEYMAP[OWL::KEY::ID], id},
        {OWL::KEYMAP[OWL::KEY::TITLE], title},
        {OWL::KEYMAP[OWL::KEY::BODY], body},
        {OWL::KEYMAP[OWL::KEY::COLOR], color},
        {OWL::KEYMAP[OWL::KEY::PIN], pin},
        {OWL::KEYMAP[OWL::KEY::FAV], fav},
        {OWL::KEYMAP[OWL::KEY::UPDATED], QDateTime::currentDateTime().toString()},
        {OWL::KEYMAP[OWL::KEY::ADD_DATE], QDateTime::currentDateTime().toString()}
    };

    if(this->insert(OWL::TABLEMAP[OWL::TABLE::NOTES], note_map))
    {
        for(auto tg : tags)
            this->tag->tagAbstract(tg, OWL::TABLEMAP[OWL::TABLE::NOTES], id, color);

        this->noteInserted(note_map);
        return true;
    }

    return false;
}

bool DBActions::updateNote(const QVariantMap &note)
{
    auto id = note[OWL::KEYMAP[OWL::KEY::ID]].toString();
    auto title = note[OWL::KEYMAP[OWL::KEY::TITLE]].toString();
    auto body = note[OWL::KEYMAP[OWL::KEY::BODY]].toString();
    auto color = note[OWL::KEYMAP[OWL::KEY::COLOR]].toString();
    auto pin = note[OWL::KEYMAP[OWL::KEY::PIN]].toInt();
    auto fav = note[OWL::KEYMAP[OWL::KEY::FAV]].toInt();
    auto tags = note[OWL::KEYMAP[OWL::KEY::TAG]].toStringList();
    auto updated = note[OWL::KEYMAP[OWL::KEY::UPDATED]].toString();

    QVariantMap note_map =
    {
        {OWL::KEYMAP[OWL::KEY::TITLE], title},
        {OWL::KEYMAP[OWL::KEY::BODY], body},
        {OWL::KEYMAP[OWL::KEY::COLOR], color},
        {OWL::KEYMAP[OWL::KEY::PIN], pin},
        {OWL::KEYMAP[OWL::KEY::FAV], fav},
        {OWL::KEYMAP[OWL::KEY::UPDATED], updated}
    };

    for(auto tg : tags)
        this->tag->tagAbstract(tg, OWL::TABLEMAP[OWL::TABLE::NOTES], id, color);

    return this->update(OWL::TABLEMAP[OWL::TABLE::NOTES], note_map, {{OWL::KEYMAP[OWL::KEY::ID], id}} );
}

bool DBActions::removeNote(const QVariantMap &note)
{
    qDebug()<<note;
    return this->remove(OWL::TABLEMAP[OWL::TABLE::NOTES], note);
}

QVariantList DBActions::getNotes()
{
    return this->get("select * from notes ORDER BY updated ASC");
}

QVariantList DBActions::getNoteTags(const QString &id)
{
    return this->tag->getAbstractTags(OWL::TABLEMAP[OWL::TABLE::NOTES], id);
}

bool DBActions::insertLink(const QVariantMap &link)
{
    auto url = link[OWL::KEYMAP[OWL::KEY::LINK]].toString();
    auto color = link[OWL::KEYMAP[OWL::KEY::COLOR]].toString();
    auto pin = link[OWL::KEYMAP[OWL::KEY::PIN]].toInt();
    auto fav = link[OWL::KEYMAP[OWL::KEY::FAV]].toInt();
    auto tags = link[OWL::KEYMAP[OWL::KEY::TAG]].toStringList();
    auto preview = link[OWL::KEYMAP[OWL::KEY::PREVIEW]].toString();
    auto title = link[OWL::KEYMAP[OWL::KEY::TITLE]].toString();

    auto image_path = OWL::saveImage(Linker::getUrl(preview), OWL::LinksPath+QUuid::createUuid().toString());

    QVariantMap link_map =
    {
        {OWL::KEYMAP[OWL::KEY::LINK], url},
        {OWL::KEYMAP[OWL::KEY::TITLE], title},
        {OWL::KEYMAP[OWL::KEY::PIN], pin},
        {OWL::KEYMAP[OWL::KEY::FAV], fav},
        {OWL::KEYMAP[OWL::KEY::PREVIEW], image_path},
        {OWL::KEYMAP[OWL::KEY::COLOR], color},
        {OWL::KEYMAP[OWL::KEY::ADD_DATE], QDateTime::currentDateTime().toString()},
        {OWL::KEYMAP[OWL::KEY::UPDATED], QDateTime::currentDateTime().toString()}

    };

    if(this->insert(OWL::TABLEMAP[OWL::TABLE::LINKS], link_map))
    {
        for(auto tg : tags)
            this->tag->tagAbstract(tg, OWL::TABLEMAP[OWL::TABLE::LINKS], url, color);

        this->linkInserted(link_map);
        return true;
    }

    return false;
}

bool DBActions::updateLink(const QVariantMap &link)
{
    auto url = link[OWL::KEYMAP[OWL::KEY::LINK]].toString();
    auto color = link[OWL::KEYMAP[OWL::KEY::COLOR]].toString();
    auto pin = link[OWL::KEYMAP[OWL::KEY::PIN]].toInt();
    auto fav = link[OWL::KEYMAP[OWL::KEY::FAV]].toInt();
    auto tags = link[OWL::KEYMAP[OWL::KEY::TAG]].toStringList();
    auto updated = link[OWL::KEYMAP[OWL::KEY::UPDATED]].toString();

    QVariantMap link_map =
    {
        {OWL::KEYMAP[OWL::KEY::COLOR], color},
        {OWL::KEYMAP[OWL::KEY::PIN], pin},
        {OWL::KEYMAP[OWL::KEY::FAV], fav},
        {OWL::KEYMAP[OWL::KEY::UPDATED], updated},
    };

    for(auto tg : tags)
        this->tag->tagAbstract(tg, OWL::TABLEMAP[OWL::TABLE::LINKS], url, color);

    return this->update(OWL::TABLEMAP[OWL::TABLE::LINKS], link_map, {{OWL::KEYMAP[OWL::KEY::LINK], url}} );

}

bool DBActions::removeLink(const QVariantMap &link)
{
    return this->remove(OWL::TABLEMAP[OWL::TABLE::LINKS], link);
}

QVariantList DBActions::getLinks()
{
    return this->get("select * from links ORDER BY updated ASC");
}

QVariantList DBActions::getLinkTags(const QString &link)
{
    return this->tag->getAbstractTags(OWL::TABLEMAP[OWL::TABLE::LINKS], link);
}

bool DBActions::execQuery(const QString &queryTxt)
{
    auto query = this->getQuery(queryTxt);
    return query.exec();
}

void DBActions::removeAbtractTags(const QString &key, const QString &lot)
{

}

