#pragma once
#include <QString>
namespace XT
{
	// DPI模式
	enum DpiMode
	{
		DPI,			//不区分X/Y情况下使用该项
		DPI_X,			//水平方向DPI
		DPI_Y			//垂直方向DPI
	};

	// 操作模式
	enum OperateMode
	{
		NORMAL,			//正常浏览模式
		QUERY,			//要素属性查询模式
		SELECT			//海图抽选模式
	};

	// 海图抽选模式
	enum SelectionMode
	{
		NO_SELECT = 0,		//未设置
		P_SELECT,			//点抽选
		INDETAIL_SELECT,	//详细抽选
		SUMMARY_SELECT,		//概要抽选
		INREGION_SELECT,	//分区抽选
		INSET_SELECT,		//套系抽选
		A_SELECT			//面抽选
	};

	//符号缩放模式
	enum SymbolScaleMode
	{
		ABS_SCALE,//绝对比例尺
		REL_SCALE //相对比例尺
	};

	// 几何类型
	enum GeometryType
	{
		PointGeometry,		//点几何类型
		LineGeometry,		//线几何类型
		PolygonGeometry,	//多边形几何类型
		UnknownGeometry,	//未知几何类型
		NullGeometry		//空几何类型
	};

	// 查询模式
	enum QueryMode
	{
		NO_QUERY,			//空查询模式
		P_QUERY,			//点要素查询模式
		L_QUERY,			//线要素查询模式
		A_QUERY				//面要素查询模式
	};

	// 符号模式
	enum SymbolMode
	{
		SIMPLIFIED_CHART,	//简单符号海图模式
		PAPER_CHART,		//纸图符号海图模式
		CDC_CHART			//CDC符号海图模式
	};

	// 显示分类
	enum DisplayCategory
	{
		DISPLAY_BASE,		//基础显示
		DISPLAY_STANDARD,	//标准显示
		DISPLAY_OTHER,		//其他显示
		DISPLAY_CUSTOM		//自定义显示
	};

	// 投影类型
	enum ProjType
	{
		MCT,				// 墨卡托投影
		GAUSS,				// 高斯克吕格投影
		GNOMONIC,			// 日晷投影(Gnomonic projection)Gnomonic
	};

	// 导出模式
	enum ExportMode
	{
		CAPTURE,			//截屏导出
		IMG,				//图片导出
		PDF,				//PDF导出
		PRINT				//打印模式
	};
	// 渲染类型
	enum RenderType
	{
		RENDER_DISPLAY,	//显示渲染
		RENDER_PRINT	//打印渲染
	};

	// S52色标模式
	enum S52ColorMode
	{
		DAY = 0,			// 白天亮背景
		DAY_WHITEBACK,		// 白天白色背景
		DAY_BLACKBACK,		// 白天黑色背景
		DUSK,				// 黄昏
		NIGHT				// 夜
	};

	// 导出参数
	struct sExportParameters
	{
		XT::ExportMode exportMode = XT::CAPTURE;	//导出模式
		QString cellName = "";		//图名
		QString cellNo;				//图号
		double leftMargin = 0.0;	//左边距
		double rightMargin = 0.0;	//右边距
		double topMargin = 0.0;		//上边距
		double bottomMargin = 0.0;	//下边距
		int		dpi = -1;			//导出DPI精度
		QString exportPath;			//导出文件路径
		double	displayScale;		//打印比例尺

		int lbLD;					//左下经度：度
		int lbLM;					//左下经度：分
		float lbLS;					//左下经度：秒
		int lbBD;					//左下纬度：度
		int lbBM;					//左下纬度：分
		float lbBS;					//左下纬度：秒

		int rtLD;					//右上经度：度
		int rtLM;					//右上经度：分
		float rtLS;					//右上经度：秒
		int rtBD;					//右上纬度：度
		int rtBM;					//右上纬度：分
		float rtBS;					//右上纬度：秒

		QString projName;			//投影名称

		int baseLineBD;		//基准线纬度：度
		int baseLineBM;		//基准线纬度：分
		float baseLineBS;	//基准线纬度：秒
	};

	//导出图廓范围
	struct sExportCellBorderRange
	{
		QString lbBD;				//左下纬度：度
		QString lbBM;				//左下纬度：分
		QString lbBS;				//左下纬度：秒
		QString lbLD;				//左下经度：度
		QString lbLM;				//左下经度：分
		QString lbLS;				//左下经度：秒
		QString rtBD;				//右上纬度：度
		QString rtBM;				//右上纬度：分
		QString rtBS;				//右上纬度：秒
		QString rtLD;				//右上经度：度
		QString rtLM;				//右上经度：分
		QString rtLS;				//右上经度：秒
	};
}
