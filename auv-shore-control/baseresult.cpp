#include "baseresult.h"

BaseResult::BaseResult(ModuleType sourceType, ResultStatus status)
	:m_sourceModule(sourceType),
	m_status(status),
	m_timestamp(QDateTime::currentDateTime())

{}


ModuleType BaseResult::sourceType() const
{
	return m_sourceModule;
}

ResultStatus BaseResult::status() const
{
	return m_status;
}

void BaseResult::setStatus(ResultStatus status)
{
	m_status = status;
}

QDateTime BaseResult::timeStamp() const
{
	return m_timestamp;
}

QString BaseResult::description() const
{
	return QString("»ù´¡½á¹û");
}
