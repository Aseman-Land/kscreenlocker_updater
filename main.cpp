#define CACHE_DIR QString(QDir::homePath() + "/.cache/kscreenlocker_updater")

#include <QCoreApplication>
#include <QDir>
#include <QFileSystemWatcher>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QPainter>
#include <QImageWriter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QCryptographicHash>

class RepeativeFileWatcher: public QFileSystemWatcher
{
public:
    RepeativeFileWatcher() : QFileSystemWatcher() {}
    virtual ~RepeativeFileWatcher() {}

    bool addFile(const QString &file) {
        if(!addPath(file))
            return false;
        connect(this, &RepeativeFileWatcher::fileChanged, this, [this, file](){
            addPath(file);
        }, Qt::QueuedConnection);
        return true;
    }
};

QImage applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent=0)
{
    if(src.isNull()) return QImage();   //No need to do anything else!
    if(!effect) return src;             //No need to do anything else!
    QGraphicsScene *scene = new QGraphicsScene;
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
    item->setPixmap(QPixmap::fromImage(src));
    item->setGraphicsEffect(effect);
    scene->addItem(item);
    QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene->render(&ptr, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );
    scene->deleteLater();
    return res;
}

void writeKLockerConfigs(const QString &image)
{
    const QString path = QDir::homePath() + "/.config/kscreenlockerrc";
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
        return;

    QByteArray data;
    bool started = false;
    while(!file.atEnd())
    {
        QByteArray line = file.readLine().trimmed();

        if(line == "[Greeter][Wallpaper][org.kde.image][General]")
            started = true;
        else
        if(line.count() && line[0] == '[')
            started = false;
        else
        if(started)
        {
            int idx = line.indexOf("=");
            if(idx >= 0)
            {
                QString key = line.left(idx);
                if(key == "Image")
                {
                    QString value = line.mid(idx+1);

                    QString cache;
                    bool quote = 0;
                    for(int i=0; i<value.length(); i++)
                    {
                        const QChar ch = value[i];
                        if( (ch == '"' || ch == '\'') && (i==0 || value[i-1]!='\\'))
                        {
                            quote = !quote;
                            continue;
                        }

                        cache += ch;
                    }

                    if(cache == image)
                    {
                        qDebug() << "There is no new changes";
                        return;
                    }

                    line = (key + "=" + image).toUtf8();
                    qDebug() << "New wallpaper detected";
                }
            }
        }

        data += line + "\n";
    }

    file.close();

    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "Can't change new lockscreen background.";
        return;
    }

    file.write(data);
    file.close();
    qDebug() << "Lockscreen background changed.";
}

QString cacheBluredImage(QString imgPath)
{
    QString preDest;
    if(imgPath.left(7) == "file://")
    {
        imgPath = imgPath.mid(7);
        preDest = "file://";
    }

    QString hash = QCryptographicHash::hash(imgPath.toUtf8(), QCryptographicHash::Md5).toHex();
    QString destPath = CACHE_DIR + "/" + hash + ".png";
    if( QFileInfo::exists(destPath) )
        return preDest + destPath;

    QStringList extraFiles = QDir(CACHE_DIR).entryList({"*.png"});
    for(const QString &f: extraFiles)
        QFile::remove(CACHE_DIR + "/" + f);

    QDir().mkpath(CACHE_DIR);
    QImage img(imgPath);

    QGraphicsBlurEffect effect;
    effect.setBlurRadius(30);
    effect.setBlurHints(QGraphicsBlurEffect::QualityHint);

    QImageWriter writer(destPath);
    writer.write( applyEffectToImage(img, &effect) );
    return preDest + destPath;
}

void checkWallpaperConfigsFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
        return;

    qDebug() << "Checking for new changes...";

    bool started = false;
    while(!file.atEnd())
    {
        QByteArray line = file.readLine().trimmed();
        if(line.isEmpty())
            continue;

        if(line[0] == '[' && line.contains("[Wallpaper][org.kde.image][General]"))
            started = true;
        else
        if(line[0] == '[')
            started = false;
        else
        if(started)
        {
            int idx = line.indexOf("=");
            if(idx >= 0)
            {
                QString key = line.left(idx);
                if(key == "Image")
                {
                    QString value = line.mid(idx+1);

                    QString cache;
                    bool quote = 0;
                    for(int i=0; i<value.length(); i++)
                    {
                        const QChar ch = value[i];
                        if( (ch == '"' || ch == '\'') && (i==0 || value[i-1]!='\\'))
                        {
                            quote = !quote;
                            continue;
                        }

                        cache += ch;
                    }

                    writeKLockerConfigs( cacheBluredImage(cache) );
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QThread thread;
    thread.start();

    const QString configPath = QDir::homePath() + "/.config/plasma-org.kde.plasma.desktop-appletsrc";
    checkWallpaperConfigsFile(configPath);

    RepeativeFileWatcher *wallpaperWatcher = new RepeativeFileWatcher();
    wallpaperWatcher->addFile(configPath);
    wallpaperWatcher->moveToThread(&thread);
    wallpaperWatcher->connect(wallpaperWatcher, &RepeativeFileWatcher::fileChanged, wallpaperWatcher, &checkWallpaperConfigsFile);

    int res = app.exec();

    thread.quit();
    thread.wait();
    delete wallpaperWatcher;
    return res;
}
