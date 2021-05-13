#include "rock_protocol.h"
#include "log.h"

namespace seaice{

static Logger::ptr logger = SEAICE_LOGGER("system");

RockBody::RockBody() {}
RockBody::~RockBody() {}
	

bool RockBody::serilizeToByteArray(ByteArray::ptr ba) {
	ba->writeStringVint(m_body);
	return true;
}

bool parseFromByteArray(ByteArray::ptr ba) {
	m_body = ba->readStringVint();
	return true;
}

std::string RockRequest::toString() const {
	std::stringstream ss;
	ss  << "[RockRequest sn = " << m_sn
		<< " cmd = " << m_cmd
		<< " body.length = " << m_body.size()
		<< "]"
	return ss.str();
}

const std::string& RockRequest::getName() const {
	static const std::string& s_name = "RockRequest";
	return s_name;
}

int32_t RockRequest::getType() const {
	return Message::REQUEST;
}

bool RockRequest::serializeToByteArray(ByteArray::ptr ba)  {
	try{
		bool v = true;
		v &= Request::serializeToByteArray(ba);
		v &= RockBody::serializeToByteArray(ba);
		return v;
	} catch(...) {
		SEAICE_LOG_ERROR(logger) << "RockRequest serializeToByteArray error";
	}
	return false;
}

bool RockRequest::parseFromByteArray(ByteArray::ptr ba) {
	try{
		bool v = true;
		v &= Request::parseFromByteArray(ba);
		v &= RockBody::parseFromByteArray(ba);
		return v;
	} catch(...) {
		SEAICE_LOG_ERROR(logger) << "RockRequest parseFromByteArray error";
	}
	return false;
}

std::string RockResponse::toString() const {
	std::stringstream ss;
	ss  << "[RockResponse sn = " << m_sn
		<< " cmd = " << m_cmd
		<< " result = " << m_result
		<< " result_msg = " << m_resultStr
		<< " body.length = " << m_body.size()
		<< "]";
	return ss.str();
}

const std::string& RockResponse::getName() const {
	static const std::string& s_name = "RockResponse";
	return s_name;
}

int32_t RockResponse::getType() const {
	return Message::RESPONSE;
}


bool RockResponse::serializeToByteArray(ByteArray::ptr ba)  {
	try{
		bool v = true;
		v &= Response::serializeToByteArray(ba);
		v &= RockBody::serializeToByteArray(ba);
		return v;
	} catch(...) {
		SEAICE_LOG_ERROR(logger) << "RockResponse parseFromByteArray error";
	}
	return false;
}

bool RockResponse::parseFromByteArray(ByteArray::ptr ba) {
	try{
		bool v = true;
		v &= Response::parseFromByteArray(ba);
		v &= RockBody::parseFromByteArray(ba);
		return v;
	} catch(...) {
		SEAICE_LOG_ERROR(logger) << "RockResponse parseFromByteArray error";
	}
	return false;
}

class RockNotify : public Notify, public RockBody
{
public:
	typedef std::shared_ptr<RockNotify> ptr;

	virtual std::string toString() const override;
	virtual const std::string& getName() const override;
	virtual int32_t getType() const override;

	virtual bool serializeToByteArray(ByteArray::ptr ba) override;	
	virtual bool parseFromByteArray(ByteArray::ptr ba) override;
};

struct RockMsgHeader {
	RockMsgHeader();
	uint8_t magic[2];
	uint8_t version;
	uint8_t flag;
	uint32_t length;
};

class RockMessageDecoder : public MessageDecoder
{
public:
	typedef std::shared_ptr<RockMessageDecoder> ptr;

	virtual Message::ptr parseFrom(Stream::ptr stream) override;
	virtual int32_t serializeTo(Stream::ptr stream, Message::ptr msg) override;
};

}