#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <previewdialog.h>
#include <QWindow>
#include <QStringList>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    EditFont = new BYTE[MAX_FONT_SQ*MAX_FONT_WB];
    memset(EditFont, 0, MAX_FONT_SQ*MAX_FONT_WB);
    ui->setupUi(this);
    ui->tab->setLayout(new QVBoxLayout());
    tabWidget1 = new TabWidget(0,this);
    tabWidget2 = new TabWidget(1,this);
    ui->tab->setLayout(new QVBoxLayout());
    ui->tab->layout()->addWidget(tabWidget1);
    tabs.append(tabWidget1);
    connect(tabWidget1, SIGNAL(fh_changed(int,int)), this, SLOT(fh_changed(int,int)));
    connect(tabWidget1, SIGNAL(fw_changed(int,int)), this, SLOT(fw_changed(int,int)));
    ui->tab_2->setLayout(new QVBoxLayout());
    ui->tab_2->layout()->addWidget(tabWidget2);
    tabs.append(tabWidget2);
    connect(tabWidget2, SIGNAL(fw_changed(int,int)), this, SLOT(fw_changed(int,int)));
    connect(tabWidget2, SIGNAL(fw_changed(int,int)), this, SLOT(fw_changed(int,int)));
    ui->idc_tab1->setCurrentIndex(0);
    tabs.at(Dbcs)->ui->idc_code->setToolTip("2/4 digit hexadecimal number, 1 character, select by wheel");
    tabs.at(0)->ui->idc_code->setInputMask("HH");
    tabs.at(1)->ui->idc_code->setInputMask("HHHH");

    connect(ui->idc_tab1, SIGNAL(currentChanged(int)), this, SLOT(on_tabWidget_current_changed(int)));
    connect(ui->idc_exit, &QPushButton::clicked, [=]{
        close();
        exit(0);
    });

    connect(ui->idc_imp, &QPushButton::clicked, [=]{
        /* インポートボタン (Import button)*/
        import_file();
        change_tab(0);
    });

    connect(ui->idc_exp, &QPushButton::clicked, [=]{
        /* エクスポートボタン (Export button)*/
        export_file();
    });;

    connect(ui->idc_pvc, &QPushButton::clicked, [=]{
        /* プリビューボタン (Preview button)*/
       PreviewDialog* dlg = new PreviewDialog(this);
       int rslt = dlg->exec();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString Str[30][2] = {
    /*  0 */ {" - FONTXエディタ R0.05"," - FONTX Editor R0.05"},
    /*  1 */ {"無題","No name"},
    /*  2 */ {"全角, %u字","DBC, %1 chrs"},
    /*  3 */ {"半角, 256字","SBC, 256 chrs"},
    /*  4 */ {"全角フォントを保存","Save DBC Font"},
    /*  5 */ {"半角フォントを保存","Save SBC Font"},
    /*  6 */ {"FONTXファイル (*.fnt)\0*.fnt\0全てのファイル (*.*)\0*.*\0","FONTX file (*.fnt)\0*.fnt\0All file (*.*)\0*.*\0"},
    /*  7 */ {"書き込みに失敗しました.","Failed to save to the file"},
    /*  8 */ {"ファイルに保存","Save file"},
    /*  9 */ {"ファイルを開く","Open file"},
    /* 10 */ {"FONTXファイル (*.fnt)\0*.fnt\0全てのファイル (*.*)\0*.*\0","FONTX files (*.fnt)\0*.fnt\0All files (*.*)\0*.*\0"},
    /* 11 */ {"ファイルの読み込みに失敗しました.","Failed to load the file"},
    /* 12 */ {"読み込み","Load FONTX file"},
    /* 13 */ {"ファイルからインポート","Import font from a file"},
    /* 14 */ {"X11 BDF (*.bdf)\0*.bdf\0","X11 BDF (*.bdf)\0*.bdf\0"},
    /* 15 */ {"%1字読み込みました.","%1 chrs imported"},
    /* 16 */ {"インポート","Import"},
    /* 17 */ {"ファイルへ書き出し","Write font as..."},
    /* 18 */ {"垂直並びLCD形式/生(半角)\0*.*\0垂直並びLCD形式/サイズヘッダ(半角)\0*.*\0指定文字限定(全角)\0*.*\0","Vertical LCD form (SBCS)\0*.*\0Vertical LCD form with size header (SBCS)\0*.*\0Specified code only (DBCS)\0*.*\0"},
    /* 19 */ {"全角フォントはこの形式で出力できません.","DBC font cannot be written in this format"},
    /* 20 */ {"%d字書き込みました.","%d chrs written"},
    /* 21 */ {"エクスポート","Export"},
    /* 22 */ {"ファイルの読み込みに失敗しました.","Failed to load file"},
    /* 23 */ {"読み込み","Load file"},
    /* 24 */ {"半角","H/W"},
    /* 25 */ {"全角","F/W"},
    /* 26 */ {"ファイルの書き出しに失敗しました.", "Failed to write file."},
    /* 27 */ {"半角フォントはこの形式で出力できません.","SBC font cannot be written in this format"},
    /* 28 */ {"出力文字を含むSJISテキストを指定","Specify SJIS text with output characters"},
    /* 29 */ {"ファイルのインポートに失敗しました.","Failed to import font"}};


void MainWindow::write_bytes(QTextStream* strm, BYTE* buf, int len)
{
   for(int i=0; i < len; i++)
   {
      BYTE b = buf[i];
      QString str = QString("0x%1").arg(b, 2, 16, QChar('0'));
      byte_count++;
      if(ix > 0)
          *strm << ", ";
      else
          *strm << "  ";
      *strm << str;
      if(++ix > 11)
      {
          *strm << ",\n";
          ix = 0;
      }
   }
   // keep a copy in QByteArray.
   fontx2->append((char*)buf, len);
}

/* 1文字のフォントデータをファイルに書き込み (Write 1 character font data to file)*/
void MainWindow::write_font (
    //QFile* h,	/* ファイルハンドル (File handle)*/
    QTextStream* strm,
    WORD code,	/* 文字コード (Character code)*/
    UINT fw,	/* フォント幅[dot] (Font width [dot])*/
    UINT fh		/* フォント高さ[dot] (Font height [dot])*/)
{
    char buf[MAX_FONT_WB * MAX_FONT_SQ], *d;
    BYTE* s;
    DWORD bw;
    UINT v, fwb;


    //s = FontImage[code][0];
    s = fontMap.value(QChar(code), new BYTE[0, MAX_FONT_WB * MAX_FONT_SQ]);
    d = buf;
    fwb = (fw + 7) / 8;
    for (v = 0; v < fh; v++) {
        //CopyMemory(d, s, fwb);
        memcpy(d, s, fwb);
        d += fwb; s += MAX_FONT_WB;
    }
    //WriteFile(h, buf, fwb * fh, &bw, 0);
    write_bytes(strm, (BYTE*)buf, fwb * fh);
    //h->write((const char *)buf, fwb * fh);
}
void MainWindow::write_font (
    //QFile* h,	/* ファイルハンドル (File handle)*/
    QFile* h,
    WORD code,	/* 文字コード (Character code)*/
    UINT fw,	/* フォント幅[dot] (Font width [dot])*/
    UINT fh		/* フォント高さ[dot] (Font height [dot])*/)
{
    char buf[MAX_FONT_WB * MAX_FONT_SQ], *d;
    BYTE* s;
    DWORD bw;
    UINT v, fwb;


    //s = FontImage[code][0];
    s = fontMap.value(QChar(code), new BYTE[0, MAX_FONT_WB * MAX_FONT_SQ]);
    d = buf;
    fwb = (fw + 7) / 8;
    for (v = 0; v < fh; v++) {
        //CopyMemory(d, s, fwb);
        memcpy(d, s, fwb);
        d += fwb; s += MAX_FONT_WB;
    }
    //WriteFile(h, buf, fwb * fh, &bw, 0);
    //write_bytes(strm, (BYTE*)buf, fwb * fh);
    h->write((const char *)buf, fwb * fh);
}

/* フォント情報の表示更新 */
void MainWindow::rfsh_fontinfo (void)
{
    QString str;
    UINT nchr, i;

    /* キャプション */
    if (!FontFile[Dbcs][0].isNull())
    {
     //GetFileTitle(FontFile[Dbcs], str, _MAX_PATH - 20);
        QFileInfo inf(FontFile[Dbcs]);
        str = inf.baseName();
    }
    else
    {
        //strcpy(str, Str[1][Loc]);
        str = Str[1][Loc];
    }
    str = str + Str[0][Loc];
    //SetWindowText(hMainDlg, str);
    setWindowTitle(str);

    /* 文字数 (word count)*/
    if (Dbcs) {
        for (nchr = 0, i = 0x100; i < 0x10000; i++) {
            //if (CodeStat[i] & 1) nchr++;
            if(fontMap.contains(i)) nchr++;
        }
        //sprintf(str, Str[2][Loc], nchr);
        tabWidget2->ui->idc_info->setText(QString(Str[2][Loc]).arg(nchr));
    } else {
        //sprintf(str, Str[3][Loc]);
        tabWidget1->ui->idc_info->setText(Str[3][Loc]);
    }
    //SetDlgItemText(hMainDlg, IDC_INFO, str);

    /* 幅と高さ (Width and height)*/
        //sprintf(str, "%u", FontWidth[Dbcs]);
        //SetDlgItemText(hMainDlg, IDC_FW, str);
        tabs.at(Dbcs)->ui->idc_fw->setValue(FontWidth[Dbcs]);

//        sprintf(str, "%u", FontHeight[Dbcs]);
//        SetDlgItemText(hMainDlg, IDC_FH, str);
        tabs.at(Dbcs)->ui->idc_fh->setValue(FontHeight[Dbcs]);

    /* 上書保存ボタン */
    //EnableWindow(GetDlgItem(hMainDlg, IDC_SAVE), FileChanged[Dbcs]);
        tabs.at(Dbcs)->ui->idc_save->setEnabled(true);
}



/* フォント編集フラグの更新 (Update font edit flag)*/
void MainWindow::edit_set_changed (
    bool f	/* TRUE:変更あり, FALSE:変更なし */
)
{
    if (f != EditChanged) {
        //EnableWindow(GetDlgItem(hMainDlg, IDC_APP), f);
        tabs.at(Dbcs)->ui->idc_app->setEnabled(f);
        //EnableWindow(GetDlgItem(hMainDlg, IDC_REV), f);
        tabs.at(Dbcs)->ui->idc_rev->setEnabled(f);
    }
    EditChanged = f;
}

/* ファイル編集フラグの更新 */
void MainWindow::file_set_changed (
    bool f	/* TRUE:変更あり, FALSE:変更なし */
)
{
    if (f != FileChanged[Dbcs]) {
        //EnableWindow(GetDlgItem(hMainDlg, IDC_SAVE), f);
        tabs.at(Dbcs)->ui->idc_save->setEnabled(f);
    }
    FileChanged[Dbcs] = f;
}

void MainWindow::get_font (WORD code,	/* ロードする文字コード (Character code to load)*/
    BYTE *font	/* ロード先編集バッファ (Load destination edit buffer) */
)
{
    char str[20];


    /* フォントイメージのロード (Load font image)*/
    //CopyMemory(font, FontImage[code], MAX_FONT_SQ * MAX_FONT_WB);
//    memcpy(font, FontImage[code], MAX_FONT_SQ * MAX_FONT_WB);
//    /* コード入力エディトボックスを更新 (Update chord input edit box)*/
//    //ZeroMemory(str, sizeof str);
//    memset(str, 0, sizeof(str));
//    if (IS_SBC(code) || (Loc && code >= 0x20)) {
//        str[16] = (BYTE)code;
//        if (code == '&') str[17] = (BYTE)code;
//    } else {
//        if (IS_DBC(code)) {
//            str[17] = (BYTE)code;
//            str[16] = (BYTE)(code >> 8);
//        }
//    }
    font = fontMap.value(QChar(code), new BYTE[0, MAX_FONT_SQ * MAX_FONT_WB]);
    //sprintf(str, "%s", str + 16);
    //SetDlgItemText(hMainDlg, IDC_CHAR, str);
    tabs.at(Dbcs)->ui->idc_char->setText(QChar(code));
    sprintf(str, Dbcs ? "%04X" : "%02X", code);
    //SetDlgItemText(hMainDlg, IDC_CODE, str);
    tabs.at(Dbcs)->ui->idc_code->setText(QString("%1").arg(code,2,16, QChar('0')));

//    edit_set_changed(FALSE);
//    InvalidateRect(hEdit, 0, TRUE);	/* 編集ウィンドウを更新 (Update edit window);*/
    update();
}

/* 編集文字コードの切り替え (Switching the editing character code)*/
void MainWindow::change_code ( int dir		/* 1: the next code, -1: the previous code */)
{
    WORD chr;
    QChar qChar(chr);

    chr = CurrentCode[Dbcs];
    if (dir) {
        if (Dbcs) {
            do {
                chr += dir;
                qChar = QChar(chr);
                if (chr < 0x100 && dir > 0) chr = 0x100;
                if (chr < 0x100 && dir < 0) chr = 0xFFFF;
            } while (!fontMap.contains(qChar));//while (!(CodeStat[chr] & 1));
        } else {
            chr = (chr + dir) & 255;
        }
    }
    CurrentCode[Dbcs] = chr;
    if(fontMap.contains(QChar(chr)))
        EditFont=fontMap.value(QChar(chr));
    else
        EditFont = new BYTE[0, MAX_FONT_SQ*MAX_FONT_WB];
    get_font(chr, EditFont);
    emit code_changed(chr, EditFont);
}

/* Switching tabs and updating font information */
void MainWindow::change_tab (
    UINT f	/* 0:不変, 1:半角 2:全角 (0: No change, 1: Half-width 2: Full-width)*/
)
{
    if (f) {
        Dbcs = (f == 2) ? 1 : 0;
        //SendMessage(hTab, TCM_SETCURSEL, (WPARAM)Dbcs, 0);
        CurrentCode[Dbcs] = 0xFFFF;
        change_code(1);
    } else {
        change_code(0);
    }
    //rfsh_fontinfo();
//	if (hPv) InvalidateRect(hPv, 0, TRUE);
}

/* 編集フォントのドット値を設定 */
void MainWindow::edit_set_dot (
    UINT x,		/* 水平位置(0:左端) */
    UINT y,		/* 垂直位置(0:上端) */
    UINT val	/* 0:クリア(0: Clear), 1:セット (1 set)*/
)
{
    int ix = y * MAX_FONT_WB + (x / 8);
    char c = EditFont[ix];
    if (val) {
        //EditFont[y][x / 8] |= 0x80 >> (x % 8);
        c = c | (0x80 >> (x % 8));
    } else {
        //EditFont[y][x / 8] &= ~(0x80 >> (x % 8));
        //c = c & (0x80 << ~((x % 8)));
        c  &= ~(0x80 >> (x % 8));
    }
    EditFont[ix] = c;

}

/* 編集フォントのドット値を取得 */
UINT MainWindow::edit_test_dot (
    UINT x,		/* 水平位置(0:左端) */
    UINT y		/* 垂直位置(0:上端) */
)
{
    //return EditFont[y][x / 8] & (0x80 >> (x % 8)) ? 1 : 0;
    int bix = y*MAX_FONT_WB + x/8;
    char c = EditFont[bix] & (0x80 >> (x % 8)) ? 1 : 0;
    return c;
}

/* 編集フォントのローテート */
void MainWindow::edit_rotate (int dir		/* 1:左, 2:右, 3:上, 4:下 (1: left, 2: right, 3: up, 4: down) */)
{
    BYTE f, m, buf[MAX_FONT_WB];
    UINT vc, hc, i;

#if 0
    switch (dir) {
    case 1:	/* left */
        i = (FontWidth[Dbcs] - 1) / 8;
        m = 0x80 >> ((FontWidth[Dbcs] - 1) % 8);
        for (vc = 0; vc < FontHeight[Dbcs]; vc++) {
            f = EditFont[vc][0] & 0x80;
            hc = 0;
            do {
                EditFont[vc][hc] <<= 1;
                if (hc + 1 < MAX_FONT_WB) {
                    if (EditFont[vc][hc + 1] & 0x80) EditFont[vc][hc] |= 1;
                }
            } while (++hc < MAX_FONT_WB);
            if (f) {
                EditFont[vc][i] |= m;
            } else {
                EditFont[vc][i] &= ~m;
            }
        }
        break;

    case 2:	/* right */
        i = (FontWidth[Dbcs] - 1) / 8;
        m = 0x80 >> ((FontWidth[Dbcs] - 1) % 8);
        for (vc = 0; vc < FontHeight[Dbcs]; vc++) {
            f = EditFont[vc][i] & m;
            hc = MAX_FONT_WB - 1;
            do {
                EditFont[vc][hc] >>= 1;
                if (hc) {
                    if (EditFont[vc][hc - 1] & 1) EditFont[vc][hc] |= 0x80;
                }
            } while (hc--);
            if (f) EditFont[vc][0] |= 0x80;
        }
        break;

    case 3:	/* up */
        memcpy(buf, EditFont[0], MAX_FONT_WB);
        memmove(EditFont[0], EditFont[1], (FontHeight[Dbcs] - 1) * MAX_FONT_WB);
        memcpy(EditFont[FontHeight[Dbcs] - 1], buf, MAX_FONT_WB);
        break;

    case 4:	/* down */
        memcpy(buf, EditFont[FontHeight[Dbcs] - 1], MAX_FONT_WB);
        memmove(EditFont[1], EditFont[0], (FontHeight[Dbcs] - 1) * MAX_FONT_WB);
        memcpy(EditFont[0], buf, MAX_FONT_WB);
        break;
    }

    edit_set_changed(true);
    InvalidateRect(hEdit, 0, TRUE);	/* 編集ウィンドウを更新 */
#endif
}

bool MainWindow::write_fnt_file(QString path)
{
    QFile f(path);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "failed to open output file: " << path;
    }
    f.write(fontx2->constData(), fontx2->size());
    f.close();
}

/* フォントファイルの作成 (Creating a font file)*/
long MainWindow::write_fontxfile (QString fname, BYTE flagmask)
{
    QFile* h;
    BYTE buf[20];
    WORD range[256][2], sr, er, i;
    DWORD bw, nr, r;
    UINT wf, hf;
    long nfnt;
    bool hFormat = false;
    QFileInfo inf(fname);
    QTextStream* strm = nullptr;

    if(inf.suffix() == "h" )
    {
        hFormat = true;
    }

    ix = 0;     //used by write_bytes();
    byte_count; //used by write_bytes();
    fontx2->clear();

    //h = CreateFile(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    //if (h == INVALID_HANDLE_VALUE) return -1;

    QIODevice::OpenMode mode;
    if(hFormat)
    {
        mode = QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text;
    }
    else
    {
        mode = QIODevice::WriteOnly | QIODevice::Truncate;

    }
    h = new QFile(fname);
    if(!h->open(mode))
    {
        qDebug() <<"unable to open output " << fname;
        QMessageBox::critical(this, tr("Error"), tr("Unable to open output fl %1").arg(fname), QMessageBox::Ok);
        return (-1);
    }

    if(hFormat)
    {
        strm = new QTextStream(h);

        strm->setCodec("UTF-8");
        strm->seek(0);
        *strm << header;
        *strm << "const unsigned char ";
        *strm << fontName;
        *strm  << "[] = {\n";
    }

    wf = FontWidth[Dbcs]; hf = FontHeight[Dbcs];

    /* FONTXヘッダの書き込み (Write FONTX header)*/
    strcpy((char *)buf, "FONTX2        ");
    memcpy((char *)buf+6, fontName.toLatin1(), fontName.length());
    buf[14] = wf; buf[15] = hf; buf[16] = Dbcs;
    //WriteFile(h, buf, 17, &bw, 0);
    if(hFormat)
        write_bytes( strm, buf, 17); // write font metadata
    else
        h->write((const char *)buf, 17);

    if (Dbcs) {	/* 全角フォント (Full-width font)*/
        /* コードレンジテーブルを作成 (Create code range table)*/
        for (nr = 0, r = /*0x100*/1; nr < 256; nr++) {
            int codeStat_r = fontMap.contains(QChar(r))? 1 : 0;
            for ( ; (r < 0x10000) && !(codeStat_r & flagmask); r++) ;
            if (r >= 0x10000) break;
            range[nr][0] = (WORD)r;
            for ( ; (r < 0x10000) && (codeStat_r & flagmask); r++) ;
            range[nr][1] = (WORD)(r - 1);
        }
        if (nr >= 256) {
            //CloseHandle(h); DeleteFile(fname);
            h->close();
            h->remove();
            return -1;
        }

        /* レンジ数を書き込み */
        buf[0] = (BYTE)nr;
        if(hFormat)
            write_bytes(strm, buf, 1);
        else
            h->write((const char *)buf, 1);
        /* レンジテーブルを書き込み (Write range table)*/
        for (i = 0; i < nr; i++) {
            sr = range[i][0];
            er = range[i][1];
            qDebug() << QString("range 0x%1 -> 0x%2").arg(sr,2,16).arg(er,2,16);
            buf[1] = (BYTE)(sr >> 8); buf[0] = (BYTE)sr;
            buf[3] = (BYTE)(er >> 8); buf[2] = (BYTE)er;
            if(hFormat)
                write_bytes(strm, buf, 4);
            else
                h->write((const char *)buf, 4);
        }

        /* フォントイメージの書き込み (Write font image)*/
        nfnt = 0;
        for (i = 0; i < nr; i++) {
            sr = range[i][0];
            er = range[i][1];
            do {
                if(hFormat)
                    write_font(strm, sr, wf, hf);
                else
                    write_font(h, sr, wf, hf);

                nfnt++;
            } while (sr++ < er);
        }
    } else {	/* 半角フォント (Half-width font)*/
        /* フォントイメージの書き込み (Write font image)*/
        if(hFormat)
            for (i = 0; i < 256; write_font(strm, i++, wf, hf)) ;
        else
            for (i = 0; i < 256; write_font(h, i++, wf, hf)) ;

        nfnt = 256;
    }

    if(hFormat)
    {
        *strm << "\n};\n";
        *strm << "unsigned int " << fontName.toLatin1() << "_size = " << QString::number(byte_count).toLatin1() << ";\n";
    }
    //CloseHandle(h);
    h->close();
    if(hFormat && Dbcs)
    {
        unsigned int nc, sb, eb;
        char bc;
        const uint8_t *cblk;
        cblk = (uint8_t *)fontx2->data()+18; nc = 0;  /* Code block table */
        bc = fontx2->at(17);
        sb = cblk[0] + cblk[1] * 0x100;  /* Get range of the code block */
        eb = cblk[2] + cblk[3] * 0x100;
        qDebug() << "DBCS format: number of code blocks=" << (unsigned int)bc;

        qDebug() << "1st range = " << QString("0x%1").arg(sb,2,16, QChar('0'))  << " -> "<< QString("0x%1").arg(eb,2,16, QChar('0'));
    }

    return nfnt;
}

/* フォントファイルの読み込み */
UINT MainWindow::load_file (bool opn
 /* 0:再読込, 1:名前を指定して読み込み (0: Reload, 1: Load by name)*/)
{
#if 0
    OPENFILENAME ofn;
    TCHAR onf[_MAX_PATH], *d, *s, c;
    UINT f, i;


    strcpy(onf, FontFile[Dbcs]);
    if (opn || !onf[0]) {
        ofn.lStructSize = sizeof (OPENFILENAME);
        ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.hwndOwner = hwnd;
        ofn.lpstrTitle = Str[9][Loc];
        ofn.lpstrInitialDir = 0;
        ofn.lpstrFilter = Str[10][Loc];
        ofn.lpstrCustomFilter = 0;
        ofn.nFilterIndex = 0;
        ofn.lpstrDefExt = 0;
        ofn.lpstrFile = onf;
        ofn.nMaxFile = sizeof onf;
        ofn.lpstrFileTitle = 0;
        if (!GetOpenFileName(&ofn)) return 0;
    }

    /* 選択したファイルを全部読み込む */
    s = d = onf + ofn.nFileOffset;
    d[-1] = '\\';
    do {
        i = 0;
        do d[i++] = c = *s++; while (c);
        f = read_fontxfile(onf);
    } while (f && *s);

    if (!f) MessageBox(hwnd, Str[11][Loc], Str[12][Loc], MB_OK | MB_ICONEXCLAMATION);

    return f;
#endif
    QString fileName = FontFile[Dbcs];
    UINT f, i;

    if(opn)
    {
        fileName = QFileDialog::getOpenFileName(this,
            tr("Open Font File"), currPath, tr("Font Files (*.fnt)"));
        if(fileName.isEmpty())
            return 0;   // cancel if nofile selected

    }
    QFileInfo inf(fileName);
    inFile = inf.absoluteFilePath();
    currPath = inf.path();

    /* 選択したファイルを全部読み込む(Read all selected files) */
    f = read_fontxfile(fileName);
    ui->idc_pvc->setEnabled(true);
    ui->idc_exp->setEnabled(true);
    return f;
}

/* BDFファイルのインポート (BDF file import)*/
long MainWindow::read_bdffile (QFile* fp)
{
    char str[128], *s;
    BYTE b1, b2;
    UINT nchr;
    long chr, lc;
    DWORD d;
    int lines =0;
    encoded_count =0;
    bitmap_count = 0;
    expectedChars = 0;
    int bbxWidth = 0;
    int bbxHeight = 0;

    fontMap.clear();
    BYTE* fontImage = new BYTE[0, MAX_FONT_WB * MAX_FONT_SQ];
    nchr = 0;
    chr = lc = -1;
    //while (fgets(str, sizeof str, fp))
    qint64 read;
    while(read = fp->readLine(str, sizeof(str)))
    {
        if(read == 0)
            break;
        if(read < 0)
        {
            qDebug() << "read error at line" << lines;
           break;
        }
        lines++;
        if (lc >= 0) {
            if (!memcmp(str, "ENDCHAR", 7)) {
                //CodeStat[chr] = 1;
                fontMap.insert(QChar((WORD)chr), fontImage);
                chr = lc = -1;
                nchr++;
                //fontImage = new QByteArray(MAX_FONT_WB * MAX_FONT_SQ, 0);
            } else {
                if (lc < MAX_FONT_SQ) {
                    s = str;
                    for (d = 0; d < MAX_FONT_WB; d++) {
                        int bix= MAX_FONT_WB * lc + d;
                        b1 = toupper(*s++);
                        if (b1 > '9') b1 -= 7;
                        if (b1 > 'F') break;
                        //FontImage[chr][lc][d] = b1 << 4;
                        b2 = toupper(*s++);
                        if (b2 > '9') b2 -= 7;
                        if (b2 > 'F') break;
                        //FontImage[chr][lc][d] |= b1 & 0x0F;
                        //fontImage->replace(bix, b1 | (b2&0x0F));
                        char c = b1 << 4 | (b2&0x0F);
                        fontImage[bix] = c ;
                    }
                    lc++;
                }
            }
            continue;
        }
        if(!memcmp(str, "ENDCHAR", 7))
        {
           qDebug()  << "ENDCHAR lc=" << lc << " chr=" << chr;
        }
        if (!memcmp(str, "ENCODING ", 9)) {
            s = str + 9;
            d = strtoul(s, &s, 0);
            qDebug() << "ENCODING " << (qint32)d;
            if (d >= 0x2121 && d <= 0x7E7E) {
                d -= 0x2121;
                b1 = (BYTE)(d >> 8); b2 = (BYTE)d;
                if (b1 & 1) b2 += 0x5E;
                if (b2 >= 0x3F) b2++;
                chr = ((b1 >> 1) << 8) + 0x8140 + b2;
                if (chr >= 0xA000) chr += 0x4000;
                if (!IS_DBC(chr)) chr = -1;
            } else {
                chr = (d < 0x100) ? (long)d : -1;
            }
            encoded_count++;
            continue;
        }
        if (chr >= 0 && !memcmp(str, "BITMAP", 6)) {
            //memset(FontImage[chr][0], 0, MAX_FONT_WB * MAX_FONT_SQ);
            if(fontMap.contains(QChar((WORD)chr)))
                fontImage = fontMap.value(QChar((WORD)chr));
            else
                fontImage = new BYTE[0,MAX_FONT_WB * MAX_FONT_SQ];
            lc = 0;
            bitmap_count++;
            continue;
        }
        if(!memcmp(str, "CHARS ", 6))
        {
             QString chars(str+6);
             expectedChars = chars.toInt();
             qDebug() << "expected chars=" << str;
             continue;
        }
        if(!memcmp(str, "BBX", 3))
        {
            QString chars(str);
            QStringList sl = chars.split(" ");
            if(sl.count() > 3)
            {
                bool ok;
                int w = sl.at(1).toInt(&ok);
                if(!ok) continue;
                int h = sl.at(2).toInt(&ok);
                if(!ok) continue;
                if(w > bbxWidth)
                    bbxWidth = w;
                if(h > bbxHeight)
                    bbxHeight = h;
            }
        }
    }
    if(bbxHeight > 0 && bbxWidth > 0)
        emit size_change(Dbcs, bbxWidth, bbxHeight);
    return nchr;
}

/* 各種形式のファイルからインポート (Import from files of various formats)*/
void MainWindow::import_file (
    //HWND hwnd	/* メインウィンドウのハンドル */
)
{
//	OPENFILENAME ofn;
//	TCHAR onf[_MAX_PATH];
//	LPCTSTR mes = Str[29][Loc];
    long nc;
//	FILE *fp;


//	ofn.lStructSize = sizeof (OPENFILENAME);
//	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
//	ofn.hwndOwner = hwnd;
//	ofn.lpstrTitle = Str[13][Loc];
//	ofn.lpstrInitialDir = 0;
//	ofn.lpstrFilter = Str[14][Loc];
//	ofn.lpstrCustomFilter = 0;
//	ofn.nFilterIndex = 0;
//	ofn.lpstrDefExt = 0;
//	ofn.lpstrFile = onf;
//	ofn.nMaxFile = sizeof onf;
//	ofn.lpstrFileTitle = 0;
//	onf[0] = 0;
//	if (!GetOpenFileName(&ofn)) return;
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,
                tr("Open Font File"), QDir::homePath(), tr("Bit Distribution Format (*.bdf)"));
    if(fileName.isEmpty())
        return;
    QFile* f = new QFile(fileName); 	/* X Window BDF形式 (BDF form)*/
    if(	f->open( QIODevice::ReadOnly | QIODevice::Text))
    {
        nc = read_bdffile(f);
        f->close();

        if(nc > 0)
        {
            //sprintf(onf, Str[15][Loc], nc);
            //MessageBox(hwnd, onf, Str[16][Loc], MB_OK);
            QMessageBox::critical(this, Str[16][Loc], QString(Str[15][Loc]).arg(nc), QMessageBox::Ok);
            ui->idc_pvc->setEnabled(true);
            ui->idc_exp->setEnabled(true);
            QFileInfo inf(fileName);
            inFile = inf.absoluteFilePath();
            currPath = inf.path();
            setWindowTitle(inf.baseName());
            if(inf.baseName().at(0).isLetter() && inf.baseName().length() <= 8)
                fontName = inf.baseName();
            FontFile[Dbcs] = currPath + QDir::separator() + inf.fileName();
        }
    }
    else {
        QMessageBox::information(this, Str[16][Loc], Str[29][Loc],  QMessageBox::Ok);
    }
}

/* 垂直バイト構成LCD用で書き出し */
long MainWindow::write_vlcdfile (
    QString fname,
    int szinfo )
{
#if 1
    DWORD i, bw, fw, fh, fhb, hc, vc, bc;
    BYTE d, bms, bmd, *fnt;
    BYTE buf[MAX_FONT_SQ * MAX_FONT_SQ / 8], *wp;
    QFile* h;

    //h = CreateFile(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    //if (h == INVALID_HANDLE_VALUE) return -1;
    h = new QFile(fname);
    if(!h->open(QIODevice::WriteOnly | QIODevice::Truncate))
        return -1;

    fw = FontWidth[0]; fh = FontHeight[0];
    fhb = (fh + 7) / 8;
    if (szinfo) {	/* サイズ情報 {x,y} */
        buf[0] = (BYTE)fw; buf[1] = (BYTE)fh;
        //WriteFile(h, buf, 2, &bw, 0);
        bw = h->write((char*)buf,2);
    }

    for (i = 0; i < 256; i++) {
        wp = buf;
        for (vc = 0; vc < fhb; vc++) {
            for (hc = 0; hc < fw; hc++) {
                //fnt = &FontImage[i][vc * 8][hc / 8];
                fnt = fontMap.value(i) + vc*8 + hc/8;
                bms = 0x80 >> (hc % 8);
                bc = (fh < vc * 8) ? fh - vc * 8 : 8;
                d = 0; bmd = 1;
                do {
                    if (*fnt & bms) d |= bmd;
                    bmd <<= 1;
                    fnt += MAX_FONT_WB;
                } while (--bc);
                *wp++ = d;
            }
        }
        //WriteFile(h, buf, fw * fhb, &bw, 0);
        bw = h->write((char*)buf,fw * fhb);
    }

    //CloseHandle(h);
    h->close();
#endif
    return 256;
}



/* 各種形式でエクスポート */
void MainWindow::export_file (
    //HWND hwnd	/* メインウィンドウのハンドル */
)
{
#if 1
//    OPENFILENAME fn;
//    TCHAR snf[_MAX_PATH], txf[_MAX_PATH];
//    LPCTSTR mes = Str[26][Loc];
    long nc;


//    fn.lStructSize = sizeof (OPENFILENAME);
//    fn.Flags = OFN_OVERWRITEPROMPT;
//    fn.hwndOwner = hwnd;
//    fn.lpstrTitle = Str[17][Loc];
//    fn.lpstrInitialDir = 0;
//    fn.lpstrFilter = Str[18][Loc];
//    fn.lpstrCustomFilter = 0;
//    fn.nFilterIndex = 0;
//    fn.lpstrDefExt = 0;
//    fn.lpstrFile = snf;
//    fn.nMaxFile = sizeof snf;
//    fn.lpstrFileTitle = 0;
//    snf[0] = 0;
//    if (!GetSaveFileName(&fn)) return;
    QString mes;
    QString selectedFilter;
    QString snf = QFileDialog::getSaveFileName(this, tr("Export file"), currPath, tr("1-Vertical byte configuration LCD without size header(*.lcd));;2-Vertical byte configuration LCD with size header(*.lcd));;3-FONTX file with only specified characters(*.fnt)"),&selectedFilter);
    if(snf.isEmpty())
        return;
    nc = 0;
    switch (selectedFilter.at(0).digitValue()) {
    case 1:	/* 垂直バイト構成LCD用(サイズヘッダなし) (For vertical byte configuration LCD (without size header)*/
        if (Dbcs) {	/* 半角専用 */
            mes = Str[19][Loc];
        } else {
            nc = write_vlcdfile(snf, 0);
        }
        break;
    case 2:	/* 垂直バイト構成LCD用(サイズヘッダあり) (For vertical byte configuration LCD (with size header))*/
        if (Dbcs) {	/* 半角専用 */
            mes = Str[19][Loc];
        } else {
            nc = write_vlcdfile(snf, 1);
        }
        break;
    case 3:	/* 指定文字のみ集めたFONTXファイル (FONTX file with only specified characters)*/
        if (!Dbcs) {	/* 全角専用 */
            mes = Str[27][Loc];
        } else {
            QFile *h;
            DWORD n, i;
            QByteArray tp;
            WORD wc;

//            fn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
//            fn.lpstrFile = txf;
//            fn.nMaxFile = sizeof txf;
//            fn.lpstrTitle = Str[28][Loc];
//            fn.lpstrFilter	= "Text file (*.txt)\0*.txt";
//            txf[0] = 0;
//            if (!GetOpenFileName(&fn)) return;
//            h = CreateFile(txf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            QString fn = QFileDialog::getOpenFileName(this, tr("text file with cas to select"), QString(), tr("text file(*.txt"));
            if (fn.isEmpty()) return;
            //for (n = 0; n < sizeof CodeStat; CodeStat[n] &= 1, n++) ;
            n = fontMap.count();
            //tp = malloc(0x10000);
            //ReadFile(h, tp, 0x10000, &n, 0);
            tp= h->readAll();
            n = tp.count();
            //CloseHandle(h);
            h->close();
            i = 0;
            while (i < n) {
                wc = (WORD)tp.at(i);
                if (i < n && IS_DBC1(wc) && IS_DBC2(tp[i])) {
                    wc = wc << 8 | tp[i++];
                    //if (CodeStat[wc] & 1) CodeStat[wc] |= 2;
                }
            }
            //free(tp);
            nc = write_fontxfile(snf, 2);
        }
    }

    if (nc > 0) {
        snf = QString(Str[20][Loc]).arg(nc);
        QMessageBox::information(this, Str[21][Loc], snf, QMessageBox::Ok);
    } else {
        //DeleteFile(snf);
        QFile f(snf);
        f.remove();
        QMessageBox::critical(this, Str[21][Loc], mes, QMessageBox::Ok);
    }
#endif
}

/* ファイルセーブ File save()*/
bool MainWindow::save_file (
    //HWND hwnd,	/* メインウィンドウのハンドル */
    int mode	/* 0:上書き保存, 1:別名保存 (0: Save the book, 1: Save the alias)*/
)
{
//	OPENFILENAME ofn;
//	TCHAR svf[_MAX_PATH];


//	if (mode || !FontFile[Dbcs][0]) {	/* 別名保存・新規保存のときはファイル名を入力 (Enter a file name when saving to a new name or saving a new file)*/
//		strcpy(svf, FontFile[Dbcs]);
//		ofn.lStructSize = sizeof (OPENFILENAME);
//		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//		ofn.hwndOwner = hwnd;
//		ofn.lpstrTitle = Dbcs ? Str[4][Loc] : Str[5][Loc];
//		ofn.lpstrInitialDir = NULL;
//		ofn.lpstrFilter = Str[6][Loc];
//		ofn.lpstrCustomFilter = NULL;
//		ofn.nFilterIndex = 0;
//		ofn.lpstrDefExt = "fnt";
//		ofn.lpstrFile = svf;
//		ofn.nMaxFile = sizeof svf;
//		ofn.lpstrFileTitle = 0;
//		if (!GetSaveFileName(&ofn)) return FALSE;
//		mode = 1;
//	} else {	/* 上書き保存の時は現在のファイル名を使用 */
//		strcpy(svf, FontFile[Dbcs]);
//	}

    QString path = currPath;
    if(mode || !FontFile[Dbcs].isEmpty())
        path = FontFile[Dbcs];
    if(path.contains("."))
        path = path.mid(0, path.indexOf("."));
    QString selectedFilter;
    QString svf = QFileDialog::getSaveFileName(this, Dbcs ? Str[4][Loc] : Str[5][Loc], path,
            tr("FONTX file (*.fnt );;C/C++ header file(*.h)"), &selectedFilter);


    QFileInfo inf(svf);
    if(!(inf.suffix() == "fnt" || inf.suffix() == "h"))
    {
        if(inf.suffix().isEmpty())
        {
            if(selectedFilter.startsWith("FONTX"))
                svf.append(".fnt");
            else
            {
                svf.append(".h");
            }
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("The file must have either \"fnt\" or \"h\" extension"), QMessageBox::Ok);
            return -1;
        }
        inf = QFileInfo(svf);
        if(inf.suffix() == "h" )
        {
            bool cont = true;
            while (cont)
            {
                if(!inf.baseName().at(0).isLetter() || inf.baseName().length() > 8)
                {
                    bool ok;
                    QString text = QInputDialog::getText(this, "Font name:",
                                          "The font needs a name 8 bytes or less starting with a letter\nThe base filename will also be changed to this.",
                                          QLineEdit::Normal, "??", &ok);
                    if(!ok)
                        return 0;
                    if(!(text.length() > 0 && text.at(0).isLetter() && text.length() <= 8 ))
                    {
                        int rslt = QMessageBox::critical(this, "Error", "An invalid name was entered. ", QMessageBox::Cancel, QMessageBox::Retry);
                        switch (rslt) {
                        case QMessageBox::Cancel:
                            return -1;
                        case QMessageBox::Retry:
                            continue;
                        default:
                            continue;
                        }
                    }
                    fontName = text;
                    svf = inf.path() + QDir::separator() + text + ".h";
                    inf = QFileInfo(svf);
                    if(inf.exists())
                    {
                        int rslt = QMessageBox::question(this, tr("File exists"), tr("The file exists. Do you wish to overwrite it?"), QMessageBox::Yes, QMessageBox::Retry, QMessageBox::Cancel);
                        switch(rslt)
                        {
                        case QMessageBox::Yes:
                            cont = false;
                            break;
                        case QMessageBox::Cancel:
                            return -1;
                        case QMessageBox::Retry:
                            break;
                        }
                    }
                    break;
                }
                else
                {
                    fontName = inf.baseName();
                    break;
                }
            }
        }
    }
    if (write_fontxfile(svf, 1) > 1) {
        if (mode) {
            FontFile[Dbcs] = svf;
            rfsh_fontinfo();
        }
        return true;
    } else {
        QFile file (svf);
        file.remove();
        QMessageBox::critical(this, Str[8][Loc], Str[7][Loc], QMessageBox::Ok);
        return false;
    }
}

/* ファイルから格納順に1文字のフォントデータを読み込み (Read font data of 1 character from the file in storage order)*/
void MainWindow::read_font (
    //HANDLE h,	/* ファイルハンドル */
    QFile* f,
    QChar code,	/* 読み込み先の文字コード (Character code of the read destination)*/
    UINT fw,	/* フォントの幅[dot] (Font width [dot])*/
    UINT fh		/* フォントの高さ[dot] (Font height [dot])*/
)
{
    //BYTE buf[MAX_FONT_WB * MAX_FONT_SQ], *d, *s;
    DWORD /*br,*/ fwb, btr, v, h;
    //qint64 br;
    QByteArray buf;

//    s = buf;
//    d = FontImage[code][0];	/* 読み込み先 */
    fwb = (fw + 7) / 8;		/* 幅[バイト] (Width [bytes])*/
    btr = fwb * fh;			/* 1文字のバイト数 (The number of bytes in one character)*/

    //memset(s, 0, btr);
    //buf = QByteArray(btr, 0);
    //ReadFile(h, s, btr, &br, 0);
    //br = f->read((char*)s, btr);
    buf = f->read(btr);

    //memset(d, 0, MAX_FONT_WB * MAX_FONT_SQ);
    BYTE* fontImage;
    if(fontMap.contains(QChar(code)))
        fontImage = fontMap.value(QChar(code));
    else
        fontImage = new BYTE[0, MAX_FONT_WB * MAX_FONT_SQ];
//    for (v = 0; v < fh; v++) {
//        memcpy(d, s, fwb);
//        s += fwb; d += MAX_FONT_WB;
//    }
    int ix = 0;
    int bix = 0;
    for (v = 0; v < fh; v++)
    {
        for(int i=0; i < fwb; i++)
        {
          fontImage[ix + i] =  buf.at(bix++);
        }
        ix += MAX_FONT_WB;
    }
    fontMap.insert(code, fontImage);
}


/* FONTXファイルから読み込み */
UINT MainWindow::read_fontxfile (	/* 0:失敗, 1:半角, 2:全角 (0: Failure, 1: Halfwidth, 2: Fullwidth)*/
    QString fname
)
{
    //HANDLE h;
    BYTE buf[20], nr, ri, dbcs;
    WORD range[256][2], i;
    qint64 br;
    UINT fw, fh;

//	h = CreateFile(fname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//	if (h == INVALID_HANDLE_VALUE) return 0;
    QFile* f = new QFile(fname);
    if(!f->open(QIODevice::ReadOnly))
        return 0;

    /* FONTXヘッダ読み込み */
//	if (!ReadFile(h, buf, 17, &br, 0) || br != 17 || memcmp(buf, "FONTX2", 6)) {
//		CloseHandle(h); return 0;
//	}

    if((br = f->read((char*)buf, 17))!= 17 || memcmp(buf, "FONTX2", 6))
    {
        f->close();
        return 0;
    }
    fw = buf[14]; fh = buf[15];		/* 幅と高さ (Width and height)*/
    dbcs = buf[16];					/* 半角/全角フラグ (Half-width/full-width flag)*/
    emit size_change(Dbcs, fw, fh);

    if ((dbcs && Loc) || dbcs > 1 || fw < 4 || fw > MAX_FONT_SQ || fh < 4 || fh > MAX_FONT_SQ) {
        f->close();
        return 0;
    }

    if (dbcs) {
        /* レンジテーブルを読み込み (Read range table)*/
        //if (!ReadFile(h, &nr, 1, &br, 0) || br != 1 || !nr) {
        if((br=f->read((char*)&nr, 1) !=1 || !nr))
        {
            f->close(); return 0;
        }
        if ((br=f->read((char*)range, (4 * nr))) != (4 * nr)) {
            f->close(); return 0;
        }

        /* フォントデータの読み込み (Read font data)*/
        //memset(&CodeStat[0x100], 0, sizeof CodeStat - 0x100);
        for (ri = 0; ri < nr; ri++) {
            i = range[ri][0];
            do {
                read_font(f, i, fw, fh);
                //CodeStat[i] = 1;
            } while (i++ < range[ri][1]);
        }
    } else {
        /* フォントデータの読み込み (Read font data)*/
        for (i = 0; i < 0x100; i++) {
            qint64 p = f->pos();
            read_font(f, i, fw, fh);
            //CodeStat[i] = 1;
        }
    }

    f->close();

    /* フォント情報更新 */
    FontWidth[dbcs] = fw; FontHeight[dbcs] = fh;
    FileChanged[dbcs] = 0;
    //strcpy(FontFile[dbcs], fname);
    FontFile[dbcs] = fname;
    return 1 + dbcs;
}


void MainWindow::on_tabWidget_current_changed(int iTab)
{
    //if (nmupdown->hdr.code != TCN_SELCHANGE) break;
    Dbcs = iTab;//(UINT)TabCtrl_GetCurSel(hTab);
    if (Dbcs > 1) Dbcs = 0;
    if (Dbcs && Loc) {
        Dbcs = 0;
        //TabCtrl_SetCurSel(hTab, 0);
        ui->idc_tab1->setCurrentIndex(0);
        QMessageBox::critical(this, "Error", "User locale is not Japanese.");
    }
    rfsh_fontinfo();
    change_code(0);

}

void MainWindow::fh_changed(int Dbcs,int fh)
{
    if(FontHeight[Dbcs] != fh)
    {
     FontHeight[Dbcs] = fh;
     rfsh_fontinfo();
     file_set_changed(true);
     tabs.at(Dbcs)->update();
     emit size_change(Dbcs, fh,  FontWidth[Dbcs]);
    }
}

void MainWindow::fw_changed(int Dbcs,int fw)
{
    if(FontWidth[Dbcs] != fw)
    {
        FontWidth[Dbcs] = fw;
        rfsh_fontinfo();
        file_set_changed(true);
        tabs.at(Dbcs)->update();
        emit size_change(Dbcs, FontHeight[Dbcs],  fw);
   }
}

