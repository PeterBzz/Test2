
#include "App.h"

App::App()
{
    this->self = Contact(getpid(), personalQueueNamePrefix, personalMessageMaxCount, personalMessageMaxSize);
    this->personalQueueName = this->personalQueueNamePrefix + to_string(this->self.pid);
    struct mq_attr lockAttributes = mq_attr
    {
        .mq_flags = 0,
        .mq_maxmsg = lockMessageMaxCount,
        .mq_msgsize = lockMessageMaxSize,
        .mq_curmsgs = 0
    };
    this->lockQueue = mq_open(lockQueueName.c_str(), O_CREAT | O_RDWR | O_NONBLOCK, 0644, &lockAttributes);
    struct mq_attr globalAttributes = mq_attr
    {
        .mq_flags = 0,
        .mq_maxmsg = globalMessageMaxCount,
        .mq_msgsize = globalMessageMaxSize,
        .mq_curmsgs = 0
    };
    this->globalQueue = mq_open(globalQueueName.c_str(), O_CREAT | O_RDWR | O_NONBLOCK, 0644, &globalAttributes);
    struct mq_attr personalAttributes = mq_attr
    {
        .mq_flags = 0,
        .mq_maxmsg = personalMessageMaxCount,
        .mq_msgsize = personalMessageMaxSize,
        .mq_curmsgs = 0
    };
    this->personalQueue = mq_open(personalQueueName.c_str(), O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &personalAttributes);
}
App::~App()
{
    FreeLock();
    mq_close(lockQueue);
    mq_unlink(lockQueueName.c_str());
    mq_close(globalQueue);
    mq_unlink(globalQueueName.c_str());
    mq_close(personalQueue);
    mq_unlink(personalQueueName.c_str());
}
App& App::Singletone()
{
    static App s;
    return s;
}
void App::GetLock()
{
    if(hasLock)
    {
        return;
    }
    struct mq_attr attr;
    while(true)
    {
        Debug::Log("Waiting for free lock queue");
        mq_getattr(lockQueue, &attr);
        if(attr.mq_curmsgs > 0)
        {
            Timer::Wait_msec(retryTimeBase_ms, retryTimeRand_ms);
        }
        else
        {
            break;
        }
    }
    char buffer[lockMessageMaxSize];
    while(true)
    {
        Debug::Log("Trying to get lock");
        Message(MessageType::lock).Send(lockQueue, "lockQueue");
        mq_getattr(lockQueue, &attr);
        if(attr.mq_curmsgs != 1)
        {
            Debug::Log("Conflict");
            ssize_t bytes_read;
            while(bytes_read >= 0)
            {
                memset(buffer, 0x00, sizeof(buffer));
                bytes_read = mq_receive(lockQueue, buffer, lockMessageMaxSize, NULL);
            }
            Timer::Wait_msec(retryTimeBase_ms, retryTimeRand_ms);
        }
        else
        {
            break;
        }
    }
    Debug::Log("Got a lock");
    hasLock = true;
}
void App::FreeLock()
{
    
    if(Message::Receive(lockMessageMaxSize, lockQueue, "lockQueue", false, 0, 0).type == MessageType::lock)
    {
        Debug::Log("Freed a lock");
        hasLock = false;
    }
}
void App::Start()
{
    GetLock();
    struct mq_attr attr;
    mq_getattr(globalQueue, &attr);
    if(attr.mq_curmsgs > 0)
    {
        ConnectWithPrevious();
    }
    else
    {
        hasMarker = true;
    }
    Message(MessageType::hello).Send(globalQueue, "globalQueue");
    FreeLock();
    Listen();
}
void App::ParseArgs(int argc, char* argv[])
{
    this->self.symbols = App::RemoveCopiesInSelf(argv[1]);
}
bool App::MergeContacts(Contact contact)
{
    bool foundCopy = false;
    bool updated = false;
    for(vector<Contact>::iterator it = this->contacts.begin(); it != this->contacts.end(); ++it)
    {
        if(it->pid == contact.pid)
        {
            if(it->epoch < contact.epoch)
            {
                *it = contact;
                updated = true;
            }
            foundCopy = true;
            break;
        }
    }
    if(!foundCopy)
    {
        this->contacts.push_back(contact);
        updated = true;
    }
    return updated;
}
bool App::MergeContacts(vector<Contact> contacts)
{
    bool updated = false;
    for(vector<Contact>::iterator it = contacts.begin(); it != contacts.end(); ++it)
    {
        updated |= MergeContacts(*it);
    }
    return updated;
}
void App::ConnectWithPrevious()
{
    Message msg = Message::Receive(globalMessageMaxSize, globalQueue, "globalQueue", false, 0, 0);
    Contact contact = Contact(msg.sender, personalQueueNamePrefix, personalMessageMaxCount, personalMessageMaxSize);
    contact.RequestContacts();
    MergeContacts(self.ReceiveContacts(personalMessageMaxSize, personalQueueNamePrefix, personalMessageMaxCount, personalMessageMaxSize, retryTimeBase_ms, retryTimeRand_ms));
    self.Update(self.symbols);
    MergeContacts(self);
    contact.SendContacts(this->contacts);
}
void App::Listen()
{
    Debug::Log("Started listening to personalQueue");
    logicsStartTime = Timer::GetOffsetedTime_msec(waitBeforeLogicsTimeBase_ms, waitBeforeLogicsTimeRand_ms);
    bool logicsOffsetNeeded = false;
    while(!finishing)
    {
        Message msg = Message::Receive(personalMessageMaxSize, personalQueue, "", false, 0, 0);
        switch(msg.type)
        {
            case MessageType::requestContacts:
            {
                MergeContacts(self);
                Contact contact = Contact(msg.sender, personalQueueNamePrefix, personalMessageMaxCount, personalMessageMaxSize);
                MergeContacts(contact);
                contact.SendContacts(this->contacts);
                logicsOffsetNeeded = true;
                break;
            }
            case MessageType::sendContacts:
            {
                bool updated = MergeContacts(Contact::ContactsFromMessage(msg, personalQueueNamePrefix, personalMessageMaxCount, personalMessageMaxSize));
                Debug::Log("Got contacts from " + to_string(msg.sender));
                if(updated)
                {
                    for(int i = 0; i < contacts.size(); i++)
                    {
                        if(contacts[i].pid != getpid() && contacts[i].pid != msg.sender)
                        {
                            contacts[i].SendContacts(contacts);
                            Debug::Log("Sent updated contacts to " + contacts[i].pid);
                        }
                    }
                }
                logicsOffsetNeeded = true;
                break;
            }
            case MessageType::finish:
            {
                Debug::Log("Finished balancing with this result: " + self.symbols);
                finishing = true;
                break;
            }
            case MessageType::giveMarker:
            {
                hasMarker = true;
                break;
            }
            case MessageType::sendSymbol:
            {
                char c = msg.data[0][0];
                self.Update(self.symbols + c);
                MergeContacts(self);
                SendUpdatedContacts();
                break;
            }
        }
        if(logicsOffsetNeeded)
        {
            logicsStartTime = Timer::GetOffsetedTime_msec(waitBeforeLogicsTimeBase_ms, waitBeforeLogicsTimeRand_ms);
            logicsOffsetNeeded = false;
        }
        if(boost::posix_time::microsec_clock::universal_time() > logicsStartTime && hasMarker)
        {
            LogicsIteration();
            logicsOffsetNeeded = true;
        }
        Timer::Wait_msec(retryTimeBase_ms, retryTimeRand_ms);
    }
}
State App::GetState()
{
    State state;
    state.symbolsCount = App::CountSymbols(contacts);
	state.copies = App::GetCopies(contacts, state.symbolsCount);
	state.fullness = App::GetFullness(contacts, state.symbolsCount);
	state.copiesCount = App::CountCopies(state.copies);
	state.fullnessNotNominalCount = App::CountFullnessNotNominal(state.fullness, state.symbolsCount);
    return state;
}
void App::LogicsIteration()
{
    State state = GetState();
    if(state.copiesCount == 0 && state.fullnessNotNominalCount == 0)
    {
        Debug::Log("Finished balancing with this result: " + self.symbols);
        finishing = true;
    }
    if(finishing)
    {
        for(int i = 0; i < contacts.size(); i++)
        {
            contacts[i].SendFinish();
        }
    }
    else
    {
        int selfIndex = GetSelfIndex();
        bool removedSomeCopies = false;
        int indexReceiver = -1;
        int maxDice = -1;
        for(int j = 0; j < contacts.size(); j++)
        {
            if(selfIndex != j && state.fullness[selfIndex] - state.fullness[j] >= 1.0f / state.symbolsCount)
            {
                int dice = (rand() % (contacts.size() * state.symbolsCount)) *
                    (state.fullness[selfIndex] - state.fullness[j]);
                if(maxDice < dice)
                {
                    maxDice = dice;
                    indexReceiver = j;
                }
            }
            if(state.copies[selfIndex][j].length() > 0)
            {
                if(state.fullness[selfIndex] > state.fullness[j] ||
                    state.fullness[selfIndex] == state.fullness[j] && (rand() % 2 == 1))
                {
                    self.Update(App::RemoveCopies(state.copies[selfIndex][j], contacts[selfIndex].symbols));
                    MergeContacts(self);
                    SendUpdatedContacts();
                    removedSomeCopies = true;
                    //break;
                    state = GetState();
                }
            }
        }
        if(!removedSomeCopies && indexReceiver != -1)
        {
            char c = self.symbols[self.symbols.length() - 1];
            self.Update(self.symbols.substr(0, self.symbols.length() - 1));
            MergeContacts(self);
            SendUpdatedContacts();
            contacts[indexReceiver].SendSymbol(c);
        }
        if(selfIndex + 1 < contacts.size())
        {
            contacts[selfIndex + 1].GiveMarker();
        }
        else
        {
            contacts[0].GiveMarker();
        }
        hasMarker = false;
    }
}
int App::GetSelfIndex()
{
    for(int i = 0; i < contacts.size(); i++)
    {
        if(contacts[i].pid == self.pid)
        {
            return i;
        }
    }
    return -1;
}
void App::SendUpdatedContacts()
{
    for(int i = 0; i < contacts.size(); i++)
    {
        if(contacts[i].pid != getpid())
        {
            contacts[i].SendContacts(contacts);
            Debug::Log("Sent updated contacts to " + contacts[i].pid);
        }
    }
}
void App::SendUpdatedContacts(long excludedPid)
{
    for(int i = 0; i < contacts.size(); i++)
    {
        if(contacts[i].pid != getpid() && contacts[i].pid != excludedPid)
        {
            contacts[i].SendContacts(contacts);
            Debug::Log("Sent updated contacts to " + contacts[i].pid);
        }
    }
}
string App::RemoveCopiesInSelf(string str)
{
    string result = "";
    for(int i = 0; i < str.length(); i++)
    {
        size_t index = result.find(str[i]);
        if(index == string::npos)
        {
            result += str[i];
        }
    }
    return result;
}
int App::CountSymbols(vector<Contact> contacts)
{
    int result = 0;
    for(int i = 0; i < contacts.size(); i++)
    {
        result += contacts[i].symbols.length();
    }
    return result;
}
string App::FindCopies(string source, string destination)
{
    string result = "";
    for(int i = 0; i < source.length(); i++)
    {
        size_t index = destination.find(source[i]);
        if(index != string::npos)
        {
            result += source[i];
        }
    }
    return result;
}
vector< vector<string> > App::GetCopies(vector<Contact> contacts, int symbolsCount)
{
    vector< vector<string> > result;
    for(int i = 0; i < contacts.size(); i++)
    {
        vector<string> row;
        for(int j = 0; j < contacts.size(); j++)
        {
            if(i == j)
            {
                row.push_back("");
            }
            else
            {
                row.push_back(App::FindCopies(contacts[j].symbols, contacts[i].symbols));
            }
        }
        result.push_back(row);
    }
    return result;
}
int App::CountCopies(vector< vector<string> > copies)
{
    int result = 0;
    for(int i = 0; i < copies.size(); i++)
    {
        for(int j = 0; j < copies.size(); j++)
        {
            result += copies[i][j].length();
        }
    }
    return result;
}
vector<float> App::GetFullness(vector<Contact> contacts, int symbolsCount)
{
    vector<float> result;
    for(int i = 0; i < contacts.size(); i++)
    {
        result.push_back((float)contacts[i].symbols.length() / (float)symbolsCount -
            1.0 / (float)contacts.size());
    }
    return result;
}
int App::CountFullnessNotNominal(vector<float> fullness, int symbolsCount)
{
    int result = 0;
    for(int i = 0; i < fullness.size(); i++)
    {
        if(abs(fullness[i]) >= 1.0f / (float)symbolsCount)
        {
            result++;
        }
    }
    return result;
}
string App::RemoveCopies(string copies, string str)
{
    string result = "";
    for(int i = 0; i < str.length(); i++)
    {
        size_t index = copies.find(str[i]);
        if(index == string::npos)
        {
            result += str[i];
        }
    }
    return result;
}
