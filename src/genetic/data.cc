#include "genetic/data.h"

constexpr auto GRAPH{ "Graph" };
constexpr auto NAME{ "Name" };
constexpr auto ACTIVE{ "Active" };
constexpr auto COPY_SIGNAL{ "Signal" };
constexpr auto TYPE{ "Type" };
constexpr auto NEXT{ "Next" };
constexpr auto PREV{ "Prev" };
constexpr auto CONFIG{ "Config" };
constexpr auto WIDTH{ "Width" };
constexpr auto HEIGHT{ "Height" };

constexpr auto NOISE{"Noise"};
constexpr auto FOLDER{"Input"};
constexpr auto CLEAN_TRAIN{ "clean_train" };
constexpr auto GT_TRAIN{ "gt_train" };

constexpr auto CLEAN{ "clean" };
constexpr auto GT{ "gt" };

constexpr auto STREAM_INPUT{"StreamInput"};
constexpr auto VIDEO_GT{"VideoGT"};
constexpr auto START_FRAME{"StartFrame"};
constexpr auto STOP_FRAME{"StopFrame"};
constexpr auto START_GT{"StartGT"};
constexpr auto STOP_GT{"StopGT"};
constexpr auto RESIZE{"Resize"};

constexpr auto PATH_TO_DATASET{ "PathToDataset" };
constexpr auto CONFIG_NAME{ "ConfigName" };

constexpr auto INPUT_TYPE{ "InputType" };
constexpr auto OUTPUT_TYPE{ "OutputType" };
constexpr auto INPUT_PREFIX{ "InputPrefix" };
constexpr auto DATASET_UNIX{ "DatasetLinux" };
constexpr auto DATASET_WIN32{ "DatasetWin32" };

//#define DEBUG
//#define DEBUG_PREPROCESS
//#define DEBUG_OPENCV

DataMemory::DataMemory()
{
	#ifdef DEBUG
	Logger->debug("DataMemory::DataMemory()");
	#endif
	DataMemory::createSplit();
}

DataMemory::DataMemory(QJsonObject jDataset)
{
	#ifdef DEBUG
	Logger->debug("DataMemory::DataMemory()");
	#endif
	DataMemory::createSplit();
	loadData(jDataset);
}

DataMemory::~DataMemory(){}

void DataMemory::clearDataForNextIteration()
{
	m_data.clear();
	m_outputData.clear();
}

void DataMemory::createSplit()
{
	#ifdef _WIN32
	m_split = "\\";
	#endif // _WIN32
	#ifdef __linux__
	m_split = "/";
	#endif // _UNIX
}

bool DataMemory::preprocess(QJsonArray dataGraph)
{
	#ifdef DEBUG_PREPROCESS
	Logger->debug("DataMemory::preprocess()");
	#endif
	m_inputData.clear();
	m_gtData.clear();
	m_data.clear();
	m_block.clear();
	m_graph = dataGraph;

	m_graph_processing.loadGraph(dataGraph, m_block);
	#ifdef DEBUG_PREPROCESS
	Logger->debug("DataMemory::preprocess() m_cleanData.size():{}",  m_cleanData.size());
	Logger->debug("DataMemory::preprocess() m_graph.size():{}",  m_graph.size());
	#endif
	
	for (qint32 iteration = 0; iteration < m_cleanData.size(); iteration++)
	{
		if (iteration % 100 == 0)
		{
			Logger->info("DataMemory::preprocess() processing:{}/{} m_inputData.size():{}", iteration, m_cleanData.size(), m_inputData.size());
			Logger->info("DataMemory::preprocess() processing:{}/{} m_gtData.size():{}", iteration, m_gtData.size(), m_gtData.size());
		}
		std::vector<cv::Mat> input{ m_cleanData[iteration], m_gtCleanData[iteration] };

		clearDataForNextIteration();
		
		// PROCESSING
		for (int i = 0; i < m_graph.size(); i++) 
		{
			std::vector<_data> dataVec;
			const QJsonObject _obj = m_graph[i].toObject();
			const QJsonArray _prevActive = _obj[PREV].toArray();
			const QJsonArray _nextActive = _obj[NEXT].toArray();
 
			if (m_graph_processing.checkIfLoadInputs(_prevActive, dataVec, input, i))
			{
				m_graph_processing.loadInputs(_prevActive, dataVec, dataGraph, m_data);				
			}
			try
			{
				m_block[i]->process((dataVec));
			}
			catch (cv::Exception& e)
			{
				const char* err_msg = e.what();
				qDebug() << "exception caught: " << err_msg;
			}
			m_data.push_back((dataVec));
			if (m_graph_processing.checkIfReturnData(_nextActive))
			{
				m_graph_processing.returnData(i, m_outputData, m_data);
			}
			dataVec.clear();
			#ifdef DEBUG_OPENCV
			Logger->debug("DataMemory::preprocess() push_back to m_inputData");
			if (m_outputData.size() > 0)
			{
				cv::imshow("m_outputData:", m_outputData[0]);
				cv::waitKey(0);
			}
			#endif
		}
		if (m_outputData.size() > 1)
		{
			m_inputData.push_back(m_outputData[0]);
			m_gtData.push_back(m_outputData[1]);
		}
		#ifdef DEBUG
		Logger->debug("DataMemory::preprocess() iteration done");
		#endif
	}

	#ifdef DEBUG
	Logger->debug("DataMemory::preprocess() sizes: m_cleanData:{}, m_gtCleanData{}", m_cleanData.size(), m_gtCleanData.size());
	Logger->debug("DataMemory::preprocess() sizes: m_inputData:{}, m_gtData{}", m_inputData.size(), m_gtData.size());
	#endif

	if(m_inputData.size() != m_cleanData.size())
	{
		Logger->error("DataMemory::preprocess() size not correct:");
	}
	m_loaded = true;
	
	if(false)
	{
		for(int i = 0 ; i < m_inputData.size() ; i++)
		{
			QString name_all = (m_cleanTrain +  m_split + QString::number(i) + m_outputType);
			Logger->trace("write:{}", (name_all).toStdString());
			cv::imwrite(name_all.toStdString(), m_inputData[i]);

			name_all = (m_gtTrain + m_split + QString::number(i) + m_outputType);
			Logger->trace("write:{}", (name_all).toStdString());
			cv::imwrite(name_all.toStdString(), m_gtData[i]);
		}
	}
		
	emit(memoryLoaded());
	return true;
}

bool DataMemory::configure(QJsonObject a_config)
{
	#ifdef DEBUG
	qDebug() << "DataMemory::loadData() a_config:"<< a_config;
	#endif
	#ifdef _WIN32
	QJsonObject jDataset{ a_config[DATASET_WIN32].toObject() };
	#endif // _WIN32
	#ifdef __linux__ 
	QJsonObject jDataset{ a_config[DATASET_UNIX].toObject() };
	#endif // _UNIX
	#ifdef DEBUG
	qDebug() << "DataMemory::loadData() jDataset:"<< jDataset;
	#endif
	QString m_configName = jDataset[CONFIG_NAME].toString();
	QString m_pathToConfig = jDataset[PATH_TO_DATASET].toString();
	QJsonObject m_datasetConfig;
	Logger->trace("DataMemory::loadData() open config file:{}", (m_pathToConfig + m_configName).toStdString());
	ConfigReader* configReader = new ConfigReader();
	if (!configReader->readConfig(m_pathToConfig + m_configName, m_datasetConfig))
	{
		Logger->error("DataMemory::loadData() File {} not readed", (m_pathToConfig + m_configName).toStdString());
	}
	delete configReader;
	m_startGT = m_datasetConfig[START_GT].toInt();
	m_stopGT = m_datasetConfig[STOP_GT].toInt();
	m_resize = m_datasetConfig[RESIZE].toBool();
	m_width = m_datasetConfig[WIDTH].toInt();
	m_height = m_datasetConfig[HEIGHT].toInt();
	m_cleanTrain = m_pathToConfig + m_datasetConfig[CLEAN_TRAIN].toString() + m_split;
	m_gtTrain = m_pathToConfig + m_datasetConfig[GT_TRAIN].toString() + m_split;

	m_clean = m_pathToConfig + m_datasetConfig[CLEAN].toString() + m_split + m_datasetConfig[INPUT_PREFIX].toString();
	m_gt = m_pathToConfig + m_datasetConfig[GT].toString() + m_split + m_datasetConfig[INPUT_PREFIX].toString();

	m_outputType = m_datasetConfig[OUTPUT_TYPE].toString();

	#ifdef DEBUG_OPENCV
		Logger->debug("DataMemory::preprocess() m_clean:{}", m_clean.toStdString());
		Logger->debug("DataMemory::preprocess() m_gt:{}", m_gt.toStdString());
		Logger->debug("DataMemory::preprocess() m_cleanTrain:{}", m_cleanTrain.toStdString());
		Logger->debug("DataMemory::preprocess() m_gtTrain:{}", m_gtTrain.toStdString());
	#endif
}

bool DataMemory::loadData(QJsonObject a_config)
{
	configure(a_config);
	int ret{ 1 };
	#ifdef DEBUG
	Logger->debug("DataMemory::loadData() m_clean data:{}", (m_clean).toStdString());
	#endif
	ret = m_videoFromFile.open(m_clean.toStdString());
	if (ret < 0)
	{
		Logger->error("input data failed to open:{}", (m_clean).toStdString());
	}

	Logger->trace("m_gt data:{}", (m_gt).toStdString());
	ret = m_videoFromFileGT.open(m_gt.toStdString());
	if (ret < 0)
	{
		Logger->error("input data failed to open:{}", (m_gt).toStdString());
	}
	loadDataFromStream(m_videoFromFile, m_cleanData, true);
	loadDataFromStream(m_videoFromFileGT, m_gtCleanData, true);
	#ifdef DEBUG
	Logger->debug("DataMemory::loadData() m_cleanData.size data:{}", (m_cleanData).size());
	Logger->debug("DataMemory::loadData() m_gtCleanData.size data:{}", (m_gtCleanData).size());
	#endif

	m_loaded = true;
	emit(memoryLoaded());
	
	return true;
}

void DataMemory::loadDataFromStream(cv::VideoCapture videoFromFile, std::vector<cv::Mat> &data, bool resize)
{
	#ifdef DEBUG
	Logger->info("DataMemory::loadDataFromStream()");
	#endif
	int iter{ 0 };
	while (videoFromFile.isOpened())
	{
		cv::Mat inputMat;
		videoFromFile >> inputMat;
		if (resize && inputMat.channels() > 1)
		{
			cv::cvtColor(inputMat, inputMat, 6);
		}
		iter++;
		if (inputMat.cols == 0 || inputMat.rows == 0 || iter >= m_stopGT)
		{
			#ifdef DEBUG
			Logger->info("push_back {} images end", iter);
			#endif
			break;
		}
		data.push_back(inputMat);
	}
}