#pragma once
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#include <unordered_set>

#include "ui_meetingClient.h"

#include <QThread>
#include <QTcpSocket>
#include <QMutex>
#include <QHostAddress>
#include <QMessageBox>

#include "Msg.h"
#include "DequeBuffer.h"
#include "RingBuffer.h"

#ifndef MB
#define MB (1024 * 1024)
#endif

typedef unsigned char uchar;


class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
private:
    Ui::meetingClientClass* _ui;
    // uchar* _buf;
    RingBuffer* _ring_buf;
    // QTcpSocket* _socktcp;
    QThread* _sockThread;
    uchar* _sendbuf;
    uchar* _recvbuf;

    quint32 __ip;
    quint16 __port;

public:
    DequeBuffer* _sendDequeBuffer;
    DequeBuffer* _recvDequeBuffer;
    std::unordered_set<std::string> _partners;

public:
    MyTcpSocket(Ui::meetingClientClass* ui, QObject* par = NULL);
    ~MyTcpSocket();

    void init();
    // QString errorString();
    void disconnectFromHost();
    quint32 getlocalip();

    void setIpAndPort(quint32 ip, quint16 port);

    quint32 getIPU32() {
        return __ip;
    }

    std::string getIPStr(quint32 ipU32) {
        QHostAddress ipAddress(ipU32);
        return ipAddress.toString().toStdString();
    }

    std::string getIPAndPort(quint32 ip, quint16 port) {
        QHostAddress ipAddress(ip);
        return ipAddress.toString().toStdString() + ":" + QString::number(port).toStdString();
    }

    std::string getLocalIPAndPort() {
        QHostAddress ipAddress(__ip);
        return ipAddress.toString().toStdString() + ":" + QString::number(__port).toStdString();
    }

    quint32 getPort() {
        return __port;
    }

    bool isBelongMe(quint32 ip, quint16 port){
        if (__ip == 0 || __port == 0) return true;
        return ip == __ip && port == __port;
    }

signals:
    void sendMsg(uint8_t* sendData);
    // 
    void recvMsg(MESGType, quint32, quint16, void*);
    void recvRoomStatus(MESGType, quint32, quint16, QString roomId = "");

    void socketerror(QAbstractSocket::SocketError);
    void sendTextOver();

    
    void creatRoom(QString);
    void joinRoom(QString);
    void exitRoom(QString);

public slots:
    void connectSlot(QString, quint16);
    void closeSocket();
    void sendMsgSlot(uint8_t* sendData);
    void recvFromSocket();
    void errorDetect(QAbstractSocket::SocketError error);


    
    


    // void run() override;
    // qint64 readn(char*, quint64, int);
    
    
    // quint64 hasrecvive;

    //QMutex m_lock;
    //volatile bool m_isCanRun;
//private slots:
    // void sendData(MESG*);
    



    // void on_tcp_connected();




    
    // void stopImmediately();
    


    

};