#include <QCoreApplication>

#include "utils/configreader.h"
#include "utils/includespdlog.h"

#include "genetic/data.h"
#include "genetic/mainloop.h"

#include <X11/Xlib.h>

constexpr auto CONFIG{ "config.json" };
constexpr auto PREPROCESS{ "preprocess.json" };
constexpr auto LOG_LEVEL{ "LogLevel" };
constexpr auto GENERAL{ "General" };
constexpr auto PID{ "Pid" };

void intro();

QJsonObject readConfig(QString name);

int main(int argc, char* argv[])
{
	//XInitThreads();
	XInitThreads();
	QCoreApplication app(argc, argv);
	

	//qRegisterMetaType<QString>("QString");

	Logger->set_level(static_cast<spdlog::level::level_enum>(0));
	Logger->set_pattern("[%Y-%m-%d] [%H:%M:%S.%e] [%t] [%^%l%$] %v");

	QJsonObject config = readConfig(QString::fromStdString(CONFIG));
	QJsonObject preConfig = readConfig(QString::fromStdString(PREPROCESS));
	intro();
	qint32 messageLevel{ config[GENERAL].toObject()[LOG_LEVEL].toInt() };
	Logger->info("messageLevel:{}", messageLevel);
	Logger->set_level(static_cast<spdlog::level::level_enum>(messageLevel));

	Logger->info("start DataMemory:");
	//DataMemory dm{config};
	//dm.preprocess(preConfig);
	//window.show();
	//qint64 pid = app.applicationPid();
	//config[PID] = pid;
	//qDebug() << "config" << config;
	MainLoop mainLoop{ config};

	return app.exec();
}

void intro() {
	Logger->info("\n\n\t\033[1;31mGenetic v3.0\033[0m\n"
		"\tAuthor: Grzegorz Matczak\n"
		"\t02.06.2021\n");
}

QJsonObject readConfig(QString name)
{
	QString configName{ name };
	std::shared_ptr<ConfigReader> cR = std::make_shared<ConfigReader>();
	QJsonObject jObject;
	if (!cR->readConfig(configName, jObject))
	{
		Logger->error("File {} read confif failed", configName.toStdString());
		exit(-1);
	}
	return jObject;
}
