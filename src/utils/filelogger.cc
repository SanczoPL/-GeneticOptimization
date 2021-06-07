#include "utils/filelogger.h"

//#define DEBUG

FileLogger::FileLogger()
{
	m_file = new QFile("temp.json");
}

void FileLogger::configure(QString name, bool additionalLogs)
{
	#ifdef DEBUG
	Logger->debug("FileLogger::configure() ");
	#endif
	if (name == "")
	{
		Logger->error("FileLogger::configure() filename not set");
		name = "error";
	}

	m_name = name;
	m_file->setFileName(name);
	#ifdef DEBUG
	Logger->debug("FileLogger::configure() <{}>", name.toStdString().c_str());
	#endif
}

void FileLogger::appendFileLogger(QStringList list)
{
	#ifdef DEBUG
	Logger->debug("FileLogger::appendFileLogger() ");
	#endif
	append(list, m_file);
}


void FileLogger::append(QStringList list, QFile* file)
{
	#ifdef DEBUG
	Logger->debug("FileLogger::Append() clean:");
	#endif
	if (file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		QTextStream stream(file);
		for (int i = 0; i < list.size(); ++i)
		{
			stream << list.at(i).toLocal8Bit().constData();
		}
		file->close();
	}
	#ifdef DEBUG
	Logger->debug("FileLogger::Append() clean: done");
	#endif
}

void FileLogger::clear()
{
	Logger->error("FileLogger::clear()");
	m_file->remove();
	if (m_file->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		m_file->close();
	}
}

void FileLogger::logJsonBest(QJsonObject json) {
	#ifdef DEBUG
	qDebug() << "FileLogger::logJsonBest:" << json;
	#endif
	auto test = QJsonDocument(json).toJson(QJsonDocument::Indented);
	m_file->open(QFile::WriteOnly);
	m_file->write(test);
	m_file->close();
}