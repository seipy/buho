#include "linker.h"
#include <QEventLoop>
#include "owl.h"

Linker::Linker(QObject *parent) : QObject(parent)
{

}

/* extract needs to extract from a url the title, the body and a preview image*/
void Linker::extract(const QString &url)
{
    auto data = this->getUrl(url);

    auto titles = query(data, HtmlTag::TITLE);
    QStringList imgs ;

    for(auto img : query(data, HtmlTag::IMG, "src"))
    {
        if(imgs.contains(img) || img.isEmpty()) continue;


        if(url.at(url.length()-1) == "/")
        {
            if(img.startsWith("http"))
                imgs << img;
            else
                imgs << url+img;
        }else
        {
            if(img.startsWith("http"))
                imgs << img;
            else
                imgs << url+"/"+img;
        }
    }
    LINK link_data {{OWL::KEYMAP[OWL::KEY::TITLE], titles},
                    {OWL::KEYMAP[OWL::KEY::BODY], data},
                    {OWL::KEYMAP[OWL::KEY::IMAGE], imgs}};
    emit previewReady(link_data);
}

QByteArray Linker::getUrl(const QString &url)
{
    QUrl mURL(url);
    QNetworkAccessManager manager;
    QNetworkRequest request (mURL);

    QNetworkReply *reply =  manager.get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop,
            SLOT(quit()));

    loop.exec();

    if(reply->error())
    {
        qDebug() << reply->error();
        return QByteArray();
    }

    if(reply->bytesAvailable())
    {
        auto data = reply->readAll();
        reply->deleteLater();

        return data;
    }

    return QByteArray();
}

QStringList Linker::query(const QByteArray &array, const HtmlTag &tag, const QString &attribute)
{
    QStringList res;
    auto doc = QGumboDocument::parse(array);
    auto root = doc.rootNode();

    auto node = root.getElementsByTagName(tag);

    for(const auto &i : node)
    {
        if(attribute.isEmpty())
            res << i.innerText();
        else res << i.getAttribute(attribute);
    }

    return res;
}

