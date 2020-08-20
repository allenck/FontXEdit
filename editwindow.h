#ifndef EDITWINDOW_H
#define EDITWINDOW_H
#include <QFrame>
#include "defines.h"
#include <QPaintEvent>
#include <QImage>

class QImage;
class MainWindow;
class EditWindow : public QFrame
{
    Q_OBJECT
public:
    EditWindow(BYTE Dbcs, UINT fontHeight, UINT fontWidth, BYTE* editFont, MainWindow *mainWindow, QWidget* parent =0);
    UINT edit_test_dot (
        UINT x,		/* 水平位置(0:左端) (Horizontal position (0: left end))*/
        UINT y		/* 垂直位置(0:上端) (Vertical position (0: upper end))*/
    );

public slots:
    void code_change(WORD code, BYTE* );
    void size_change(int Dbcs, int fw, int fh);

private:
    BYTE* editFont;
    void paintEvent(QPaintEvent* evt);
    UINT fontHeight = 0;
    UINT fontWidth = 0;
    UINT EditDotSize = 0;
    BYTE Dbcs;
    WORD code;
    MainWindow* mainWindow;
    int dx =0, dy =0;
    QPoint mPoint;
    bool lButton = false;

    void edit_draw_dot (
        //HDC hdc,	/* 編集ウィンドウのデバイスコンテキスト */
        UINT x,		/* 水平位置(0:左端) */
        UINT y,		/* 垂直位置(0:上端) */
        UINT ty		/* 形状 b0:ドット値, b1:カーソル, b2:画面描画 */
    );
    QImage* editBm = nullptr;
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void create_image();
    void leaveEvent(QEvent*);

};

#endif // EDITWINDOW_H
