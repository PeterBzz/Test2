
#ifndef Message_H
#define Message_H

#include <vector>
#include <boost/date_time.hpp>
#include <sstream>
#include <mqueue.h>

#include "../Debug/Debug.h"
#include "../Timer/Timer.h"

using namespace std;

enum MessageType
{
	undefined,
	lock,
	hello,
	//requestSymbols,
	//sendSymbols,
	requestContacts,
	sendContacts,
	//giveContacts,
	//getContacts,
	giveMarker,
	finish,
	sendSymbol
};

class Message
{
	private:

	public:
		long sender;
		MessageType type;
		vector<string> data;

		Message();
		Message(MessageType type);
		Message(MessageType type, vector<string> data);
		Message(string str);
		void Send(mqd_t queue, string contactName);
		string ToString();
		
		static string TypeToString(MessageType type);
		static MessageType TypeFromString(string str);
		static Message Receive(int maxMessageSize, mqd_t queue, string contactName, bool tryTillSuccess, int retryTimeBase_ms, int retryTimeRand_ms);
};

#endif
