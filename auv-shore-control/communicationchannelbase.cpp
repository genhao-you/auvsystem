#include "communicationchannelbase.h"

CommunicationChannelBase::CommunicationChannelBase(CommunicationChannel type, QObject* parent)
    : QObject(parent), m_type(type) {}

CommunicationChannelBase::~CommunicationChannelBase() {
   // stop();
}

CommunicationChannel CommunicationChannelBase::type() const {
    return m_type;
}