
#include "Debug.h"

void Debug::Log(string text)
{
    stringstream ss;
    ss << boost::posix_time::microsec_clock::universal_time() << " " << text << endl;
    cout << ss.str();
    write(Debug::Singletone().file, ss.str().c_str(), ss.str().length());
}
Debug::Debug()
{
    stringstream ss;
    ss << "logs/" << getpid() << ".txt";
    file = open(ss.str().c_str(), O_WRONLY | O_CREAT | O_APPEND);
}
Debug::~Debug()
{
    close(file);
}
Debug& Debug::Singletone()
{
    static Debug s;
    return s;
}
