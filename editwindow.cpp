#include "editwindow.h"
#include <QImage>
#include <QPainter>
#include "mainwindow.h"

EditWindow::EditWindow(BYTE Dbcs, UINT fontHeight, UINT fontWidth, BYTE *editFont, MainWindow* mainWindow, QWidget* parent)
    : QFrame(parent)
{
    this->Dbcs = Dbcs;
    this->editFont = editFont;
    this->fontHeight = fontHeight;
    this->fontWidth = fontWidth;
    this->mainWindow = mainWindow;


    //qApp->installEventFilter(this);
    setMouseTracking(true);

    create_image();
    setFrameStyle(QFrame::Panel | QFrame::Sunken);

    connect(mainWindow, SIGNAL(code_changed(WORD,BYTE*)), this, SLOT(code_change(WORD,BYTE*)));
    connect(mainWindow, SIGNAL(size_change(int,int,int)), this, SLOT(size_change(int,int,int)));
    update();
}

void EditWindow::code_change(WORD code, BYTE *buffer)
{
    this->code = code;
    create_image();
    editFont = buffer;
    for(int y = 0; y < fontHeight; y++)
    {
        for(int x=0; x < fontWidth; x++)
        {
            edit_draw_dot( x, y, edit_test_dot(x, y));
        }
    }
    update();
}

void EditWindow::size_change(int Dbcs, int fw, int fh)
{
    if(this->Dbcs == Dbcs)
    {
        fontHeight = fh;
        fontWidth = fw;
        create_image();
        code_change(code, editFont);
    }
}

void EditWindow::create_image()
{
    int n, m;
    n = 0;
//    if (n < FontHeight[0]) n = FontHeight[0];
//    if (n < FontWidth[0]) n = FontWidth[0];
//    if (n < FontHeight[1]) n = FontHeight[1];
//    if (n < FontWidth[1]) n = FontWidth[1];
    if (n < fontHeight) n = fontHeight;
    if (n < fontWidth) n = fontWidth;

    m = 9;
    if (n * 9 > EDIT_WINDOW_SQ) m = 7;
    if (n * 7 > EDIT_WINDOW_SQ) m = 5;
    if (n * 5 > EDIT_WINDOW_SQ) m = 3;
    EditDotSize = m;
    editBm = new QImage(fontWidth*EditDotSize, fontHeight*EditDotSize, QImage::Format_Mono);
    editBm->fill(0xFFFFFF);

}

/*private*/ void EditWindow::paintEvent(QPaintEvent *evt) /* 再描画 (Draw again)*/
{
    UINT x, y, n, m;
    QFrame::paintEvent(evt);

    QPainter* paint = new QPainter(this);

    n = 0;
//    if (n < FontHeight[0]) n = FontHeight[0];
//    if (n < FontWidth[0]) n = FontWidth[0];
//    if (n < FontHeight[1]) n = FontHeight[1];
//    if (n < FontWidth[1]) n = FontWidth[1];
    if (n < fontHeight) n = fontHeight;
    if (n < fontWidth) n = fontWidth;

    m = 9;
    if (n * 9 > EDIT_WINDOW_SQ) m = 7;
    if (n * 7 > EDIT_WINDOW_SQ) m = 5;
    if (n * 5 > EDIT_WINDOW_SQ) m = 3;
    EditDotSize = m;

    //hdc = BeginPaint(hwnd, &pnt);
    for (y = 0; y < fontHeight; y++) {
        for (x = 0; x < fontWidth; x++) {
            edit_draw_dot( x, y, edit_test_dot(x, y));
        }
    }
#if 1 // temp disable
    if(!mPoint.isNull() && ! lButton)
    {
        edit_draw_dot( dx, dy, edit_test_dot(dx, dy) | 4);
        edit_draw_dot(  mPoint.x(),  mPoint.y(), edit_test_dot( mPoint.x(),  mPoint.y()) | 6);
        mPoint = QPoint();
    }
//    else
//    {
//        WORD sr = edit_test_dot(x, y) ? 1 : 0;
//        mainWindow->edit_set_dot(x, y, sr);
//        edit_draw_dot(x, y, sr | 6);
//    }
#endif
    x = EditDotSize * fontWidth;
    y = EditDotSize * fontHeight;
    //StretchDIBits(hdc, 0, 0, x, y, 0, 0, x, y, EditBm[0], &EditBi, DIB_RGB_COLORS, SRCCOPY);
    paint->drawImage(QPointF(0,0), *editBm);
#if 0
    hbrs = GetSysColorBrush(COLOR_3DFACE);
    hpbrs = SelectObject(hdc, hbrs);
    hppen = SelectObject(hdc, GetStockObject(NULL_PEN));
    Rectangle(hdc, x, 0, EDIT_WINDOW_SQ + 1, EDIT_WINDOW_SQ + 1);
    Rectangle(hdc, 0, y, EDIT_WINDOW_SQ + 1, EDIT_WINDOW_SQ + 1);
    SelectObject(hdc, hpbrs);
    SelectObject(hdc, hppen);
    DeleteObject(hbrs);
    EndPaint(hwnd, &pnt);
#else
    paint->setPen(Qt::NoPen);
    paint->setBrush(QBrush(Qt::lightGray));
    paint->drawRect(QRectF(x, 0, EDIT_WINDOW_SQ + 1, EDIT_WINDOW_SQ + 1));
    paint->drawRect(QRectF(0, y, EDIT_WINDOW_SQ + 1, EDIT_WINDOW_SQ + 1));
#endif
}

/* 編集ウィンドウにドットを描画 */
void EditWindow::edit_draw_dot (
    UINT x,		/* 水平位置(0:左端) (Horizontal position (0: left end))*/
    UINT y,		/* 垂直位置(0:上端) (Vertical position (0: upper end))*/
    UINT ty		/* 形状 b0:ドット値, b1:カーソル, b2:画面描画 (Shape b0: Dot value, b1: Cursor, b2: Screen drawing) */
)
{
    WORD c;
    UINT xx, yy;
    QPainter qPainter(editBm);
    qPainter.setPen(Qt::black);

    if (x >= fontWidth || y >= fontHeight) return;

    c = (ty & 1) ? 0 : 1;
    x *= EditDotSize;
    y *= EditDotSize;
//    for (yy = y; yy < y + EditDotSize; yy++) {
//        //for (xx = x; xx < x + EditDotSize; EditBm[yy][xx++] = c) ;
//        for (xx = x; xx < x + EditDotSize; xx++)
//        {
//            editBm->setPixel(x, y, !c);
//        }
//    }
    qPainter.setBrush(QBrush(c?Qt::black:Qt::white));
    qPainter.setPen(c?Qt::black:Qt::white);
    qPainter.drawRect(x,y,EditDotSize,EditDotSize);

    switch (EditDotSize) {
    case 3:
        if (ty & 2) {
//			EditBm[y + 1][x + 0] ^= 0x7FFF;
            editBm->setPixel(x+0, y+1, editBm->pixel(x+0, y+1)^ 0x7FFF);
//			EditBm[y + 1][x + 1] ^= 0x7FFF;
            editBm->setPixel(x+1, y+1, editBm->pixel(x+1, y+1)^ 0x7FFF);
//			EditBm[y + 1][x + 2] ^= 0x7FFF;
            editBm->setPixel(x+2, y+1, editBm->pixel(x+2, y+1)^ 0x7FFF);
//			EditBm[y + 0][x + 1] ^= 0x7FFF;
            editBm->setPixel(x+1, y+0, editBm->pixel(x+1, y+0)^ 0x7FFF);
//			EditBm[y + 2][x + 1] ^= 0x7FFF;
            editBm->setPixel(x+1, y+2, editBm->pixel(x+1, y+2)^ 0x7FFF);
        }
        if (ty & 4) {
//			StretchDIBits(hdc, x, y, 3, 3, x, EDIT_WINDOW_SQ - y - EditDotSize, 3, 3, EditBm[0], &EditBi, DIB_RGB_COLORS, SRCCOPY);
            qPainter.setBrush(QBrush(Qt::black));
            qPainter.setPen(Qt::black);
            qPainter.drawRect(x,y,3,3);
        }
        break;

    case 5:
//		EditBm[y + 2][x + 2] ^= 0x7FFF;
        editBm->setPixel(x+2, y+2, editBm->pixel(x+2, y+2)^0x7FFF);
        if (ty & 2) {
//			EditBm[y + 2][x + 1] ^= 0x7FFF;
            editBm->setPixel(x+1, y+2, editBm->pixel(x+1, y+2));
//			EditBm[y + 2][x + 3] ^= 0x7FFF;
            editBm->setPixel(x+3, y+2, editBm->pixel(x+3, y+2));
//			EditBm[y + 1][x + 2] ^= 0x7FFF;
            editBm->setPixel(x+2, y+1, editBm->pixel(x+2, y+1));
//			EditBm[y + 3][x + 2] ^= 0x7FFF;
            editBm->setPixel(x+2, y+3, editBm->pixel(x+2, y+3));
        }
        if (ty & 4) {
//			StretchDIBits(hdc, x, y, 5, 5, x, EDIT_WINDOW_SQ - y - EditDotSize, 5, 5, EditBm[0], &EditBi, DIB_RGB_COLORS, SRCCOPY);
            qPainter.setBrush(QBrush(Qt::black));
            qPainter.setPen(Qt::black);
            qPainter.fillRect(x,y,5,5, Qt::black);
        }
        break;

    case 7:
//		EditBm[y + 3][x + 3] ^= 0x7FFF;
        editBm->setPixel(x+3, y+3, editBm->pixel(x+3, y+3)^0x7FFF);
        if (ty & 2) {
//			EditBm[y + 3][x + 1] ^= 0x7FFF;
            editBm->setPixel(x+1, y+3, editBm->pixel(x+1, y+3)^ 0x7FFF);
//			EditBm[y + 3][x + 2] ^= 0x7FFF;
            editBm->setPixel(x+2, y+3, editBm->pixel(x+2, y+3)^ 0x7FFF);
//			EditBm[y + 3][x + 4] ^= 0x7FFF;
            editBm->setPixel(x+4, y+3, editBm->pixel(x+4, y+3)^ 0x7FFF);
//			EditBm[y + 3][x + 5] ^= 0x7FFF;
            editBm->setPixel(x+5, y+3, editBm->pixel(x+5, y+3)^ 0x7FFF);
//			EditBm[y + 1][x + 3] ^= 0x7FFF;
            editBm->setPixel(x+3, y+1, editBm->pixel(x+3, y+1)^ 0x7FFF);
//			EditBm[y + 2][x + 3] ^= 0x7FFF;
            editBm->setPixel(x+3, y+2, editBm->pixel(x+3, y+2)^ 0x7FFF);
//			EditBm[y + 4][x + 3] ^= 0x7FFF;
            editBm->setPixel(x+3, y+4, editBm->pixel(x+3, y+4)^ 0x7FFF);
//			EditBm[y + 5][x + 3] ^= 0x7FFF;
            editBm->setPixel(x+3, y+5, editBm->pixel(x+3, y+5)^ 0x7FFF);
        }
        if (ty & 4) {
//			StretchDIBits(hdc, x, y, 7, 7, x, EDIT_WINDOW_SQ - y - EditDotSize, 7, 7, EditBm[0], &EditBi, DIB_RGB_COLORS, SRCCOPY);
            qPainter.setBrush(QBrush(Qt::black));
            qPainter.setPen(Qt::black);
            qPainter.drawRect(x,y,7,7);
        }
        break;

    case 9:
//		EditBm[y + 4][x + 4] ^= 0x7FFF;
//        editBm->setPixel(x+4, y+4, editBm->pixel(x+4, y+4)^0x7FFF);
        if (ty & 2) {
////			EditBm[y + 4][x + 1] ^= 0x7FFF;
//            editBm->setPixel(x+1, y+4, editBm->pixel(x+1, y+4)^ 0x7FFF);
////			EditBm[y + 4][x + 2] ^= 0x7FFF;
//            editBm->setPixel(x+2, y+4, editBm->pixel(x+2, y+4)^ 0x7FFF);
////			EditBm[y + 4][x + 3] ^= 0x7FFF;
//            editBm->setPixel(x+3, y+4, editBm->pixel(x+3, y+4)^ 0x7FFF);
////			EditBm[y + 4][x + 5] ^= 0x7FFF;
//            editBm->setPixel(x+5, y+4, editBm->pixel(x+5, y+4)^ 0x7FFF);
////			EditBm[y + 4][x + 6] ^= 0x7FFF;
//            editBm->setPixel(x+6, y+4, editBm->pixel(x+1, y+4)^ 0x7FFF);
////			EditBm[y + 4][x + 7] ^= 0x7FFF;
//            editBm->setPixel(x+7, y+4, editBm->pixel(x+1, y+4)^ 0x7FFF);
////			EditBm[y + 1][x + 4] ^= 0x7FFF;
//            editBm->setPixel(x+4, y+1, editBm->pixel(x+4, y+1)^ 0x7FFF);
////			EditBm[y + 2][x + 4] ^= 0x7FFF;
//            editBm->setPixel(x+4, y+2, editBm->pixel(x+4, y+2)^ 0x7FFF);
////			EditBm[y + 3][x + 4] ^= 0x7FFF;
//            editBm->setPixel(x+4, y+3, editBm->pixel(x+4, y+3)^ 0x7FFF);
////			EditBm[y + 5][x + 4] ^= 0x7FFF;
//            editBm->setPixel(x+4, y+5, editBm->pixel(x+4, y+5)^ 0x7FFF);
////			EditBm[y + 6][x + 4] ^= 0x7FFF;
//            editBm->setPixel(x+4, y+6, editBm->pixel(x+4, y+6)^ 0x7FFF);
////			EditBm[y + 7][x + 4] ^= 0x7FFF;
//            editBm->setPixel(x+4, y+7, editBm->pixel(x+4, y+7)^ 0x7FFF);
            qPainter.drawLine(x+4, y+1, x+4, y+7);
            qPainter.drawLine(x+1, y+4, x+7, y+4);
        }
        if (ty & 4) {
            //StretchDIBits(hdc, x, y, 9, 9, x, EDIT_WINDOW_SQ - y - EditDotSize, 9, 9, EditBm[0], &EditBi, DIB_RGB_COLORS, SRCCOPY);
            qPainter.setBrush(QBrush(Qt::black));
            qPainter.setPen(Qt::black);
            qPainter.drawRect(x,y,9,9);
        }
        break;
    }
    qPainter.end();
}

/* 編集フォントのドット値を取得 (Get dot value of edit font)*/
UINT EditWindow::edit_test_dot (
    UINT x,		/* 水平位置(0:左端) (Horizontal position (0: left edge))*/
    UINT y		/* 垂直位置(0:上端) (Vertical position (0: top))*/
)
{
    //return editFont[y][x / 8] & (0x80 >> (x % 8)) ? 1 : 0;
    BYTE* p;
    p = editFont;
    BYTE c = p[y*MAX_FONT_WB +(x / 8)] & (0x80 >> (x % 8)) ? 1 : 0;
    if(c==0)
        return 1;
    else
        return 0;
}

/*private*/ void EditWindow::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x()/EditDotSize;
    int y = event->pos().y()/EditDotSize;
    if (x < fontWidth && y < fontHeight) {
        if (x != dx || y != dy) {
    //                     if (wpar & MK_LBUTTON) {
    //                        edit_set_dot(x, y, sr);
    //                        edit_set_changed(TRUE);
    //                    }
            lButton = (event->buttons()&Qt::LeftButton == Qt::LeftButton);
    //                    hdc = GetDC(hwnd);
//            QPainter* paint = new QPainter(this);
            edit_draw_dot(dx, dy, edit_test_dot(dx, dy) | 2);
            edit_draw_dot(x,  y, edit_test_dot( x,  y) | 2);
            dx = x; dy = y;
            mPoint = QPoint(x,y);
            update();
        }
    }
    if(event->pos().x() > fontWidth*EditDotSize || event->pos().y() > fontHeight*EditDotSize )
    {
        create_image();
        update();
    }
}

/*private*/ void EditWindow::mousePressEvent(QMouseEvent *event)
{
    WORD sr;
    int x = event->pos().x()/EditDotSize;
    int y = event->pos().y()/EditDotSize;
    if ((x < fontWidth && y < fontHeight) && event->button() == Qt::LeftButton ) {
        lButton = true;
                sr = edit_test_dot(x, y) ? 1 : 0;
                mainWindow->edit_set_dot(x, y, sr);
//                hdc = GetDC(hwnd);
                edit_draw_dot(x, y, sr | 6);

                mainWindow->edit_set_changed(true);
                dx = x; dy = y;
                mPoint = QPoint(x,y);
                repaint();}
}

/*private*/ void EditWindow::leaveEvent(QEvent *)
{
    create_image();
    update();
}
