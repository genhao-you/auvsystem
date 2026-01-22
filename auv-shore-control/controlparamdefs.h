#pragma once
#include <cstdint>
#include <QByteArray>
#include <QTimer>
#include <QDateTime>

// ========== 控制参数模块专属枚举（仅本模块使用，不对外暴露） ==========
/**
 * @brief 控制类型（仅控制参数模块用）
 */
enum class ControlType : uint8_t {
    DepthControl = 0xAA,    // 深度控制
    HeightControl = 0xBB,   // 高度控制
    HeadingControl = 0xCC,  // 航向控制
    UnknownControl = 0x00   // 未知控制
};


// ========== 控制参数模块专属常量（参数相关） ==========
constexpr int16_t CONTROL_PARAM_SCALE = 100;  // 参数缩放系数
constexpr int CMD_TIMEOUT_MS = 5000;           // 命令超时时间
constexpr int CLEANUP_INTERVAL_MS = 30000;     // 清理间隔

// ========== 深度控制参数（严格匹配通信协议顺序/变量名/初始值） ==========
struct DepthControlParams {
    // 协议字段（按顺序）
    double deep_pid_kp = -3.0;                // 深度控制外环kp
    double deep_pid_ki = 0.0;                 // 深度控制外环ki
    double deep_pid_kd = 0.0;                 // 深度控制外环kd
    double deep_sita_pid_kp = -7.2;           // 深度控制内环kp
    double deep_sita_pid_ki = -3.4;           // 深度控制内环ki
    double deep_sita_pid_kd = 0.0;            // 深度控制内环kd
    double sita_xf_max = 10.0;                // 深度最大俯仰角限幅
    double sita_xf_min = -10.0;               // 深度最小俯仰角限幅
    double dleta_xf_max = 15.0;               // 深度最大垂直舵角限幅
    double dleta_xf_min = -15.0;              // 深度最小垂直舵角限幅
    double dynamic_sp_dleta_number = 1.0;     // 深度动态变化阈值
    double dleta_sp_angle = 0.0;              // 深度小信号死区值
    double deep_dleta_rate_max = 15.0;        // 深度舵速最大限幅
    double deep_dleta_rate_min = 15.0;        // 深度舵速最小限幅（协议初始值15）
    double deep_lb_number = 0.2;              // 实际深度滤波系数
    double deep_sita_lb_number = 0.2;         // 实际俯仰角滤波系数
    double dleta_sp_rate_lb_number = 0.2;     // 舵速限幅后滤波系数
    double deep_control_period = 0.1;              // 控制周期（单位：秒，初始0.1）

    /**
     * 序列化：按协议顺序转为int16字节流（大端序）
     * 总字节数：18个参数 × 2字节 = 36字节
     */
    QByteArray toBytes() const {
        QByteArray bytes;
        auto addParam = [&](double val) {
            int16_t scaled = static_cast<int16_t>(val * CONTROL_PARAM_SCALE);
            bytes.append(static_cast<char>((scaled >> 8) & 0xFF)); // 高字节
            bytes.append(static_cast<char>(scaled & 0xFF));        // 低字节
        };

        // 严格按协议字段顺序序列化（共18个参数）
        addParam(deep_pid_kp);
        addParam(deep_pid_ki);
        addParam(deep_pid_kd);
        addParam(deep_sita_pid_kp);
        addParam(deep_sita_pid_ki);
        addParam(deep_sita_pid_kd);
        addParam(sita_xf_max);
        addParam(sita_xf_min);
        addParam(dleta_xf_max);
        addParam(dleta_xf_min);
        addParam(dynamic_sp_dleta_number);
        addParam(dleta_sp_angle);
        addParam(deep_dleta_rate_max);
        addParam(deep_dleta_rate_min);
        addParam(deep_lb_number);
        addParam(deep_sita_lb_number);
        addParam(dleta_sp_rate_lb_number);
        addParam(deep_control_period);

        return bytes;
    }

    /**
     * 反序列化：从int16字节流解析为参数（严格按协议顺序）
     */
    static DepthControlParams fromBytes(const QByteArray& bytes) {
        DepthControlParams params;
        if (bytes.size() < 36) return params; // 修正：18个参数×2字节=36字节

        int idx = 0;
        auto getParam = [&]() -> double {
            int16_t scaled = (static_cast<uint8_t>(bytes[idx]) << 8) | static_cast<uint8_t>(bytes[idx + 1]);
            idx += 2;
            return static_cast<double>(scaled) / CONTROL_PARAM_SCALE;
        };

        // 严格按协议字段顺序解析
        params.deep_pid_kp = getParam();
        params.deep_pid_ki = getParam();
        params.deep_pid_kd = getParam();
        params.deep_sita_pid_kp = getParam();
        params.deep_sita_pid_ki = getParam();
        params.deep_sita_pid_kd = getParam();
        params.sita_xf_max = getParam();
        params.sita_xf_min = getParam();
        params.dleta_xf_max = getParam();
        params.dleta_xf_min = getParam();
        params.dynamic_sp_dleta_number = getParam();
        params.dleta_sp_angle = getParam();
        params.deep_dleta_rate_max = getParam();
        params.deep_dleta_rate_min = getParam();
        params.deep_lb_number = getParam();
        params.deep_sita_lb_number = getParam();
        params.dleta_sp_rate_lb_number = getParam();
        params.deep_control_period = getParam();

        return params;
    }
};

// ========== 高度控制参数（严格匹配通信协议） ==========
struct HeightControlParams {
    // 协议字段（按顺序）
    double high_pid_kp = -3.0;                // 高度控制外环kp
    double high_pid_ki = 0.0;                 // 高度控制外环ki（协议初始值空，设0）
    double high_pid_kd = 0.0;                 // 高度控制外环kd（协议初始值空，设0）
    double high_sita_kp = -7.2;               // 高度控制内环kp
    double high_sita_ki = 0.0;                // 高度控制内环ki
    double high_sita_kd = -3.4;               // 高度控制内环kd
    double high_sita_max = 10.0;              // 高度最大俯仰角限幅
    double high_sita_min = -10.0;             // 高度最小俯仰角限幅
    double high_dleta_max = 15.0;             // 高度最大垂直舵角限幅
    double high_dleta_min = -15.0;            // 高度最小垂直舵角限幅
    double dynamic_sp_dleta_number_h = 1.0;   // 高度动态变化阈值
    double dleta_sp_angle_h = 0.0;            // 高度小信号死区值
    double dleta_sp_rate_max_h = 15.0;        // 高度舵速最大限幅
    double dleta_sp_rate_min_h = -15.0;       // 高度舵速最小限幅
    double act_h_lb_number = 0.2;             // 实际高度滤波系数
    double act_h_sita_lb_number = 0.2;        // 实际俯仰角滤波系数
    double high_dleta_rate_lb_number = 0.2;   // 舵速限幅后滤波系数
    double high_control_period = 0.1;              // 控制周期（秒，初始0.1）

    /**
     * 序列化：17个参数 × 2字节 = 34字节
     */
    QByteArray toBytes() const {
        QByteArray bytes;
        auto addParam = [&](double val) {
            int16_t scaled = static_cast<int16_t>(val * CONTROL_PARAM_SCALE);
            bytes.append(static_cast<char>((scaled >> 8) & 0xFF));
            bytes.append(static_cast<char>(scaled & 0xFF));
        };

        // 严格按协议顺序
        addParam(high_pid_kp);
        addParam(high_pid_ki);
        addParam(high_pid_kd);
        addParam(high_sita_kp);
        addParam(high_sita_ki);
        addParam(high_sita_kd);
        addParam(high_sita_max);
        addParam(high_sita_min);
        addParam(high_dleta_max);
        addParam(high_dleta_min);
        addParam(dynamic_sp_dleta_number_h);
        addParam(dleta_sp_angle_h);
        addParam(dleta_sp_rate_max_h);
        addParam(dleta_sp_rate_min_h);
        addParam(act_h_lb_number);
        addParam(act_h_sita_lb_number);
        addParam(high_dleta_rate_lb_number);
        addParam(high_control_period);

        return bytes;
    }

    /**
     * 反序列化
     */
    static HeightControlParams fromBytes(const QByteArray& bytes) {
        HeightControlParams params;
        if (bytes.size() < 34) return params; // 17×2=34字节

        int idx = 0;
        auto getParam = [&]() -> double {
            int16_t scaled = (static_cast<uint8_t>(bytes[idx]) << 8) | static_cast<uint8_t>(bytes[idx + 1]);
            idx += 2;
            return static_cast<double>(scaled) / CONTROL_PARAM_SCALE;
        };

        // 严格按协议顺序解析
        params.high_pid_kp = getParam();
        params.high_pid_ki = getParam();
        params.high_pid_kd = getParam();
        params.high_sita_kp = getParam();
        params.high_sita_ki = getParam();
        params.high_sita_kd = getParam();
        params.high_sita_max = getParam();
        params.high_sita_min = getParam();
        params.high_dleta_max = getParam();
        params.high_dleta_min = getParam();
        params.dynamic_sp_dleta_number_h = getParam();
        params.dleta_sp_angle_h = getParam();
        params.dleta_sp_rate_max_h = getParam();
        params.dleta_sp_rate_min_h = getParam();
        params.act_h_lb_number = getParam();
        params.act_h_sita_lb_number = getParam();
        params.high_dleta_rate_lb_number = getParam();
        params.high_control_period = getParam();

        return params;
    }
};

// ========== 航向控制参数（严格匹配通信协议） ==========
struct HeadingControlParams {
    // 协议字段（按顺序）
    double hx_pid_kp = 0.2;                   // 航向控制kp
    double hx_pid_ki = 0.0;                   // 航向控制ki
    double hx_pid_kd = 0.1;                   // 航向控制kd
    double cz_dleta_max = 15.0;               // 航向最大垂直舵角（°）
    double cz_dleta_min = -15.0;              // 航向最小垂直舵角（°）
    double dynamic_cz_dleta_number = 1.0;     // 航向动态变化阈值（°）
    double dleta_cz_angle = 0.0;              // 航向小信号死区值（°）
    double dleta_cz_rate_max = 15.0;          // 航向舵速最大限幅（°/s）
    double dleta_cz_rate_min = -15.0;         // 航向舵速最小限幅
    double circle_r = 24.0;                   // LOS索引半径
    double psi_lb_number = 0.2;               // 实际航向角滤波系数
    double dleta_cz_lb_number = 0.2;          // 垂直舵角滤波系数
    double hx_control_period = 0.1;              // 控制周期（秒，初始0.1）

    /**
     * 序列化：13个参数 × 2字节 = 26字节
     */
    QByteArray toBytes() const {
        QByteArray bytes;
        auto addParam = [&](double val) {
            int16_t scaled = static_cast<int16_t>(val * CONTROL_PARAM_SCALE);
            bytes.append(static_cast<char>((scaled >> 8) & 0xFF));
            bytes.append(static_cast<char>(scaled & 0xFF));
        };

        // 严格按协议顺序
        addParam(hx_pid_kp);
        addParam(hx_pid_ki);
        addParam(hx_pid_kd);
        addParam(cz_dleta_max);
        addParam(cz_dleta_min);
        addParam(dynamic_cz_dleta_number);
        addParam(dleta_cz_angle);
        addParam(dleta_cz_rate_max);
        addParam(dleta_cz_rate_min);
        addParam(circle_r);
        addParam(psi_lb_number);
        addParam(dleta_cz_lb_number);
        addParam(hx_control_period);

        return bytes;
    }

    /**
     * 反序列化
     */
    static HeadingControlParams fromBytes(const QByteArray& bytes) {
        HeadingControlParams params;
        if (bytes.size() < 26) return params; // 13×2=26字节

        int idx = 0;
        auto getParam = [&]() -> double {
            int16_t scaled = (static_cast<uint8_t>(bytes[idx]) << 8) | static_cast<uint8_t>(bytes[idx + 1]);
            idx += 2;
            return static_cast<double>(scaled) / CONTROL_PARAM_SCALE;
        };

        // 严格按协议顺序解析
        params.hx_pid_kp = getParam();
        params.hx_pid_ki = getParam();
        params.hx_pid_kd = getParam();
        params.cz_dleta_max = getParam();
        params.cz_dleta_min = getParam();
        params.dynamic_cz_dleta_number = getParam();
        params.dleta_cz_angle = getParam();
        params.dleta_cz_rate_max = getParam();
        params.dleta_cz_rate_min = getParam();
        params.circle_r = getParam();
        params.psi_lb_number = getParam();
        params.dleta_cz_lb_number = getParam();
        params.hx_control_period = getParam();

        return params;
    }
};

// ========== 控制参数组合体（数据段=控制类型+具体参数） ==========
struct ControlParams {
    ControlType type = ControlType::DepthControl;
    union {
        DepthControlParams depth;
        HeightControlParams height;
        HeadingControlParams heading;
    };

    // 显式默认构造函数（解决E1790错误）
    ControlParams() {
        // 显式初始化默认的深度参数（避免union成员未初始化）
        new (&depth) DepthControlParams();
    }

    // 带类型的构造函数（增强易用性）
    explicit ControlParams(ControlType ctrlType) : type(ctrlType) {
        // 根据类型初始化对应union成员
        switch (ctrlType) {
        case ControlType::DepthControl:
            new (&depth) DepthControlParams();
            break;
        case ControlType::HeightControl:
            new (&height) HeightControlParams();
            break;
        case ControlType::HeadingControl:
            new (&heading) HeadingControlParams();
            break;
        default:
            new (&depth) DepthControlParams();
            break;
        }
    }

    // 析构函数（避免内存泄漏）
    ~ControlParams() {
        // 手动析构union中活跃的成员
        switch (type) {
        case ControlType::DepthControl:
            depth.~DepthControlParams();
            break;
        case ControlType::HeightControl:
            height.~HeightControlParams();
            break;
        case ControlType::HeadingControl:
            heading.~HeadingControlParams();
            break;
        default:
            depth.~DepthControlParams();
            break;
        }
    }

    /**
     * 组装通信数据段：控制类型(1字节) + 具体参数字节流
     */
    QByteArray toDataSegment() const {
        QByteArray data;
        // 1. 控制类型（数据段第一个字节）
        data.append(static_cast<char>(static_cast<uint8_t>(type)));
        // 2. 具体参数（严格按协议序列化）
        switch (type) {
        case ControlType::DepthControl:
            data.append(depth.toBytes());
            break;
        case ControlType::HeightControl:
            data.append(height.toBytes());
            break;
        case ControlType::HeadingControl:
            data.append(heading.toBytes());
            break;
        default:
            break;
        }
        return data;
    }
};
