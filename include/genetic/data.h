#ifndef DATAMEMORY_H
#define DATAMEMORY_H

//include "structures.h"
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "processing.h"

#include "genetic/graph.h"

#include "utils/includespdlog.h"
#include "utils/configreader.h"

class DataMemory : public QObject {
  Q_OBJECT

public:
	DataMemory();
	DataMemory(QJsonObject jDataset);
	~DataMemory();
	bool loadData(QJsonObject jDataset);
	bool preprocess(QJsonArray m_dataGraph);
	bool getLoad() { return m_loaded; };

	cv::Mat gt(qint32 i) { return m_gtData[i]; }
	cv::Mat input(qint32 i) { return m_inputData[i]; }

	qint32 getSize() { return m_inputData.size(); }
	qint32 getSizeGT() { return m_gtData.size(); }

	void loadGraph(QJsonArray m_dataGraph);
	void clearDataForNextIteration();
	bool checkIfLoadInputs(const int i, const QJsonArray _prevActive, std::vector<_data> & dataVec, std::vector<cv::Mat> &input);
	bool checkIfReturnData(const QJsonArray _nextActive);
	void loadInputs(const QJsonArray _prevActive, std::vector<_data> & dataVec);
	void returnData(int i, std::vector<cv::Mat> & m_outputData);

private:
	void loadDataFromStream(cv::VideoCapture videoFromFile, std::vector<cv::Mat>& m_cleanData, bool resize);
	bool configure(QJsonObject a_config);

signals:
  void memoryLoaded();

private:
	cv::VideoCapture m_videoFromFile;
	cv::VideoCapture m_videoFromFileGT;

	std::vector<cv::Mat> m_cleanData;
	std::vector<cv::Mat> m_gtCleanData;

	std::vector<cv::Mat> m_inputData;
	std::vector<cv::Mat> m_gtData;

private:
	bool m_loaded{};
	QString m_folderInput;
	QString m_roi;
	QString m_cleanTrain{};
	QString m_gtTrain{};
	qint32 m_stopFrame{};
	qint32 m_startFrame{};
	qint32 m_startGT{};
	qint32 m_stopGT{};
	bool m_resize{};
	qint32 m_width;
	qint32 m_height;

	QString m_clean;
	QString m_gt;

private:
	QJsonArray m_graph;
	std::vector<Processing*> m_block;
	std::vector<std::vector<_data>> m_data;

	std::vector<cv::Mat> m_outputData;

private:
	QString m_inputType;
	QString m_outputType;
	QString m_split;

	Graph<Processing, _data> m_graph_processing;
};

#endif // DATAMEMORY_H