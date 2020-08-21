#include "tabwidget.h"
#include "ui_tabwidget.h"
#include "editwindow.h"
#include "mainwindow.h"

TabWidget::TabWidget(BYTE Dbcs, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabWidget)
{
    ui->setupUi(this);
    mainWindow = (MainWindow*)parent;
    this-> Dbcs = Dbcs;
    ui->idc_fw->setValue(mainWindow->FontWidth[mainWindow->Dbcs]);
    ui->idc_fh->setValue(mainWindow->FontHeight[mainWindow->Dbcs]);
    if(Dbcs)
        ui->idc_remap->setHidden(true);

    WORD code;
    code = mainWindow->CurrentCode[mainWindow->Dbcs];
    ui->idc_char->setText(QChar(code));
    ui->idc_code->setText(QString::number(code, 2, 16));

    connect(ui->idc_load, &QPushButton::clicked, [=]{
        UINT f;
        f = mainWindow->load_file(1);
        if (f)
            mainWindow->change_tab(f);
    });
    connect(ui->idc_save, &QPushButton::clicked, [=]{
        if (mainWindow->FileChanged[this->Dbcs]) {
            if (mainWindow->save_file(0)) {
                mainWindow->file_set_changed(false);
            }
        }
    });
    connect(ui->idc_saveas, &QPushButton::clicked, [=]{
        if (mainWindow->save_file( 1)) {
            mainWindow->file_set_changed(false);
        }
    });
    connect(ui->idc_prev, &QPushButton::clicked, [=]{
        mainWindow->change_code(-1);
    });
    connect(ui->idc_next, &QPushButton::clicked, [=]{
        mainWindow->change_code(1);
    });
    connect(ui->idc_copy, &QPushButton::clicked, [=]{

    });
    connect(ui->idc_paste, &QPushButton::clicked, [=]{

    });
    connect(ui->idc_rotl, &QPushButton::clicked, [=]{
        mainWindow->edit_rotate(1);
    });
    connect(ui->idc_rotr, &QPushButton::clicked, [=]{
        mainWindow->edit_rotate(2);
    });
    connect(ui->idc_rotu, &QPushButton::clicked, [=]{
        mainWindow->edit_rotate(3);
    });
    connect(ui->idc_rotd, &QPushButton::clicked, [=]{
        mainWindow->edit_rotate(-1);
    });

    connect(ui->idc_inv, &QPushButton::clicked, [=]{
        //for (n = 0, bp = EditFont[0]; n < sizeof EditFont; *bp = ~*bp, bp++, n++) ;
        int sz = MAX_FONT_SQ * MAX_FONT_WB;
        for(int i=0; i < sz; i++)
           mainWindow->EditFont[i] = -mainWindow->EditFont[i];
        mainWindow->edit_set_changed(true);
        //InvalidateRect(hEdit, 0, TRUE);
        update();
    });

    connect(ui->idc_clr, &QPushButton::clicked, [=]{
        int sz = MAX_FONT_SQ * MAX_FONT_WB;
        for(int i=0; i< sz; i++)
            mainWindow->EditFont[i] = (char)0;
        mainWindow->edit_set_changed(true);
        //InvalidateRect(hEdit, 0, TRUE);
        update();
    });

    connect(ui->idc_code, &QLineEdit::editingFinished, [=]{
     bool ok;
     uint nHex = ui->idc_code->text().toUInt(&ok, 16);
     mainWindow->CurrentCode[mainWindow->Dbcs] = nHex;
     mainWindow->get_font(nHex, mainWindow->EditFont);
     mainWindow->change_code(0);
     emit code_changed(mainWindow->Dbcs, nHex);
     update();
    });
    connect(mainWindow, SIGNAL(size_change(int,int,int)), this, SLOT(size_change(int,int,int)));
    connect(ui->idc_fw, SIGNAL(valueChanged(int)), this, SLOT(idc_fw_value_changed(int)));
    connect(ui->idc_fh, SIGNAL(valueChanged(int)), this, SLOT(idc_fh_value_changed(int)));
    connect(ui->idc_remap, &QCheckBox::clicked, [=]{
        mainWindow->tabs.at(1)->setVisible(ui->idc_remap->isChecked());
    });

    ui->frame_2->setLayout(new QVBoxLayout());
    ui->frame_2->layout()->addWidget(new EditWindow(mainWindow->Dbcs, mainWindow->FontHeight[mainWindow->Dbcs],
                                                    mainWindow->FontWidth[mainWindow->Dbcs],mainWindow->EditFont,
                                                    mainWindow));
}

void TabWidget::size_change(int Dbcs, int fw, int fh)
{
    if(this->Dbcs == Dbcs)
    {
        if(fh != ui->idc_fh->value())
            ui->idc_fh->setValue(fh);
        if(fw != ui->idc_fw->value())
            ui->idc_fw->setValue(fw);
    }
}

void TabWidget::idc_fh_value_changed(int)
{
    if(ui->idc_fh->value() != mainWindow->FontHeight[mainWindow->Dbcs])
        emit fh_changed(mainWindow->Dbcs, ui->idc_fh->value());
}

void TabWidget::idc_fw_value_changed(int)
{
    if(ui->idc_fh->value() != mainWindow->FontWidth[mainWindow->Dbcs])
    emit fw_changed(mainWindow->Dbcs, ui->idc_fw->value());
}

TabWidget::~TabWidget()
{
    delete ui;
}

/* コード入力エディトボックスから文字コードを取得 (Get character code from code input edit box)*/
int TabWidget::get_code (void)
{
    //TCHAR str[16], *s;
    QString str;
    UINT l;
    WORD c;


    ui->idc_code->text();
    l = str.length();

    bool ok;
    if (Dbcs) {
        if (l == 2) {
            //c = (BYTE)str[0] * 256 + (BYTE)str[1];
            c = str.toInt(&ok,16);
            if (ok && IS_DBC(c)) return c;
        }
        if (l == 4) {
            //c = (WORD)strtoul(str, &s, 16);
            c = str.toInt(&ok,16);
            //if (s == &str[4] && IS_DBC(c)) return c;
            if(ok && IS_DBC(c)) return c;
        }
        return -1;
    } else {
        if (l == 1) {
            //c = (BYTE)str[0];
            c = str.toInt(&ok, 16);
            if (ok &&IS_SBC(c)) return c;
        }
        if (l == 2) {
            //c = (WORD)strtoul(str, &s, 16);
            //if (s == &str[2]) return c;
            if (ok &&IS_SBC(c)) return c;
        }
        return -1;
    }
}
