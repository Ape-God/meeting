#include "MyTcpSocket.h"
#include "Msg.h"

Q_DECLARE_METATYPE(MESGType)

MyTcpSocket::MyTcpSocket(Ui::meetingClientClass* ui, QObject* par) :_ui(ui), __ip(0), __port(0)
{
    qRegisterMetaType<QAbstractSocket::SocketError>();
    
    qRegisterMetaType<MESGType>("MESGType");
    connect(this, SIGNAL(sendMsg(uint8_t*)), this, SLOT(sendMsgSlot(uint8_t*)));

    connect(this, SIGNAL(readyRead()), this, SLOT(recvFromSocket()), Qt::UniqueConnection); //��������
    //�����׽��ִ���
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorDetect(QAbstractSocket::SocketError)), Qt::UniqueConnection);

    _sendDequeBuffer = new DequeBuffer(sizeof(MSG_C), 20);
    _recvDequeBuffer = new DequeBuffer(sizeof(MSG_S), 20);

    _sendbuf = (uchar*)malloc(4 * MB);
    _recvbuf = (uchar*)malloc(4 * MB);
    _ring_buf = new RingBuffer();
}

MyTcpSocket::~MyTcpSocket()
{
    delete _sendbuf;
    delete _recvbuf;
    delete _ring_buf;
}

void MyTcpSocket::init() {
    
}

void MyTcpSocket::connectSlot(QString ip, quint16 port)
{
    if (state() == QAbstractSocket::UnconnectedState) {
        // ����
        connectToHost(ip, port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
    }
    else if (state() == QAbstractSocket::ConnectedState) {
        // �Ͽ�����
        disconnectFromHost();
    }
}

void MyTcpSocket::closeSocket()
{
    if (this->isOpen()) {
        this->close();
    }
}

//QString MyTcpSocket::errorString()
//{
//    return this->errorString();
//}

void MyTcpSocket::errorDetect(QAbstractSocket::SocketError error)
{
    //qDebug() << "Sock error" << QThread::currentThreadId();
    //MESG* msg = (MESG*)malloc(sizeof(MESG));
    //if (msg == NULL)
    //{
    //    qDebug() << "errdect malloc error";
    //}
    //else
    //{
    //    memset(msg, 0, sizeof(MESG));
    //    if (error == QAbstractSocket::RemoteHostClosedError)
    //    {
    //        msg->msg_type = RemoteHostClosedError;
    //    }
    //    else
    //    {
    //        msg->msg_type = OtherNetError;
    //    }
    //    queue_recv.push_msg(msg);
    //}
}

void MyTcpSocket::disconnectFromHost() {
    //write
    //if (this->isRunning()) {
    //    QMutexLocker locker(&m_lock);
    //    m_isCanRun = false;
    //}

    //if (_sockThread->isRunning()) {
    //    _sockThread->quit();
    //    _sockThread->wait();
    //}

    //��� ���� ���У���ս��ܶ���
    /*queue_send.clear();
    queue_recv.clear();
    audio_recv.clear();*/
}

quint32 MyTcpSocket::getlocalip()
{
    if (this->isOpen())
    {
        return this->localAddress().toIPv4Address();
    }
    else
    {
        return -1;
    }
}

void MyTcpSocket::setIpAndPort(quint32 ip, quint16 port) {
    __ip = ip;
    __port = port;
    qDebug() << "this is " << QString::fromStdString(getIPAndPort(ip, port));
}

void MyTcpSocket::recvFromSocket() {
    qint64 availbytes = this->bytesAvailable();
    if (availbytes <= 0) {
        return;
    }
    qint64 size = this->read((char*)_recvbuf, availbytes);
    int len = _ring_buf->get_length();

    _ring_buf->push(_recvbuf, size);
    // ��������
    if (_ring_buf->get_length() >= MSG_S_HEAD_SIZE) {
        // ��ȡ��ǰ���MSG_S_HEAD_SIZE���ֽ�
        size = _ring_buf->top(_recvbuf, MSG_S_HEAD_SIZE);
        if (size < MSG_S_HEAD_SIZE) return;
        // ��ȡ��������
        MSG_S* msg_s = reinterpret_cast<MSG_S*>(_recvbuf);
        uint16_t len = u8u16(msg_s->len);
        if (_ring_buf->get_length() < len) {
            // ����û���룬ֱ�ӷ���
            return;
        }
        memset(msg_s, 0, sizeof(MSG_S));
        size = _ring_buf->pop(_recvbuf, len);
        // ��������
        if (msg_s->type == MESGType::CreatRoomResp || msg_s->type == MESGType::JoinRoomSuccess ||
            msg_s->type == MESGType::JoinRoomAlreadyJoin || msg_s->type == MESGType::JoinRoomNotFound ||
            msg_s->type == MESGType::ExitRoomSuccess || msg_s->type == MESGType::ExitRoomNotJoin) {
            emit recvRoomStatus(msg_s->type, u8u32(msg_s->ip), u8u16(msg_s->port), QString(msg_s->data));
        } else if(msg_s->type == MESGType::textMsgResp || msg_s->type == MESGType::audioMsgResp ||
                msg_s->type == MESGType::videoMsgResp) {
            // ȡ��һ������
            uint8_t* recvData = _recvDequeBuffer->popFrontFrom_dequeBuffEmpty();
            if (recvData == NULL) {
                qDebug() << "_recvDequeBuffer error ���Ժ�����";
                return;
            }
            memcpy(recvData, msg_s, len);
            if (len < sizeof(MSG_S)) recvData[len] = 0;
            emit recvMsg(msg_s->type, u8u32(msg_s->ip), u8u16(msg_s->port), recvData);
        }
    }
}

void MyTcpSocket::sendMsgSlot(uint8_t* sendData) {
    if (sendData == NULL) return;
    // δ�����򲻴���
    if (state() == SocketState::UnconnectedState) {
        _sendDequeBuffer->pushBackTo_dequeBuffEmpty(sendData);
        return;
    }
    
    MSG_C* msg_c = reinterpret_cast<MSG_C*>(sendData);
    // ���͵�������
    this->write((char*)sendData, u8u16(msg_c->len));
    this->waitForBytesWritten();
    _sendDequeBuffer->pushBackTo_dequeBuffEmpty(sendData);
}