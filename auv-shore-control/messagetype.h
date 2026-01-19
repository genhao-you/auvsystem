#pragma once

enum class MessageType
{
	Command,//控制命令
	Status,//狀態更新
	Event,//事件通知
	DataTransfer//数据传输
};

