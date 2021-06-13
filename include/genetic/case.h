#ifndef CASE_H
#define CASE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "processing.h"
#include "postprocessing.h"

#include "genetic/data.h"
#include "genetic/graph.h"
#include "utils/includespdlog.h"
#include "utils/configreader.h"


class Case : public QObject
{
	Q_OBJECT

public:
	explicit Case(DataMemory* data);
	fitness onConfigureAndStart(QJsonArray a_graph, QJsonArray a_config, QJsonArray a_postprocess);

public slots:
	void onUpdate();
	void onConfigureAndStartSlot(QJsonArray a_graph, QJsonArray a_config, QJsonArray a_postprocess, int processSlot);
	
private:
	fitness process();
	void configure(QJsonArray a_graph, QJsonArray a_config, QJsonArray a_postprocess);
	void clearDataForNextIteration();

signals:
	void quit();
	void signalOk(struct fitness fs, qint32 slot);
	void configureAndStartSlot(QJsonArray a_graph, QJsonArray a_config, QJsonArray a_postprocess, qint32 processSlot);

private:
	cv::TickMeter m_timer;
	DataMemory* m_dataMemory;
	QJsonArray m_config;
	QJsonArray m_graph_config;
	QJsonArray m_postprocess_config;
	std::vector<Processing*> m_block;
	std::vector<PostProcess*> m_blockPostprocess;

	std::vector<std::vector<_data>> m_data;
	std::vector<std::vector<_postData>> m_dataPostprocess;

	std::vector<cv::Mat> m_outputData;
	bool m_firstTime{};

private:
	Graph<Processing, _data> m_graph_processing;
	Graph<PostProcess, _postData> m_graph_postprocessing;

};

#endif // CASE_H
