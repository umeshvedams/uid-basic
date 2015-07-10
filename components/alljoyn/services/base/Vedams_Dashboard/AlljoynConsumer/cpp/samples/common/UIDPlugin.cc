#include "UIDPlugin.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

UIDUtils::UIDUtils(){

}

UIDUtils::~UIDUtils(){

}



void UIDUtils::setAJNotificationDataField(string key,string value)
{


	if (!key.compare("MessageId")) {
		myNotificationData.MessageId = value;
	} else if (!key.compare("DeviceId")){
		myNotificationData.DeviceId = value;
	} else if (!key.compare("DeviceName")){
		myNotificationData.DeviceName = value;
	} else if (!key.compare("AppId")){
		myNotificationData.AppId = value;
	} else if (!key.compare("AppName")){
		myNotificationData.AppName = value;
	} else if (!key.compare("SenderBusName")){
		myNotificationData.SenderBusName = value;
	} else if (!key.compare("MessageType")){
		myNotificationData.MessageType = value;
	} else if (!key.compare("Message")){
		myNotificationData.Message = value;
	} else if (!key.compare("NotificationVersion")){
		myNotificationData.NotificationVersion = value;
	} else {
		cerr << "UIDUtils::setAJnotificationDataField invalid call" << endl;
	}


}

string UIDUtils::getAJNotificationDataField(string key)
{

	if (!key.compare("MessageId")) {
		return myNotificationData.MessageId;
	} else if (!key.compare("DeviceId")) {
		return myNotificationData.DeviceId;
	} else if (!key.compare("DeviceName")) {
		return myNotificationData.DeviceName;
	} else if (!key.compare("AppId")) {
		return myNotificationData.AppId;
	} else if (!key.compare("AppName")) {
		return myNotificationData.AppName;
	} else if (!key.compare("SenderBusName")) {
		return myNotificationData.SenderBusName;
	} else if (!key.compare("MessageType")) {
		return myNotificationData.MessageType;		
	} else if (!key.compare("Message")) {
		return myNotificationData.Message;
	} else if (!key.compare("NotificationVersion")) {
		return myNotificationData.NotificationVersion;
	} else {
		cerr << "UIDUtils::getAJNotificationDataField invalid call" << endl;
	} 

}

void UIDUtils::insertAJNotificationDataToDb()
{
	string hostname = "localhost";
	string username = "root";
	string password = "vedams123";

	MYSQL *conn;

	string createTable = "CREATE TABLE IF NOT EXISTS UIDNotificationTable (\
		MessageId VARCHAR(30) NOT NULL,\
		DeviceId VARCHAR(30) NOT NULL,\
		DeviceName VARCHAR(30) NOT NULL,\
		AppId VARCHAR(30) NOT NULL,\
		AppName VARCHAR(30) NOT NULL,\
		SenderBusName VARCHAR(30) NOT NULL,\
		MessageType VARCHAR(30) NOT NULL,\
		Message VARCHAR(30) NOT NULL,\
		NotificationVersion VARCHAR(30) NOT NULL)";

	
	string insertTable = "INSERT INTO UIDNotificationTable (MessageId,\
		DeviceId,DeviceName,AppId,AppName,SenderBusName,MessageType,\
		Message,NotificationVersion) VALUES ( '" + myNotificationData.MessageId + "', \
		'" + myNotificationData.DeviceId + "', '" + myNotificationData.DeviceName + "',\
		'" + myNotificationData.AppId + "', '" + myNotificationData.AppName + "',\
		'" + myNotificationData.SenderBusName + "', '" + myNotificationData.MessageType + "',\
		'" + myNotificationData.Message + "','" + myNotificationData.NotificationVersion + "')";

	
 	string searchTable = "SELECT * FROM UIDNotificationTable";

	if ( ( conn = mysql_init(NULL)) == NULL) {
		cerr << "MYSQL ERROR: " << mysql_error(conn) << endl;
		exit(0);  
	}

	if ( mysql_real_connect(conn,hostname.c_str(),username.c_str(),password.c_str(),NULL,0,NULL,0) == NULL) {
		cerr << "MYSQL ERROR: " << mysql_error(conn) << endl;
		exit(0);
	}

	if ( mysql_query(conn, "CREATE DATABASE IF NOT EXISTS UIDTest")){
		cerr << "MYSQL ERROR: " << mysql_error(conn) << endl;
		exit(0);
	}


	if ( mysql_query(conn, "USE UIDTest")) {
		cerr << "MYSQL ERROR: " << mysql_error(conn) << endl;
		exit(0);
	}


	if ( mysql_query(conn, createTable.c_str())) {
		cerr << "MYSQL ERROR: " << mysql_error(conn) << endl;
		exit(0);
	}


		if (mysql_query(conn,insertTable.c_str())) {
			cerr << "MYSQL ERROR: " << mysql_error(conn) << endl;
			exit(0);
		}
	
//	mysql_free_result(result);
	mysql_close(conn);

	
	
}
