#ifndef GENETIC_H
#define GENETIC_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "processing.h"
#include "genetic/structures.h"
#include "genetic/case.h"
#include "genetic/data.h"
#include "genetic/geneticoperation.h"

#include "utils/includespdlog.h"
#include "utils/configreader.h"
#include "utils/filelogger.h"


class Genetic : public QObject {
	Q_OBJECT
  
	public:
		Genetic(QVector<Case*> testCaseVector, DataMemory* data);
		~Genetic();
		void process();

	signals:
		void geneticConfigured();
		void newConfig();
		void logJsonBest(QJsonObject json);
		void appendToFileLogger(QStringList list);
		void configureLogger(QString name, bool additionalLogs);
		void configureLoggerJSON(QString name, bool additionalLogs);

	public slots:
		void configure(QJsonObject const& a_config, QJsonObject  const& a_boundsGraph, QJsonArray  const& a_graph, 
							QJsonArray a_postprocess, QJsonArray a_preprocess, int iterationGlobal);
		void clearPopulation();
		void clearFitness();
		void onSignalOk(struct fitness fs, qint32 slot);

	private:
		void loadFromConfig(QJsonObject const& a_config);
		void iteration();
		void logPopulation();
		void clearData();

	private:
		QRandomGenerator* m_randomGenerator;

	private:
		QVector<Case*> m_testCaseVector;
		DataMemory* m_dataMemory;
		QJsonArray m_graph;
		QJsonArray m_postprocess;
		QJsonObject m_boundsGraph;
		int m_populationSize{};
		bool m_configured{};
		QString m_resultsPath{};
		QVector<bool> m_bitFinish;
		QVector<bool> m_threadProcessing;
		qint32 m_bitFinishTest;
		QVector<qint32> m_actualManProcessing;

	private:
		qint32 m_mutateCounter{};
		qint32 m_crossoverCounter{};
		qint32 m_gradientCounter{};
		qint32 m_actualPopulationIteration{};
		
		qint32 m_bestNotChange{};
		qint32 m_bestChangeIteration{};
		double m_bestChangeLast{};
		double m_delta{};

		FileLogger *m_fileLogger;
		FileLogger* m_fileLoggerJSON;
		int m_iterationGlobal;
		QString m_fileName;
		GeneticOperation m_geneticOperation;
		Case* m_testCaseBest;
		cv::TickMeter m_timer;

		QString m_graphType;
		QString m_boundsType;
		QString m_dronType;
		QString m_logsFolder;
		QJsonObject m_config;

};
#endif // GENETIC_H
