#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networktimestamp/networktimestamp.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked() {

    QDateTime time;
    time.setMSecsSinceEpoch(NetworkTimestamp::shareInstance()->currentMSTimestamp());

    ui->textBrowser->append(time.toString());
    ui->textBrowser->append(QString().setNum(time.toMSecsSinceEpoch()));
    return;


}
