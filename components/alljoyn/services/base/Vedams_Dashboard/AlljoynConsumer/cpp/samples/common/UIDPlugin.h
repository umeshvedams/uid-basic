#ifndef UIDPLUGIN_H
#define UIDPLUGIN_H


#include <iostream>
#include <mysql/mysql.h>

using namespace std;


struct AJNotificationData {

	string MessageId;
	string DeviceId;
	string DeviceName;
	string AppId;
	string AppName;
	string SenderBusName;
	string MessageType;
	string Message;
	string NotificationVersion;
};

class UIDUtils {
	struct 	AJNotificationData myNotificationData;
	MYSQL *conn;

	public:
		void setAJNotificationDataField(string key, string value);
		string getAJNotificationDataField(string key);
		void insertAJNotificationDataToDb();


		UIDUtils();
		~UIDUtils();


};
#endif
	
