#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QDebug>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


#include "utils/includespdlog.h"


class FileLogger : public QObject {
	Q_OBJECT
	public:
		explicit FileLogger();

	signals:

	public slots:
		void configure(QString name, bool additionalLogs);
		void appendFileLogger(QStringList list);

		void append(QStringList list, QFile* file);
		void clear();
		void logJsonBest(QJsonObject json);

	private:
		QString m_name;
		QFile* m_file;
		QFile* m_fileAdditional;
		bool m_additionalLogs;
};

#endif // FILELOGGER_H