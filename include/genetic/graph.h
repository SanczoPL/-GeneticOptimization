#ifndef GRAPH_H
#define GRAPH_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "processing.h"
#include "postprocessing.h"

#include "utils/includespdlog.h"
#include "utils/configreader.h"

template<typename T, typename T_data>
class Graph
{
	public:
		Graph();
		void loadGraph(QJsonArray &a_graph, QJsonArray &a_config, std::vector<T*> &a_block);
		void loadGraph(QJsonArray &a_graph, std::vector<T*> &a_block);

		bool checkIfLoadInputs(const QJsonArray & prevActive, std::vector<T_data> & dataVec, cv::Mat &input);
		bool checkIfLoadInputs(const QJsonArray & prevActive, std::vector<T_data> & dataVec, std::vector<cv::Mat> &input, int i);
		void loadInputs(const QJsonArray &prevActive, std::vector<T_data> & dataVec, QJsonArray &a_graph,
					 std::vector<std::vector<T_data>>& a_data);

		bool checkIfReturnData(const QJsonArray &nextActive);
		void returnData(int i, std::vector<cv::Mat> & outputData, std::vector<std::vector<T_data>> &outputDataVector,
							std::vector<std::vector<T_data>>& data);
							
		void returnData(int i, std::vector<cv::Mat> & outputData, std::vector<std::vector<T_data>>& data);
		
};

#endif // CASE_H
