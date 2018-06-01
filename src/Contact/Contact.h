
#ifndef Contact_H
#define Contact_H

#include <vector>
#include <boost/date_time.hpp>
#include <sstream>
#include <mqueue.h>

#include "../Message/Message.h"

using namespace std;

class Contact
{
	private:
		mqd_t queue;
		string queueName;
		string personalQueueNamePrefix;
		int messageMaxCount;
		int messageMaxSize;
	
	public:
		long pid;
		boost::posix_time::ptime epoch;
		string symbols;

		Contact();
		Contact(long pid, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize);
		Contact(long pid, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize, string symbols);
		Contact(long pid, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize, string symbols, boost::posix_time::ptime epoch);
		~Contact();
		//void RequestSymbols();
		//void ReceiveSymbols();
		//void SendSymbols(string symbols);
		void RequestContacts();
		vector<Contact> ReceiveContacts(int personalMessageMaxSize, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize, int retryTimeBase_ms, int retryTimeRand_ms);
		void SendContacts(vector<Contact> contacts);
		void SendFinish();
		void GiveMarker();
		void Update(string symbols);
		void Update(string symbols, boost::posix_time::ptime epoch);
		void SendSymbol(char c);
		
		static vector<Contact> ContactsFromMessage(Message msg, string personalQueueNamePrefix, int messageMaxCount, int messageMaxSize);
		static Message ContactsToMessage(vector<Contact> contacts);
};

#endif
