#pragma once
#include"selfcheckenum.h"
#include"commandparameter.h"
class BeidouATCommandMapper {
public:
    // 注意：CommandCode需匹配实际枚举定义（含命名空间）
    static QString getATCommand(CommandCode cmdCode, const std::shared_ptr<CommandParameter>& param) {
        QString atCmd;
        switch (cmdCode) {
            // 连接状态查询（无参）
        case CommandCode::BeidouComm_GetConnectStatus:
            atCmd = "AT+STACNT\r\n";
            break;

            // 信号质量查询（无参）
        case CommandCode::BeidouComm_GetSignalQuality:
            atCmd = "AT+CSQ\r\n";
            break;

            // 身份查询（无参）
        case CommandCode::BeidouComm_GetIdentity:
            atCmd = "AT+LNO\r\n";
            break;

            // 目标查询（无参）
        case CommandCode::BeidouComm_GetTarget:
            atCmd = "AT+PNO\r\n";
            break;

            // 目标设置（有参：抽象类指向派生类）
        case CommandCode::BeidouComm_SetTarget: {
            // 抽象类指针动态转换为派生类（核心：抽象类本身不能实例化，但可指向子类）
            auto cardParam = std::dynamic_pointer_cast<BeidouCardNoParam>(param);
            if (!cardParam) {
                qWarning() << "[ATMapper] SetTarget指令参数错误：空指针或非卡号参数";
                atCmd = ""; // 无效参数返回空指令
                break;
            }
            // 格式化7位卡号（补前导0）
            QString cardStr = QString("%1").arg(cardParam->cardNo(), 7, 10, QChar('0'));
            atCmd = QString("AT+PNO+%1\r\n").arg(cardStr);
            break;
        }

                                              // 白名单查询（无参）
        case CommandCode::BeidouComm_GetWhitelist:
            atCmd = "AT+LISTPD\r\n";
            break;

            // 白名单添加（有参）
        case CommandCode::BeidouComm_AddWhitelist: {
            auto cardParam = std::dynamic_pointer_cast<BeidouCardNoParam>(param);
            if (!cardParam) {
                qWarning() << "[ATMapper] AddWhitelist指令参数错误";
                atCmd = "";
                break;
            }
            QString cardStr = QString("%1").arg(cardParam->cardNo(), 7, 10, QChar('0'));
            atCmd = QString("AT+ADDPD %1\r\n").arg(cardStr);
            break;
        }

                                                 // 白名单删除（有参）
        case CommandCode::BeidouComm_DelWhitelist: {
            auto cardParam = std::dynamic_pointer_cast<BeidouCardNoParam>(param);
            if (!cardParam) {
                qWarning() << "[ATMapper] DelWhitelist指令参数错误";
                atCmd = "";
                break;
            }
            QString cardStr = QString("%1").arg(cardParam->cardNo(), 7, 10, QChar('0'));
            atCmd = QString("AT+DELPD %1\r\n").arg(cardStr);
            break;
        }

                                                 // 定位信息查询（无参）
        case CommandCode::BeidouComm_GetPosition:
            atCmd = "AT+GPS\r\n";
            break;

            // 恢复出厂设置（无参）
        case CommandCode::BeidouComm_FactoryReset:
            atCmd = "AT+DEF\r\n";
            break;

            // 重启设备（无参）
        case CommandCode::BeidouComm_Reboot:
            atCmd = "AT+RESET\r\n";
            break;

            // 通讯测试（无参）
        case CommandCode::BeidouComm_Test:
            atCmd = "AT\r\n";
            break;

        default:
            qWarning() << "[ATMapper] 未知指令码：" << static_cast<int>(cmdCode);
            atCmd = "";
            break;
        }
        return atCmd;
    }
};