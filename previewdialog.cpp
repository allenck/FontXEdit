#include "previewdialog.h"
#include "ui_previewdialog.h"
#include <QBoxLayout>
#include <QImage>
#include <QPainter>
#include <QDebug>

PreviewDialog::PreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewDialog)
{
    ui->setupUi(this);
    ui->idc_pvimg->setLayout(new QVBoxLayout());
    ui->idc_pvimg->layout()->addWidget(previewWidget =new PreviewWidget(this));
    connect(ui->idc_pvsrc, &QTextEdit::textChanged, [=]{
        update();
    });
}

PreviewDialog::~PreviewDialog()
{
    delete ui;
}

void PreviewDialog::resizeEvent(QResizeEvent *event)
{
    previewWidget->img = nullptr;
    previewWidget->update();
}
/*static*/ WORD PreviewWidget::col = 0;

void PreviewWidget::paintEvent(QPaintEvent *evt)
{
    int cix = 0;
    str = previewDialog->ui->idc_pvsrc->toPlainText().toLocal8Bit();
    cc = str.length();
    //GetClientRect(hwnd, &szw);
    szw = evt->rect();
    if(img == nullptr)
    {
        img = new QImage(szw.width(), szw.height(), QImage::Format_Mono);
        img->fill(0xFFFFFFFF);
    }
    //hdc = BeginPaint(hwnd, &pnt);

    for (y = 0; y < szw.bottom(); ) {
        code = 1;
        for (x = 0; x < szw.right() || code; )
        {
            if (!cc) code = 0;
            if (code)
            {
                code = (BYTE)str[cix++]; cc--;
                if (code == 13) {
                    if (cc && code == 10) {
                        cix++; cc--;
                    }
                    code = 0;
                } else {
                    if (!mainWindow->Loc && cc && IS_DBC1(code) && IS_DBC2(str[cix])) {
                        code = code << 8 | (BYTE)str[cix++]; cc--;
                    }
                }
            }
            fw = mainWindow->FontWidth[(code >= 0x100) ? 1 : 0];
            fh = mainWindow->FontHeight[(code >= 0x100) ? 1 : 0];
            if(code > 0)
                qDebug() << "pv_put_font " << szw << " x:" << x << " y:" << y << fw << fh  << QChar(code);
            pv_put_font(szw, x, y, fw, fh, code?code:32, col);
            x += fw;
        }
        y += fh;
        QPainter painter(this);
        painter.drawImage(0,0, *img);
    }

//    EndPaint(hwnd, &pnt);
//    GlobalFree(pstr);
}

/* プリビューウィンドウに1文字描画 (Draw one character in preview window)*/
void PreviewWidget::pv_put_font (
    //HDC hdc,	/* 描画先デバイスコンテキスト */
    QRect rect,	/* 描画先領域サイズ (Drawing area size)*/
    long x,		/* 水平位置(dot) (Horizontal position (dot))*/
    long y,		/* 垂直位置(dot) (Vertical position (dot))*/
    long fw,	/* 水平フォントサイズ(dot) (Horizontal font size (dot))*/
    long fh,	/* 垂直フォントサイズ(dot) (Vertical font size (dot))*/
    WORD code,	/* 文字コード (Character code)*/
    WORD col	/* 文字色(RGB16) (Text color (RGB16))*/
)
{
    //static WORD img[MAX_FONT_SQ][MAX_FONT_SQ];
    long ih, iw;
    BYTE d = 0;
    //BITMAPINFO bmi;
    QPainter paint(img);
    /* 描画先の右端下端でクリップ (Clip at the right bottom edge of the drawing destination)*/
    if (x >= rect.right() || y >= rect.bottom())
        return;
    if (x + fw > rect.right())
        fw = rect.right() - x;
    if (y + fh > rect.bottom())
        fh = rect.bottom() - y;

    /* フォントイメージの作成 (Creating a font image)*/
    unsigned char* FontImage = mainWindow->fontMap.value(code);
    if (code < 0x20 || /*!(CodeStat[code] & 1)*/ !mainWindow->fontMap.contains(code))
        code = 0x20;
    for (ih = 0; ih < fh; ih++)
    {
        for (iw = 0; iw < fw; iw++) {
            if ((iw & 7) == 0)
                d = FontImage[ih*MAX_FONT_WB + (iw/8)]; //FontImage[code][ih][iw / 8];
            //img[ih][iw] = (d & 0x80) ? col : ~col;
            //img->setPixel(iw+x, ih+y, (d & 0x80) ? qRgb(0,0,0) : qRgb(255,255,255));
            paint.setPen((d & 0x80) ? col : ~col);
            paint.drawPoint(iw+x, ih+y);
            if(code > 0x20)
             qDebug() << "setPixel" << iw+x << ih+y <<  ((d & 0x80) ? 0 : 1) << QChar(code);
            d <<= 1;
        }
    }
    //paint->drawImage(QPoint(0,0), *img);

    /* デバイスに転送 (Transfer to device)*/
//	bmi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
//	bmi.bmiHeader.biWidth = MAX_FONT_SQ;
//	bmi.bmiHeader.biHeight = 0 - MAX_FONT_SQ;
//	bmi.bmiHeader.biPlanes = 1;
//	bmi.bmiHeader.biBitCount = 16;
//	bmi.bmiHeader.biCompression = BI_RGB;
//	bmi.bmiHeader.biClrUsed = 0;
//	bmi.bmiHeader.biClrImportant = 0;
//	bmi.bmiHeader.biSizeImage = 0;
//	StretchDIBits(hdc, x, y, fw, fh, 0, 0, fw, fh, img[0], &bmi, DIB_RGB_COLORS, SRCCOPY);
}

void PreviewWidget::mousePressEvent(QMouseEvent *event)
{
    col = ~col;
    update();
}
