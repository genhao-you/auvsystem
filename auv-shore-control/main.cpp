#include "auvshorecontrol.h"
#include <QtWidgets/QApplication>
#include "BDSchannel.h"
#include"communicationchannel.h"
#include"datamessage.h"
#include"channelconfig.h"
#include <cstdio>  

int main(int argc, char *argv[])
{
    // 重定向stderr到文件（路径可自定义，如"curl_log.txt"）
    freopen("D:/curl_ftp_log.txt", "w", stderr);
    // 1. 禁用Qt网络监控（环境变量方式，兼容标准Qt）
    qputenv("QT_NETWORKMONITOR_DISABLE", "1");
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // 启用高DPI缩放
    QApplication a(argc, argv);

    //注册枚举类型
    qRegisterMetaType<CommunicationChannel>("CommunicationChannel");
    qRegisterMetaType<DataMessage>("DataMessage");
    qRegisterMetaType<DataMessage>("DataMessage&");
    qRegisterMetaType<ChannelConfigPtr>("ChannelConfigPtr");
    qRegisterMetaType<Mission*>("Mission*");
    qRegisterMetaType<Plan*>("Plan*");
    qRegisterMetaType<Task*>("Task*");

   // qRegisterMetaType<const ChannelConfigPtr>("const ChannelConfigPtr");
    AuvShoreControl w;
    w.show();
 
    return a.exec();
}
