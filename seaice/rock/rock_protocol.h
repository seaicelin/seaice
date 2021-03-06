#ifndef __SEAICE_ROCK_PROTOCOL_H__
#define __SEAICE_ROCK_PROTOCOL_H__

#include "../protocol.h"

namespace seaice {

class RockBody {
public:
    typedef std::shared_ptr<RockBody> ptr;
    RockBody();
    virtual ~RockBody();
    
    void setBody(const std::string& v) {m_body = v;}
    const std::string& getBody() const {return m_body;}

    virtual bool serializeToByteArray(ByteArray::ptr ba);
    virtual bool parseFromByteArray(ByteArray::ptr ba);    

    template<typename T>
    std::shared_ptr<T> getAsPB() const {
        try {
            std::shared_ptr<T> data(new T);
            if(data->ParseFromString(m_body)) {
                return data;
            }
        } catch(...) {
        }
        return nullptr;
    }

    template<typename T>
    bool setAsPB(const T& v) {
        try {
            v.SerializeToString(&m_body);
        } catch(...) {
        }
        return false;
    }
protected:
    std::string m_body;
};

class RockRequest : public Request, public RockBody
{
public:
    typedef std::shared_ptr<RockRequest> ptr;

    ~RockRequest() {}
    virtual std::string toString() const override;
    virtual const std::string& getName() const override;
    virtual MessageType getType() const override;

    virtual bool serializeToByteArray(ByteArray::ptr ba) override;    
    virtual bool parseFromByteArray(ByteArray::ptr ba) override;
};

class RockResponse : public Response, public RockBody {
public:
    typedef std::shared_ptr<RockResponse> ptr;

    ~RockResponse() {}
    virtual std::string toString() const override;
    virtual const std::string& getName() const override;
    virtual MessageType getType() const override;

    virtual bool serializeToByteArray(ByteArray::ptr ba) override;    
    virtual bool parseFromByteArray(ByteArray::ptr ba) override;
};

class RockNotify : public Notify, public RockBody
{
public:
    typedef std::shared_ptr<RockNotify> ptr;

    ~RockNotify() {}
    virtual std::string toString() const override;
    virtual const std::string& getName() const override;
    virtual MessageType getType() const override;

    virtual bool serializeToByteArray(ByteArray::ptr ba) override;    
    virtual bool parseFromByteArray(ByteArray::ptr ba) override;
};

struct RockMsgHeader {
    RockMsgHeader();
    uint8_t magic[2] = {0xab, 0xcd};
    uint8_t version = 0x01;
    uint8_t flag = 0;
    uint32_t length;

    std::string toString();
};

class RockMessageDecoder : public MessageDecoder
{
public:
    typedef std::shared_ptr<RockMessageDecoder> ptr;

    ~RockMessageDecoder() {}
    virtual Message::ptr parseFrom(Stream::ptr stream) override;
    virtual int32_t serializeTo(Stream::ptr stream, Message::ptr msg) override;
};




}
#endif