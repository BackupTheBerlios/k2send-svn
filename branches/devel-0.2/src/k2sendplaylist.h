

#include <klistview.h>


class QListViewItem;
class QDropEvent;
class K2sendPlayListItem;
class KConfig;
class k2sendWidget;

class K2sendPlayList : public KListView
{
    Q_OBJECT

public:
    K2sendPlayList(QWidget* parent = 0, const char* name = 0 );
    ~K2sendPlayList();

    virtual bool acceptDrag(QDropEvent*) const;
    bool isDoubleEntry(const QString& file);
    void addDir(const QString & path,K2sendPlayListItem * after = 0);
    void addFile(const QString & path,K2sendPlayListItem * after = 0);
    void add(const QString & path,K2sendPlayListItem * after = 0);
    void write(KConfig * config,k2sendWidget * w);
    void read(KConfig * config);
    void next();
    void setIndex();
    void nextIndex();
    QString & nextFile();

public slots:
    void insertDroppedEvent(QDropEvent *e, QListViewItem *parent, QListViewItem *after);
signals:
    void signalChangeStatusbar(const QString& text);

private:
    QPoint pressPos;
    bool dragging;
    K2sendPlayListItem * m_head;
    K2sendPlayListItem * m_last;

};
