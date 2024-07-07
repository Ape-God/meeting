#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_meetingClient.h"

#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QVBoxLayout>
#include <QMessageBox>


#include "MyVideoSurface.h"
#include "MyTcpSocket.h"
#include "ChatMessage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class meetingClientClass; };
QT_END_NAMESPACE

class meetingClient : public QMainWindow
{
    Q_OBJECT

public:
    meetingClient(QWidget *parent = nullptr);
    ~meetingClient();

signals:
    void connectToServer(QString, quint16);
    void socketerror(QAbstractSocket::SocketError);

public slots:
    void on_btnTest_clicked();


    // 连接相关
    void on_btnConnect_clicked();
    void on_btnRoomCreate_clicked();
    void on_btnRoomJoin_clicked();
    void on_btnRoomExit_clicked();
    void recvRoomStatusSlot(MESGType, quint32, quint16, QString);

    // 文字消息相关
    void on_lineSendMsg_returnPressed();
    void recvMsgSlot(MESGType, quint32, quint16, void*);

    void on_openVideo_clicked(bool state);
    void cameraError(QCamera::Error);
    void cameraImageCapture(QVideoFrame frame);
private:
    void addTextMsg(QString msgText, QString name, ChatMessage::User_Type type);
    void dealMessage(ChatMessage* messageW, QListWidgetItem* item, QString text, QString time, QString ip, ChatMessage::User_Type type);
    void dealMessageTime(QString curMsgTime);
private:
    Ui::meetingClientClass *ui;
    MyTcpSocket* _myTcpSocket;
    QThread* _myTcpSocketThread;

    QCamera* _camera;
    QCameraImageCapture* _imagecapture; //截屏
    MyVideoSurface* _myVideoSurface;
    
};
