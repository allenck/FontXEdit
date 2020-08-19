#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class PreviewDialog;
}

class PreviewWidget;
class PreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewDialog(QWidget *parent = 0);
    ~PreviewDialog();

private:
    Ui::PreviewDialog *ui;
    void resizeEvent(QResizeEvent *event);
    PreviewWidget* previewWidget;
    friend class PreviewWidget;
};

class PreviewWidget : public QWidget
{
    Q_OBJECT
    MainWindow* mainWindow;
public:
    PreviewWidget(PreviewDialog* previewDialog)
    {
        this->mainWindow = (MainWindow*)previewDialog->parent();
        this->previewDialog = previewDialog;
        QPixmap cursor_pixmap = QPixmap(":cursor1.cur");
        QCursor cursor_default = QCursor(cursor_pixmap, 0, 0);
        setCursor(cursor_default);
    }
private:
    void paintEvent(QPaintEvent* evt);
    void pv_put_font (
        //HDC hdc,	/* 描画先デバイスコンテキスト */
        QRect rect,	/* 描画先領域サイズ (Drawing area size)*/
        long x,		/* 水平位置(dot) (Horizontal position (dot))*/
        long y,		/* 垂直位置(dot) (Vertical position (dot))*/
        long fw,	/* 水平フォントサイズ(dot) (Horizontal font size (dot))*/
        long fh,	/* 垂直フォントサイズ(dot) (Vertical font size (dot))*/
        WORD code,	/* 文字コード (Character code)*/
        WORD col	/* 文字色(RGB16) (Text color (RGB16))*/
    );
    long x, y, fw, fh, cc;
    QRect szw;
    WORD code;
    static WORD col;
    QByteArray str;
    PreviewDialog* previewDialog;
    QImage* img = nullptr;
    QCursor* cursor;
    void mousePressEvent(QMouseEvent*);
    friend class PreviewDialog;
};
#endif // PREVIEWDIALOG_H
