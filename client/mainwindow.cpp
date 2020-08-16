#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    plot = new QCustomPlot;
    plot->xAxis->setRange(0, 60);
    plot->yAxis->setRange(0, 5000);

    ui->verticalLayout_5->addWidget(plot);
    ui->verticalLayout_5->setStretch(0,1);
    ui->verticalLayout_5->setStretch(1,4);
//    // 设置x和y坐标的标尺
//    ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, 5000, 1000);
//    ui->qwtPlot->setAxisScale(QwtPlot::xBottom, 0, 60, 5);
    // 初始化坐标数据数组
    for(int i =0;i<100;i++)
    {
       yy[i] = 0;
    }

    adr = new QHostAddress;

    my_timer = new QTimer(this);
    connect(my_timer, SIGNAL(timeout()), this, SLOT(slot_my_timer()));

    usock = new QUdpSocket(this);
    connect(usock, SIGNAL(readyRead()), this, SLOT(update_data()));

    ui->lcdNumber->setDecMode();
    ui->lcdNumber->setDigitCount(4);

    query_vol = new QByteArray;
    query_vol->push_back((char)1);
    query_vol->push_back((char)1);
    query_vol->push_back((char)1);
    query_vol->push_back((char)0);
    //qDebug("%d %d %d", datagram.at(0), datagram.at(1), datagram.at(2));


}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::slot_my_timer(void)
{
    usock->writeDatagram(query_vol->data(), query_vol->size(),*adr,port);
}

void MainWindow::update_data(){
    QByteArray datagram;
    while(usock->hasPendingDatagrams())
    {
        // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        datagram.resize(usock->pendingDatagramSize());
        // 接收数据报，将其存放到datagram中
        usock->readDatagram(datagram.data(), datagram.size());
        //qDebug("%d %d %d\n", datagram.at(0), datagram.at(1) ,  datagram.at(2));
    }
    if(datagram.at(0) == 2) return;
    int new_vol_val = datagram.at(1)*100 + datagram.at(2);

    ui->lcdNumber->display(new_vol_val);
    if(1000 <= new_vol_val && new_vol_val <= 4000){
        ui->vol_alert_lineEdit->setText("Normal");
    }else{
        ui->vol_alert_lineEdit->setText("Warning");
    }

    plot->clearGraphs(); // 清除上一次的显示数据
    QVector<double> vec_x, vec_y;
    for(int i =59;i> 0;i--)
    {
       yy[i] = yy[i - 1];
       vec_x.push_back(i);
       vec_y.push_back(yy[i]);
    }
    yy[0] = new_vol_val; // 在40至60之间生成随机数

    vec_x.push_back(0);
    vec_y.push_back(yy[0]);

    plot->addGraph();
    plot->graph(0)->setData(vec_x,vec_y);

    plot->replot();
}


void MainWindow::on_bind_bt_clicked()
{
    adr->setAddress(ui->IP_lineEdit->text());
    port = ui->Port_lineEdit->text().toInt();
}

void MainWindow::on_start_bt_clicked()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime())); // 设置随机数种子
    my_timer->start(1000);
}

void MainWindow::on_led43_on_bt_clicked()
{
    QByteArray control_msg;
    control_msg.push_back((char)2);
    control_msg.push_back((char)43);
    control_msg.push_back((char)1);
    control_msg.push_back((char)0);
    usock->writeDatagram(control_msg.data(), control_msg.size(),*adr,port);
}

void MainWindow::on_led43_off_bt_clicked()
{
    QByteArray control_msg;
    control_msg.push_back((char)2);
    control_msg.push_back((char)43);
    control_msg.push_back((char)2);
    control_msg.push_back((char)0);
    usock->writeDatagram(control_msg.data(), control_msg.size(),*adr,port);
}

void MainWindow::on_led44_on_bt_clicked()
{
    QByteArray control_msg;
    control_msg.push_back((char)2);
    control_msg.push_back((char)44);
    control_msg.push_back((char)1);
    control_msg.push_back((char)0);
    usock->writeDatagram(control_msg.data(), control_msg.size(),*adr,port);
}

void MainWindow::on_led44_off_bt_clicked()
{
    QByteArray control_msg;
    control_msg.push_back((char)2);
    control_msg.push_back((char)44);
    control_msg.push_back((char)2);
    control_msg.push_back((char)0);
    usock->writeDatagram(control_msg.data(), control_msg.size(),*adr,port);
}

void MainWindow::on_pushButton_clicked()
{
    this->close();
}

void MainWindow::on_pushButton_2_clicked()
{
    QByteArray control_msg;
    control_msg.push_back((char)3);
    control_msg.push_back((char)1);
    control_msg.push_back((char)1);
    control_msg.push_back((char)0);
    usock->writeDatagram(control_msg.data(), control_msg.size(),*adr,port);
    my_timer->stop();
    my_timer->start();
}
