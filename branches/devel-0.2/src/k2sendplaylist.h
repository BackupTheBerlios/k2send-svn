

#include <klistview.h>


class K2sendPlayList : public KListView
{
    Q_OBJECT

public:
    K2sendPlayList(QWidget* parent = 0, const char* name = 0 );
    ~K2sendPlayList();

    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );
//     void contentsMousePressEvent( QMouseEvent * );
//     void contentsMouseMoveEvent( QMouseEvent * );
//     void contentsMouseReleaseEvent( QMouseEvent * );

private:
    QPoint pressPos;
    bool dragging;
};
