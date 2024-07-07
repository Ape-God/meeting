#include "meetingClient.h"
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QDebug>
#include <QPainter>
#include "myvideosurface.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QScrollBar>
#include <QHostAddress>
#include <QTextCodec>
#include <QDateTime>
#include <QCompleter>
#include <QStringListModel>
#include <QSound>
#include <QLabel>


meetingClient::meetingClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::meetingClientClass())
{
    ui->setupUi(this);
    /*************************************************/
    // ���������
    _myTcpSocketThread = new QThread();
    _myTcpSocket = new MyTcpSocket(ui);
    _myTcpSocket->moveToThread(_myTcpSocketThread);

    
    connect(this, SIGNAL(connectToServer(QString, quint16)),
        _myTcpSocket, SLOT(connectSlot(QString, quint16)));
    // ����������
    connect(_myTcpSocket, &QTcpSocket::connected, this, [&]() {
        QMessageBox::information(nullptr, "Connection success", "���ӳɹ�",
        QMessageBox::Yes, QMessageBox::Yes);
        ui->statusBar->showMessage("���ӳɹ�");
        ui->btnConnect->setText("�Ͽ�");

        ui->btnRoomCreate->setEnabled(true);
        ui->btnRoomJoin->setEnabled(true);
        ui->btnRoomExit->setEnabled(false);
    }, Qt::QueuedConnection);
    // �Ͽ�����
    connect(_myTcpSocket, &QTcpSocket::disconnected, this, [&]() {
        QMessageBox::information(nullptr, "Disconnection success", "�ѶϿ�����",
        QMessageBox::Yes, QMessageBox::Yes);
        ui->statusBar->showMessage("�ѶϿ�����");
        ui->btnConnect->setText("����");

        ui->btnRoomCreate->setEnabled(false);
        ui->btnRoomJoin->setEnabled(false);
        ui->btnRoomExit->setEnabled(false);

    }, Qt::QueuedConnection);
    connect(_myTcpSocket, &MyTcpSocket::recvRoomStatus, this, &meetingClient::recvRoomStatusSlot);
    connect(_myTcpSocket, &MyTcpSocket::recvMsg, this, &meetingClient::recvMsgSlot);
    _myTcpSocketThread->start();


    /*************************************************/
    // �ı���Ϣ
    connect(ui->btnSendMsg, &QPushButton::clicked, this, &meetingClient::on_lineSendMsg_returnPressed);



    /*************************************************/
    // ��������ͷ
    _camera = new QCamera(this);
    // ����ͷ������
    connect(_camera, SIGNAL(error(QCamera::Error)), this, SLOT(cameraError(QCamera::Error)));
    _imagecapture = new QCameraImageCapture(_camera);
    _myVideoSurface = new MyVideoSurface(this);

    connect(_myVideoSurface, SIGNAL(frameAvailable(QVideoFrame)), this, SLOT(cameraImageCapture(QVideoFrame)));
    _camera->setViewfinder(_myVideoSurface);
    _camera->setCaptureMode(QCamera::CaptureStillImage);
}

meetingClient::~meetingClient()
{
    delete ui;
}

void meetingClient::on_btnConnect_clicked() {
    QString ip = ui->lineSeverIp->text(), port = ui->lineSeverPort->text();
    ui->statusBar->showMessage("Connecting " + ip + ":" + port);

    QRegExp ipreg("((2{2}[0-3]|2[01][0-9]|1[0-9]{2}|0?[1-9][0-9]|0{0,2}[1-9])\\.)((25[0-5]|2[0-4][0-9]|[01]?[0-9]{0,2})\\.){2}(25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2})");

    QRegExp portreg("^([0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$");
    QRegExpValidator ipvalidate(ipreg), portvalidate(portreg);
    int pos = 0;
    if (ipvalidate.validate(ip, pos) != QValidator::Acceptable)
    {
        QMessageBox::warning(this, "Input Error", "Ip Error", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (portvalidate.validate(port, pos) != QValidator::Acceptable)
    {
        QMessageBox::warning(this, "Input Error", "Port Error", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    emit connectToServer(ip, port.toUShort());
}

void meetingClient::on_btnRoomCreate_clicked(){
    uint8_t* sendData = _myTcpSocket->_sendDequeBuffer->popFrontFrom_dequeBuffEmpty();
    if (sendData == NULL) {
        QMessageBox::warning(this, "", "���Ժ�����", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    //
    MSG_C* msg_c = reinterpret_cast<MSG_C*>(sendData);
    msg_c->header[0] = 0X55;
    msg_c->header[1] = 0XAA;
    msg_c->type = MESGType::CreatRoom;
    u8u16(msg_c->len) = MSG_C_HEAD_SIZE;
    msg_c->crc = 0X00;

    emit _myTcpSocket->sendMsg(sendData);
}

void meetingClient::on_btnRoomJoin_clicked() {
    uint8_t* sendData = _myTcpSocket->_sendDequeBuffer->popFrontFrom_dequeBuffEmpty();
    if (sendData == NULL) {
        QMessageBox::warning(this, "", "���Ժ�����", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    // 
    QString roomId = ui->lineRoomId->text();
    int strl = roomId.length();
    if (strl != 5) {
        QMessageBox::warning(this, "", "�����Ϊ5λ����", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    //
    MSG_C* msg_c = reinterpret_cast<MSG_C*>(sendData);
    msg_c->header[0] = 0X55;
    msg_c->header[1] = 0XAA;
    msg_c->type = MESGType::JoinRoom;
    u8u16(msg_c->len) = MSG_C_HEAD_SIZE + strl;
    msg_c->crc = 0X00;
    strcpy(msg_c->data, roomId.toStdString().c_str());
    emit _myTcpSocket->sendMsg(sendData);
}

void meetingClient::on_btnRoomExit_clicked() {
    uint8_t* sendData = _myTcpSocket->_sendDequeBuffer->popFrontFrom_dequeBuffEmpty();
    if (sendData == NULL) {
        QMessageBox::warning(this, "", "���Ժ�����", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    //
    MSG_C* msg_c = reinterpret_cast<MSG_C*>(sendData);
    msg_c->header[0] = 0X55;
    msg_c->header[1] = 0XAA;
    msg_c->type = MESGType::ExitRoom;
    u8u16(msg_c->len) = MSG_C_HEAD_SIZE;
    msg_c->crc = 0X00;

    emit _myTcpSocket->sendMsg(sendData);
}


void meetingClient::recvRoomStatusSlot(MESGType type, quint32 ip, quint16 port, QString roomId) {
    // qDebug() << (int)type;
    // qDebug() << _myTcpSocket->getIP() << _myTcpSocket->getPort() << " " << ip << port;
    if (type == MESGType::CreatRoomResp) {
        ui->statusBar->showMessage("�Ѵ������䣺" + roomId);
        ui->lineRoomId->setText(roomId);
        QMessageBox::information(nullptr, "success", "�Ѵ������䣺" + roomId,
            QMessageBox::Yes, QMessageBox::Yes);

        ui->btnRoomCreate->setEnabled(false);
        ui->btnRoomJoin->setEnabled(false);
        ui->btnRoomExit->setEnabled(true);
        _myTcpSocket->setIpAndPort(ip, port);
        _myTcpSocket->_partners.insert(_myTcpSocket->getIPAndPort(ip, port));
    }
    else if (type == MESGType::JoinRoomSuccess && _myTcpSocket->isBelongMe(ip, port)) {
        ui->statusBar->showMessage("�Ѽ��뷿�䣺" + roomId);
        ui->lineRoomId->setText(roomId);
        QMessageBox::information(nullptr, "success", "�Ѽ��뷿�䣺" + roomId,
            QMessageBox::Yes, QMessageBox::Yes);

        ui->btnRoomCreate->setEnabled(false);
        ui->btnRoomJoin->setEnabled(false);
        ui->btnRoomExit->setEnabled(true);
        _myTcpSocket->setIpAndPort(ip, port);
    } else if (type == MESGType::JoinRoomSuccess && !_myTcpSocket->isBelongMe(ip, port)) {
        // �б���û����뷿��
        ui->statusBar->showMessage(QString::number(port) + "���뷿��");
    }
    else if (type == MESGType::JoinRoomAlreadyJoin) {
        ui->statusBar->showMessage("�Ѽ��뷿�䣺" + ui->lineRoomId->text() + " ���ܼ�����������");
        QMessageBox::information(nullptr, "success", "�Ѽ��뷿�䣺" + ui->lineRoomId->text(),
            QMessageBox::Yes, QMessageBox::Yes);

        ui->btnRoomCreate->setEnabled(false);
        ui->btnRoomJoin->setEnabled(false);
        ui->btnRoomExit->setEnabled(true);
    }
    else if (type == MESGType::JoinRoomNotFound) {
        ui->statusBar->showMessage("�����δ�ҵ���" + ui->lineRoomId->text());
        QMessageBox::information(nullptr, "", "�����δ�ҵ���" + ui->lineRoomId->text(),
            QMessageBox::Yes, QMessageBox::Yes);
    }
    else if (type == MESGType::ExitRoomSuccess && _myTcpSocket->isBelongMe(ip, port)) {
        ui->statusBar->showMessage("���˳����䣺" + ui->lineRoomId->text());
        QMessageBox::information(nullptr, "success", "���˳����䣺" + ui->lineRoomId->text(),
            QMessageBox::Yes, QMessageBox::Yes);

        ui->btnRoomCreate->setEnabled(true);
        ui->btnRoomJoin->setEnabled(true);
        ui->btnRoomExit->setEnabled(false);
    }
    else if (type == MESGType::ExitRoomSuccess && !_myTcpSocket->isBelongMe(ip, port)) {
        ui->statusBar->showMessage(port + "�˳�����");
    }
}


void meetingClient::on_lineSendMsg_returnPressed() {
    QString msg_text = ui->lineSendMsg->text();
    ui->lineSendMsg->clear();
    if (msg_text.isEmpty()) return;

    uint8_t* sendData = _myTcpSocket->_sendDequeBuffer->popFrontFrom_dequeBuffEmpty();
    if (sendData == NULL) {
        QMessageBox::warning(this, "", "���Ժ�����", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    //
    MSG_C* msg_c = reinterpret_cast<MSG_C*>(sendData);
    msg_c->header[0] = 0X55;
    msg_c->header[1] = 0XAA;
    msg_c->type = MESGType::textMsg;
    msg_c->crc = 0X00;
    strcpy(msg_c->data, msg_text.toStdString().c_str());
    u8u16(msg_c->len) = MSG_C_HEAD_SIZE + strlen(msg_text.toStdString().c_str());
    // qDebug() << strlen(msg_text.toStdString().c_str());
    if(_myTcpSocket) emit _myTcpSocket->sendMsg(sendData);

    addTextMsg(msg_text, QString::fromStdString(_myTcpSocket->getLocalIPAndPort()), ChatMessage::User_Me);
}

void meetingClient::addTextMsg(QString msgText, QString name, ChatMessage::User_Type type) {
    QString time = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
    ChatMessage* message = new ChatMessage(ui->listTextMsg);
    QListWidgetItem* item = new QListWidgetItem();
    // ��ʾʱ��
    dealMessageTime(time);
    dealMessage(message, item, msgText, time, name, type);
    message->setTextSuccess();
}

void meetingClient::dealMessage(ChatMessage* messageW, QListWidgetItem* item, QString text, QString time, QString name, ChatMessage::User_Type type)
{
    ui->listTextMsg->addItem(item);
    messageW->setFixedWidth(ui->listTextMsg->width());
    QSize size = messageW->fontRect(text);
    item->setSizeHint(size);
    messageW->setText(text, time, size, name, type);
    ui->listTextMsg->setItemWidget(item, messageW);
}

void meetingClient::dealMessageTime(QString curMsgTime)
{
    bool isShowTime = false;
    if (ui->listTextMsg->count() > 0) {
        QListWidgetItem* lastItem = ui->listTextMsg->item(ui->listTextMsg->count() - 1);
        ChatMessage* messageW = (ChatMessage*)ui->listTextMsg->itemWidget(lastItem);
        int lastTime = messageW->time().toInt();
        int curTime = curMsgTime.toInt();
        isShowTime = ((curTime - lastTime) > 60); // ������Ϣ���һ����
    } else {
        isShowTime = true;
    }
    if (isShowTime) {
        ChatMessage* messageTime = new ChatMessage(ui->listTextMsg);
        QListWidgetItem* itemTime = new QListWidgetItem();
        ui->listTextMsg->addItem(itemTime);
        QSize size = QSize(ui->listTextMsg->width(), 40);
        messageTime->resize(size);
        itemTime->setSizeHint(size);
        messageTime->setText(curMsgTime, curMsgTime, size);
        ui->listTextMsg->setItemWidget(itemTime, messageTime);
    }
}

void meetingClient::recvMsgSlot(MESGType type, quint32 ip, quint16 port, void* data) {
    MSG_S* msg_s = reinterpret_cast<MSG_S*>(data);
    if (type == MESGType::textMsgResp || !_myTcpSocket->isBelongMe(ip, port)) {
        // ����ʾ�������һ���µ���Ϣ
        addTextMsg(msg_s->data, QString::fromStdString(_myTcpSocket->getIPAndPort(ip, port)), ChatMessage::User_She);
    }
    else if (type == MESGType::videoMsgResp || !_myTcpSocket->isBelongMe(ip, port)) {

    }
    _myTcpSocket->_recvDequeBuffer->pushBackTo_dequeBuffEmpty((uint8_t*)data);
}








void meetingClient::cameraError(QCamera::Error) {
    QMessageBox::warning(this, "Camera error", _camera->errorString(), QMessageBox::Yes, QMessageBox::Yes);
}

void meetingClient::cameraImageCapture(QVideoFrame frame) {
    if (frame.isValid() && frame.isReadable())
    {
        // ��ʾͼ��
        QImage videoImg = QImage(frame.bits(), frame.width(), frame.height(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));

        QTransform matrix;
        matrix.rotate(180.0);

        QImage img = videoImg.transformed(matrix, Qt::FastTransformation).scaled(ui->labVideo->size());
        ui->labVideo->setPixmap(QPixmap::fromImage(img).scaled(ui->labVideo->size()));

        //if (partner.size() > 1)
        //{
        //    emit pushImg(img);
        //}

        //if (_mytcpSocket->getlocalip() == mainip)
        //{
        //    ui->mainshow_label->setPixmap(QPixmap::fromImage(img).scaled(ui->mainshow_label->size()));
        //}

        //Partner* p = partner[_mytcpSocket->getlocalip()];
        //if (p) p->setpic(img);

        //qDebug()<< "format: " <<  videoImg.format() << "size: " << videoImg.size() << "byteSIze: "<< videoImg.sizeInBytes();
    }
    frame.unmap();
}




void meetingClient::on_openVideo_clicked(bool state) {
    if (_camera->status() == QCamera::ActiveStatus)
    {
        _camera->stop();
        ui->statusBar->showMessage("close camera");
        if (_camera->error() == QCamera::NoError)
        {
            //_imgThread->quit();
            //_imgThread->wait();
            //ui->openVedio->setText("��������ͷ");
            //emit PushText(CLOSE_CAMERA);
        }
        //closeImg(_mytcpSocket->getlocalip());
    }
    else
    {
        _camera->start(); //��������ͷ
        ui->statusBar->showMessage("open camera");
        if (_camera->error() == QCamera::NoError)
        {
            //_imgThread->start();
            //ui->openVedio->setText("�ر�����ͷ");
        }
    }
    
    
    
    //// ��������ͷ��ͼ�ؼ�
    //QCameraViewfinder viewfinder;
    //layout.addWidget(&viewfinder);

    //// ��ȡ����ͷ�豸
    //QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    //if (cameras.isEmpty()) {
    //    // û�п��õ�����ͷ�豸
    //    return 0;
    //}
    //QCameraInfo cameraInfo = cameras.first();

    //// ��������ͷ����������ͼ�ؼ�Ϊ��ʾģʽ
    //QCamera camera(cameraInfo);
    //camera.setViewfinder(&viewfinder);

    //// ��������ͷ
    //camera.start();

    return;
}

void meetingClient::on_btnTest_clicked() {
    QLabel* lab = new QLabel;
    QListWidgetItem* itemTime = new QListWidgetItem();
    
    ui->listVideo->addItem(itemTime);
    QSize size = QSize(ui->listVideo->width() - 100, ui->listVideo->width() - 100);
    itemTime->setSizeHint(size);
    lab->setFixedSize(size);
    lab->setPixmap(QPixmap(":/meetingClient/src/defaultAvatar.jpg").scaled(size));
    itemTime->setTextAlignment(Qt::AlignCenter);
    ui->listVideo->setItemWidget(itemTime, lab);
    itemTime->setTextAlignment(Qt::AlignCenter);





    //QTableWidgetItem* item = ui->tableVideo->takeItem(1, 1);
    //delete item;



    //ui->tableVideo->
}

