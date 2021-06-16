#include "utils/configreader.h"

//#define DEBUG

ConfigReader::ConfigReader()
{
	#ifdef DEBUG
	Logger->debug("ConfigReader::ConfigReader()");
	#endif
}

ConfigReader::~ConfigReader() {}

bool ConfigReader::readConfig(QString configPathWithName, QJsonObject& obj) 
{
	#ifdef DEBUG
	Logger->debug("ConfigReader::readConfig(QJsonObject) try to load {}", (configPathWithName).toStdString());
	#endif
	QFile jConfigFile{ configPathWithName };
	if (!jConfigFile.open(QIODevice::ReadOnly))
	{
		Logger->error("Can not open json file:{}", (configPathWithName).toStdString());
		return false;
	}

	QJsonDocument jConfigDoc{ QJsonDocument::fromJson((jConfigFile.readAll())) };
	if (!jConfigDoc.isObject())
	{
		Logger->error("Invalid json config file:{}", (configPathWithName).toStdString());
		return false;
	}
	
	obj = { jConfigDoc.object() };
	#ifdef DEBUG
	Logger->debug("ConfigReader::readConfig() {} load successful", (configPathWithName).toStdString());
	#endif
	return true;
}

bool ConfigReader::readConfig(QString configPathWithName, QJsonArray& obj) 
{
	#ifdef DEBUG
	Logger->debug("ConfigReader::readConfig(QJsonArray) try to load {}", (configPathWithName).toStdString());
	#endif
	QFile jConfigFile{ configPathWithName };
	if (!jConfigFile.open(QIODevice::ReadOnly))
	{
		Logger->error("Can not open json file:{}", (configPathWithName).toStdString());
		return false;
	}

	QJsonDocument jConfigDoc{ QJsonDocument::fromJson((jConfigFile.readAll())) };
	if (!jConfigDoc.isArray())
	{
		Logger->error("Invalid json config file:{}", (configPathWithName).toStdString());
		return false;
	}
	
	obj = { jConfigDoc.array() };
	#ifdef DEBUG
	Logger->debug("ConfigReader::readConfig() {} load successful", (configPathWithName).toStdString());
	#endif
	return true;
}
