
#include "Message.h"

Message::Message()
{
    this->type = MessageType::undefined;
}
Message::Message(MessageType type)
{
    this->sender = getpid();
    this->type = type;
}
Message::Message(MessageType type, vector<string> data) : Message(type)
{
    this->data = data;
}
Message::Message(string str)
{
    //Debug::Log("Before parsing: " + str);
    istringstream iss(str);
    vector<string> words
    {
        istream_iterator<string>{iss},
        istream_iterator<string>{}
    };
    sender = stol(words[0]);
    type = Message::TypeFromString(words[1]);
    int messageLength = stoi(words[2]);
    data = words;
    for(int i = 0; i < 3; i++)
    {
        data.erase(data.begin());
    }
    //Debug::Log("Parsed: " + ToString());
}
void Message::Send(mqd_t queue, string contactName)
{
    string str = this->ToString();
    if(contactName != "")
    {
        Debug::Log("Sending: \"" + str + "\" to " + contactName);
    }
    else
    {
        Debug::Log("Sending: \"" + str + "\"");
    }
    mq_send(queue, str.c_str(), str.length(), 0);
}
string Message::ToString()
{
    stringstream ss;
    ss << sender << " " << Message::TypeToString(type) << " " << data.size() << " ";
    for(vector<string>::iterator it = data.begin(); it != data.end(); ++it)
    {
        ss << *it << " ";
    }
    return ss.str();
}
Message Message::Receive(int maxMessageSize, mqd_t queue, string contactName, bool tryTillSuccess, int retryTimeBase_ms, int retryTimeRand_ms)
{
    Message msg = Message();
    char buffer[maxMessageSize];
    struct mq_attr attr;
    /*
    if(contactName != "")
    {
        Debug::Log("Trying to get message from " + contactName);
    }
    else
    {
        Debug::Log("Trying to get message");
    }
    */
    while(true)
    {
        mq_getattr(queue, &attr);
        memset(buffer, 0x00, sizeof(buffer));
        ssize_t bytes_read = mq_receive(queue, buffer, maxMessageSize, NULL);
        //cout << attr.mq_curmsgs << " " << bytes_read << " " << errno << " " << buffer << endl;
        if(bytes_read >= 0)
        {
            msg = Message(buffer);
            if(contactName != "")
            {
                Debug::Log("Got message \"" + msg.ToString() + "\"");
            }
            break;
        }
        else
        {
            if(tryTillSuccess)
            {
                Timer::Wait_msec(retryTimeBase_ms, retryTimeRand_ms);
            }
            else
            {
                break;
            }
        }
    }
    return msg;
}
string Message::TypeToString(MessageType type)
{
    switch(type)
    {
        case MessageType::lock:
        {
            return "lock";
            break;
        }
        case MessageType::hello:
        {
            return "hello";
            break;
        }
        case MessageType::requestContacts:
        {
            return "requestContacts";
            break;
        }
        case MessageType::sendContacts:
        {
            return "sendContacts";
            break;
        }
        case MessageType::giveMarker:
        {
            return "giveMarker";
            break;
        }
        case MessageType::finish:
        {
            return "finish";
            break;
        }
        case MessageType::sendSymbol:
        {
            return "sendSymbol";
            break;
        }
        default:
        {
            return "undefined";
            break;
        }
    }
}
MessageType Message::TypeFromString(string str)
{
    if(str == "lock")
    {
        return MessageType::lock;
    }
    else if(str == "hello")
    {
        return MessageType::hello;
    }
    else if(str == "requestContacts")
    {
        return MessageType::requestContacts;
    }
    else if(str == "sendContacts")
    {
        return MessageType::sendContacts;
    }
    else if(str == "giveMarker")
    {
        return MessageType::giveMarker;
    }
    else if(str == "finish")
    {
        return MessageType::finish;
    }
    else if(str == "sendSymbol")
    {
        return MessageType::sendSymbol;
    }
    return MessageType::undefined;
}
