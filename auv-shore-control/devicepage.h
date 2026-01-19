// devicepage.h
#pragma once
#include <QString>

enum  DevicePage {
    // 父节点分类页面
    CORRESPOND,               // 通信（父节点）
    NAVIGATIONWITHLOCATION,   // 导航定位（父节点）
    DATAACQUISITION,          // 数据采集（父节点）
    PROMOTINGDEVICE,          // 推进装置（父节点）
    STEERINGDEVICE,           // 转向装置（父节点）
    ENERGYWITHENDURANCE,      // 能源与续航（父节点）
    BUOYANCYREGULATION,      // 浮力调节（父节点）
    EMERGENCYSYSTEM,          // 应急系统（父节点）
    OTHERSYSTEM,              // 其他系统（父节点，）

   // 通信子节点
   CORRESPOND_WIRED,         // 通信->有线
   CORRESPOND_RADIO,         // 通信->数传电台
   CORRESPOND_WIFI,          // 通信->WIFI
   CORRESPOND_SATELLITE,     // 通信->卫星通信（二级节点）
   CORRESPOND_BEIDOU_COM,    // 通信->卫星通信->北斗（三级节点）
   CORRESPOND_IRIDIUM,       // 通信->卫星通信->铱星（三级节点）
   CORRESPOND_WATERACOUSTIC, // 通信->水声通信

   // 导航定位子节点
   NAV_BEIDOU,               // 导航定位->北斗（二级节点）
   NAV_BEIDOU_POS,           // 导航定位->北斗->定位（三级节点）
   NAV_INERTIAL,             // 导航定位->惯导
   NAV_USBL,                 // 导航定位->USBL
   NAV_DVL,                  // 导航定位->DVL
   NAV_ALTIMETER,            // 导航定位->高度计
   NAV_DEPTHMETER,           // 导航定位->深度计

   // 数据采集子节点
   DATA_SIDESCAN,            // 数据采集->侧扫声呐
   DATA_FORWARDSCAN,         // 数据采集->前视声呐（第一个）
   DATA_DOWNWARDSCAN,        // 数据采集->下视声呐（第二个，可能是重复，按实际功能命名）
   DATA_MULTIBEAM,           // 数据采集->多波束测深仪
   DATA_CAMERA,              // 数据采集->摄像机
 

    // 推进装置子节点
    PROMOTE_TAIL,            // 推进装置->尾推
    PROMOTE_VERTICAL,         // 推进装置->垂推（二级节点）
    PROMOTE_VERTICAL_FRONT,   // 推进装置->垂推->前垂推（三级节点）
    PROMOTE_VERTICAL_BACK,    // 推进装置->垂推->后垂推（三级节点）
    PROMOTE_SIDE,             // 推进装置->侧推（二级节点）
    PROMOTE_SIDE_FRONT,       // 推进装置->侧推->前侧推（三级节点）
    PROMOTE_SIDE_BACK,        // 推进装置->侧推->后侧推（三级节点）
    PROMOTE_ANTENNA,          // 推进装置->天线折叠机构

    // 转向装置子节点
    STEER_RUDDER1,            // 转向装置->舵1
    STEER_RUDDER2,            // 转向装置->舵2
    STEER_RUDDER3,            // 转向装置->舵3
    STEER_RUDDER4,            // 转向装置->舵4

    // 能源与续航子节点
    ENERGY_POWERBAT1,          // 能源与续航->动力电池1
    ENERGY_POWERBAT2,         // 能源与续航->动力电池2
    ENERGY_METERBAT,          // 能源与续航->仪表电池

     // 浮力调节子节点
     BUOYANCY_REG,            // 浮力调节->浮调1

     // 应急系统子节点
     EMERGENCY_WATER,          // 应急系统->漏水传感器
     EMERGENCY_JETTISON        // 应急系统->抛载
};