#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_tabwidget.h"
#include "tabwidget.h"
#include "defines.h"
#include <QMap>

typedef unsigned char BYTE;
namespace Ui {
class MainWindow;
}

class TabWidget;
class QFile;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    UINT load_file (bool opn	/* 0:再読込, 1:名前を指定して読み込み */);
    UINT read_fontxfile (	/* 0:失敗, 1:半角, 2:全角 (0: Failure, 1: Halfwidth, 2: Fullwidth)*/
        QString fname );
    void edit_set_dot (
        UINT x,		/* 水平位置(0:左端) */
        UINT y,		/* 垂直位置(0:上端) */
        UINT val	/* 0:クリア, 1:セット */
    );
    UINT edit_test_dot (
        UINT x,		/* 水平位置(0:左端) */
        UINT y		/* 垂直位置(0:上端) */
    );
    UINT edit_test_dot();
    void file_set_changed (
        bool f	/* TRUE:変更あり, FALSE:変更なし */
    );
    void edit_set_changed (
        bool f	/* TRUE:変更あり, FALSE:変更なし */
    );
    bool save_file (int);
    void edit_rotate (int dir); /* 1:左, 2:右, 3:上, 4:下 */

public slots:
    void on_tabWidget_current_changed(int);
    void fh_changed(int,int);
    void fw_changed(int,int);

signals:
    void code_changed(WORD, BYTE*);
    void size_change(int Dbcs, int width, int height);

private:
    Ui::MainWindow *ui;
    TabWidget* tabWidget1;
    TabWidget* tabWidget2;
    QList<TabWidget*> tabs = QList<TabWidget*>();

    /* フォントファイル情報 */
    QString FontFile[2]/*[_MAX_PATH]*/;		/* 開いているファイル (Open file)*/
    bool FileChanged[2];				/* ファイル変更フラグ (File change flag)*/
    UINT FontWidth[2] = {9, 18}, FontHeight[2] = {18, 18};	/* フォントサイズ[ドット] (Font size [dot])*/
//    BYTE CodeStat[0x10000];				/* b0:コード存在フラグ (b0: Code existence flag)*/
//    BYTE FontImage[0x10000][MAX_FONT_SQ][MAX_FONT_WB];
    QMap<QChar, BYTE*> fontMap = QMap<QChar, BYTE*>();

    /* フォント編集情報 (Font editing information)*/
    BYTE Dbcs = 0;			/* 編集中のフォント 0:半角, 1:全角 (Editing font 0: Half-width, 1: Full-width)*/
    WORD CurrentCode[2] = {0,0};    /* 編集中のコード (Code being edited)*/
    bool EditChanged = false;	/* 変更フラグ *///BYTE EditFont[MAX_FONT_SQ][MAX_FONT_WB];
    BYTE* EditFont = nullptr; //[MAX_FONT_SQ][MAX_FONT_WB];  /* 編集バッファ (Edit buffer)*/
    BYTE* CopyFont = nullptr;//[MAX_FONT_SQ][MAX_FONT_WB];	/* コピーバッファ (Copy buffer)*/
    /* 文字列 */
    int Loc =1;	/* User locale 0:Japanese, 1:English */

//    QString Str[30][2];// = {
//        /*  0 */ {" - FONTXエディタ R0.05"," - FONTX Editor R0.05"},
//        /*  1 */ {"無題","No name"},
//        /*  2 */ {"全角, %u字","DBC, %1 chrs"},
//        /*  3 */ {"半角, 256字","SBC, 256 chrs"},
//        /*  4 */ {"全角フォントを保存","Save DBC Font"},
//        /*  5 */ {"半角フォントを保存","Save SBC Font"},
//        /*  6 */ {"FONTXファイル (*.fnt)\0*.fnt\0全てのファイル (*.*)\0*.*\0","FONTX file (*.fnt)\0*.fnt\0All file (*.*)\0*.*\0"},
//        /*  7 */ {"書き込みに失敗しました.","Failed to save to the file"},
//        /*  8 */ {"ファイルに保存","Save file"},
//        /*  9 */ {"ファイルを開く","Open file"},
//        /* 10 */ {"FONTXファイル (*.fnt)\0*.fnt\0全てのファイル (*.*)\0*.*\0","FONTX files (*.fnt)\0*.fnt\0All files (*.*)\0*.*\0"},
//        /* 11 */ {"ファイルの読み込みに失敗しました.","Failed to load the file"},
//        /* 12 */ {"読み込み","Load FONTX file"},
//        /* 13 */ {"ファイルからインポート","Import font from a file"},
//        /* 14 */ {"X11 BDF (*.bdf)\0*.bdf\0","X11 BDF (*.bdf)\0*.bdf\0"},
//        /* 15 */ {"%d字読み込みました.","%d chrs imported"},
//        /* 16 */ {"インポート","Import"},
//        /* 17 */ {"ファイルへ書き出し","Write font as..."},
//        /* 18 */ {"垂直並びLCD形式/生(半角)\0*.*\0垂直並びLCD形式/サイズヘッダ(半角)\0*.*\0指定文字限定(全角)\0*.*\0","Vertical LCD form (SBCS)\0*.*\0Vertical LCD form with size header (SBCS)\0*.*\0Specified code only (DBCS)\0*.*\0"},
//        /* 19 */ {"全角フォントはこの形式で出力できません.","DBC font cannot be written in this format"},
//        /* 20 */ {"%d字書き込みました.","%d chrs written"},
//        /* 21 */ {"エクスポート","Export"},
//        /* 22 */ {"ファイルの読み込みに失敗しました.","Failed to load file"},
//        /* 23 */ {"読み込み","Load file"},
//        /* 24 */ {"半角","H/W"},
//        /* 25 */ {"全角","F/W"},
//        /* 26 */ {"ファイルの書き出しに失敗しました.", "Failed to write file."},
//        /* 27 */ {"半角フォントはこの形式で出力できません.","SBC font cannot be written in this format"},
//        /* 28 */ {"出力文字を含むSJISテキストを指定","Specify SJIS text with output characters"},
//        /* 29 */ {"ファイルのインポートに失敗しました.","Failed to import font"}};
    void rfsh_fontinfo (void);
    void read_font (
        //HANDLE h,	/* ファイルハンドル */
        QFile* f,
        QChar code,	/* 読み込み先の文字コード (Character code of the read destination)*/
        UINT fw,	/* フォントの幅[dot] (Font width [dot])*/
        UINT fh		/* フォントの高さ[dot] (Font height [dot])*/ );
    void get_font (
        WORD code,	/* ロードする文字コード (Character code to load)*/
        BYTE* font	/* ロード先編集バッファ (Load destination edit buffer) */    );
    void change_code ( int dir		/* 1: the next code, -1: the previous code */);
    void change_tab (UINT f);
    void import_file();
    void export_file();
    long write_vlcdfile (
        QString fname,
        int szinfo );
    long read_bdffile (QFile* fp);
    int encoded_count;
    int bitmap_count;
    int expectedChars;
    QString header = QString("/*\n\n\tFONTX version of the Public Domain X11 misc-fixed typeface.\n\thttps://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html\n\n*/\n");
    QString fontName = "        ";

    long write_fontxfile (QString fname, BYTE flagmask);
    void write_bytes(QTextStream* strm, BYTE* buf, int len);
    void write_font (
        //QFile* h,	/* ファイルハンドル (File handle)*/
        QTextStream* strm,
        WORD code,	/* 文字コード (Character code)*/
        UINT fw,	/* フォント幅[dot] (Font width [dot])*/
        UINT fh		/* フォント高さ[dot] (Font height [dot])*/    );
    bool write_fnt_file(QString path);
    QString outFile;
    QString inFile;

    // working variables used by write_bytes();
    int ix = 0;     //used by write_bytes();
    int byte_count; //used by write_bytes();
    QByteArray* fontx2 = new QByteArray();
    //

    friend class TabWidget;
    friend class PreviewWidget;
};


#endif // MAINWINDOW_H
