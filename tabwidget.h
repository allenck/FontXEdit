#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QWidget>
#include "defines.h"
#include <editwindow.h>

namespace Ui {
class TabWidget;
}

class MainWindow;
class TabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabWidget(BYTE Dbcs, QWidget *parent = 0);
    ~TabWidget();

    Ui::TabWidget *ui;
    int get_code (void);

signals:
    void fw_changed(int, int);
    void fh_changed(int, int);
    void code_changed(int, WORD);
private:
 MainWindow* mainWindow;
 BYTE Dbcs;
 bool setting = false;

private slots:
 void size_change(int Dbcs, int fw, int fh); // dbcs, fw, fh
 void idc_fh_value_changed(int fh);
 void idc_fw_value_changed(int fw);


 friend class MainWindoe;
};

#endif // TABWIDGET_H
