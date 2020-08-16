#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>
#include "./qcustomplot.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();
private slots:
    void slot_my_timer(void);
    void update_data();
    void on_bind_bt_clicked();
    void on_start_bt_clicked();

    void on_led43_on_bt_clicked();

    void on_led43_off_bt_clicked();

    void on_led44_on_bt_clicked();

    void on_led44_off_bt_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QCustomPlot *plot;
    QTimer *my_timer;
    QUdpSocket *usock;
    QHostAddress *adr;
    QByteArray *query_vol;
    int port;
    int yy[150];
};

#endif // MAINWINDOW_H
