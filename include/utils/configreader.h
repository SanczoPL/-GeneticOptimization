#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <stdio.h>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>

#include "utils/includespdlog.h"

class ConfigReader : public QObject
{
	public:
		ConfigReader();
		~ConfigReader();
		bool readConfig(QString configPathWithName, QJsonObject& obj);

};
#endif //CONFIG_READER_H
