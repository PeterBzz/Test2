
#include "Contact.h"

Contact::Contact()
{
    //
}
Contact::Contact(long pid, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize)
{
    this->personalQueueNamePrefix = personalQueueNamePrefix;
    this->messageMaxCount = messageMaxCount;
    this->messageMaxSize = messageMaxSize;
    this->pid = pid;
    this->epoch = boost::posix_time::microsec_clock::universal_time();
    this->queueName = personalQueueNamePrefix + to_string(pid);
    struct mq_attr attr = mq_attr
    {
        .mq_flags = 0,
        .mq_maxmsg = messageMaxCount,
        .mq_msgsize = messageMaxSize,
        .mq_curmsgs = 0
    };
    if(pid == getpid())
    {
        this->queue = mq_open(this->queueName.c_str(), O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);
        Debug::Log("Personal queue opened as RDONLY for " + to_string(pid));
    }
    else
    {
        this->queue = mq_open(this->queueName.c_str(), O_CREAT | O_WRONLY | O_NONBLOCK, 0644, &attr);
        Debug::Log("Personal queue opened as WRONLY for " + to_string(pid));
    }
}
Contact::Contact(long pid, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize, string symbols) : Contact(pid, personalQueueNamePrefix, messageMaxCount, messageMaxSize)
{
    this->symbols = symbols;
}
Contact::Contact(long pid, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize, string symbols, boost::posix_time::ptime epoch) : Contact(pid, personalQueueNamePrefix, messageMaxCount, messageMaxSize, symbols)
{
    this->epoch = epoch;
}
Contact::~Contact()
{
    //Debug::Log("Contact destructor is called for " + to_string(pid));
    //mq_close(queue);
	//mq_unlink(queueName.c_str());
}
void Contact::RequestContacts()
{
    Message(MessageType::requestContacts).Send(queue, to_string(pid));
}
vector<Contact> Contact::ReceiveContacts(int personalMessageMaxSize, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize, int retryTimeBase_ms, int retryTimeRand_ms)
{
    Message msg = Message::Receive(personalMessageMaxSize,
        queue,
        to_string(pid),
        true,
        retryTimeBase_ms,
        retryTimeRand_ms);
    return Contact::ContactsFromMessage(msg, personalQueueNamePrefix, messageMaxCount, messageMaxSize);
}
void Contact::SendContacts(vector<Contact> contacts)
{
    Contact::ContactsToMessage(contacts).Send(queue, to_string(pid));
}
void Contact::SendFinish()
{
    Message(MessageType::finish).Send(queue, to_string(pid));
}
void Contact::GiveMarker()
{
    Message(MessageType::giveMarker).Send(queue, to_string(pid));
}
vector<Contact> Contact::ContactsFromMessage(Message msg, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize)
{
    vector<Contact> contacts;
    for(vector<string>::iterator it = msg.data.begin(); it != msg.data.end(); it += 4)
    {
        Contact c = Contact(stol(*it),
            personalQueueNamePrefix,
            messageMaxCount,
            messageMaxSize,
            (it + 1)->substr(1, (it + 1)->length() - 2),
            boost::posix_time::time_from_string(*(it + 2) + " " + *(it + 3)));
        contacts.push_back(c);
    }
    return contacts;
}
Message Contact::ContactsToMessage(vector<Contact> contacts)
{
    vector<string> v;
    for(vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); ++it)
    {
        v.push_back(to_string((*it).pid));
        v.push_back("[" + (*it).symbols + "]");
        v.push_back(to_simple_string((*it).epoch));
    }
    return Message(MessageType::sendContacts, v);
}
void Contact::Update(string symbols)
{
    this->Update(symbols, boost::posix_time::microsec_clock::universal_time());
}
void Contact::Update(string symbols, boost::posix_time::ptime epoch)
{
    this->symbols = symbols;
    this->epoch = epoch;
}
void Contact::SendSymbol(char c)
{
    vector<string> v;
    v.push_back(string(1, c));
    Message(MessageType::sendSymbol, v).Send(queue, to_string(pid));
}
