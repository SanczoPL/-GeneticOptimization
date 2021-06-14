#include "gtest_graph.h"

constexpr auto TEST_GRAPH_CONFIG{ "test_graph_config.json" };
constexpr auto TEST_GRAPH_CONFIG_POSTPROCESSING{ "test_graph_config_postprocesssing.json" };
constexpr auto TEST_GRAPH{ "test_graph.json" };
constexpr auto TEST_GRAPH_POSTPROCESSING{ "test_graph_postprocessing.json" };
constexpr auto TEST_DATA{ "TestData" };
constexpr auto GRAPH{ "Graph" };
constexpr auto TEST_DATASET{ "test_dataset.json" };
constexpr auto TEST_PREPROCESS{ "test_preprocess.json" };

constexpr auto NAME{ "Name" };
constexpr auto ACTIVE{ "Active" };
constexpr auto COPY_SIGNAL{ "Signal" };
constexpr auto TYPE{ "Type" };
constexpr auto NEXT{ "Next" };
constexpr auto PREV{ "Prev" };
constexpr auto CONFIG{ "Config" };
constexpr auto WIDTH{ "Width" };
constexpr auto HEIGHT{ "Height" };

using ::testing::AtLeast;

//#define DEBUG_GRAPH


namespace gtest_graph
{
	TEST_F(GTest_graph, test_load_graph_processing)
	{

		
		Graph<Processing, _data> m_graph_processing;
		std::vector<Processing*> m_block;
		std::vector<PostProcess*> m_blockPostprocess;
		std::vector<std::vector<_data>> m_data;
		std::vector<cv::Mat> m_outputData;

		Logger->set_level(static_cast<spdlog::level::level_enum>(3));
		
		QJsonArray m_graph_config = GTest_graph::readArray(TEST_GRAPH);
		QJsonArray m_config = GTest_graph::readArray(TEST_GRAPH_CONFIG);
		QJsonArray m_preprocess = GTest_graph::readArray(TEST_PREPROCESS);
		QJsonObject m_dataset = GTest_graph::readConfig(TEST_DATASET);

		DataMemory* m_dataMemory = new DataMemory();
		m_dataMemory->configure(m_dataset);
		if(!m_dataMemory->preprocess(m_preprocess))
		{
			EXPECT_EQ(0,1);
		}

		m_graph_processing.loadGraph(m_graph_config, m_config, m_block);

		#ifdef DEBUG_GRAPH
		Logger->debug("m_dataMemory->getSize():{}", m_dataMemory->getSize());
		#endif
		for (int iteration = 0; iteration < m_dataMemory->getSize(); iteration++)
		{
			cv::Mat input = m_dataMemory->input(iteration).clone();
			m_data.clear();
			m_outputData.clear();
			for (int i = 0; i < m_graph_config.size(); i++)
			{
				std::vector<_data> dataVec;
				const QJsonObject _obj = m_graph_config[i].toObject();
				const QJsonArray _prevActive = _obj[PREV].toArray();
				const QJsonArray _nextActive = _obj[NEXT].toArray();

				if (m_graph_processing.checkIfLoadInputs(_prevActive, dataVec, input))
				{
					m_graph_processing.loadInputs(_prevActive, dataVec, m_graph_config, m_data);	
				}

				try
				{
					#ifdef DEBUG_GRAPH
					Logger->debug("Case::process() graph[{}] Processing: block[{}]->process", iteration, i);
					#endif
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
			}
			#ifdef DEBUG_GRAPH
			for (int z = 0; z < m_data.size(); z++)
			{
				for (int zz = 0; zz < m_data[z].size(); zz++)
				{
					Logger->debug("pre [{}][{}].():{}", z, zz, m_data[z][zz].processing.cols);
					Logger->debug("pre [{}][{}].():{}", z, zz, m_data[z][zz].testStr.toStdString());
				}
			}
			#endif
		}
	}

	TEST_F(GTest_graph, test_load_graph_post_processing)
	{
		Graph<PostProcess, _postData> m_graph_processing;
		std::vector<PostProcess*> m_block;
		std::vector<std::vector<_postData>> m_data;
		std::vector<cv::Mat> m_outputData;

		Logger->set_level(static_cast<spdlog::level::level_enum>(1));
		
		QJsonArray m_graph_config = GTest_graph::readArray(TEST_GRAPH_POSTPROCESSING);
		QJsonArray m_config = GTest_graph::readArray(TEST_GRAPH_CONFIG_POSTPROCESSING);
		QJsonArray m_preprocess = GTest_graph::readArray(TEST_PREPROCESS);
		QJsonObject m_dataset = GTest_graph::readConfig(TEST_DATASET);

		DataMemory* m_dataMemory = new DataMemory();
		m_dataMemory->configure(m_dataset);
		if(!m_dataMemory->preprocess(m_preprocess))
		{
			EXPECT_EQ(0,1);
		}
		m_graph_processing.loadGraph(m_graph_config, m_config, m_block);
		#ifdef DEBUG_GRAPH
		Logger->debug("m_dataMemory->getSize():{}", m_dataMemory->getSize());
		#endif
		for (int iteration = 0; iteration <  m_dataMemory->getSize(); iteration++)
		{
			cv::Mat input = m_dataMemory->input(iteration).clone();
			cv::Mat gt = m_dataMemory->gt(iteration).clone();
			cv::Mat input2 = m_dataMemory->input(iteration).clone();
			cv::Mat gt2 = m_dataMemory->gt(iteration).clone();
			std::vector<cv::Mat> inputMatrix{input, gt, input2};
			m_data.clear();
			m_outputData.clear();
			for (int i = 0; i < m_graph_config.size(); i++)
			{
				std::vector<_postData> dataVec;
				const QJsonObject _obj = m_graph_config[i].toObject();
				const QJsonArray _prevActive = _obj[PREV].toArray();
				const QJsonArray _nextActive = _obj[NEXT].toArray();

				if (m_graph_processing.checkIfLoadInputs(_prevActive, dataVec, inputMatrix, i))
				{
					m_graph_processing.loadInputs(_prevActive, dataVec, m_graph_config, m_data);	
				}
				try
				{
					#ifdef DEBUG_GRAPH
					Logger->debug("Case::process() graph[{}] Processing: block[{}]->process", iteration, i);
					#endif
					
					m_block[i]->process((dataVec));
				}
				catch (cv::Exception& e)
				{
					const char* err_msg = e.what();
					qDebug() << "exception caught: " << err_msg;
				}

				m_data.push_back((dataVec));
				#ifdef DEBUG_GRAPH
				for (int zz = 0; zz < m_data[i].size(); zz++)
				{
					Logger->debug("[{}][{}].():{}", i, zz, m_data[i][zz].processing.cols);
					Logger->debug("[{}][{}].():{}", i, zz, m_data[i][zz].testStr.toStdString());
				}
				#endif
				if (m_graph_processing.checkIfReturnData(_nextActive))
				{
					m_graph_processing.returnData(i, m_outputData, m_data);
				}
				dataVec.clear();
			}
			#ifdef DEBUG_GRAPH
			for (int z = 0; z < m_data.size(); z++)
			{
				for (int zz = 0; zz < m_data[z].size(); zz++)
				{
					Logger->debug("pre [{}][{}].():{}", z, zz, m_data[z][zz].processing.cols);
					Logger->debug("pre [{}][{}].():{}", z, zz, m_data[z][zz].testStr.toStdString());
				}
			}
			#endif
		}
		struct fitness fs;
		for (int i = 0; i < m_graph_config.size(); i++)
		{
			const QJsonObject _obj = m_graph_config[i].toObject();
			QString _type = _obj[TYPE].toString();
			if (_type == "Fitness")
			{
				#ifdef DEBUG_GRAPH
				Logger->debug("Case::process() Calculate Fitness endProcess:");
				#endif
				m_block[i]->endProcess(m_data[i]);
				fs = m_data[i][0].fs;
				#ifdef DEBUG_GRAPH
				Logger->debug("Case::process() fs:{}", fs.fitness);
				#endif
			}
			if (_type == "Encoder")
			{
				#ifdef DEBUG_GRAPH
				Logger->debug("Case::process() Calculate Encoder endProcess:");
				#endif
				m_block[i]->endProcess(m_data[i]);
				//fs = m_data[i][0].fs;
			}
		}

	}

	TEST_F(GTest_graph, test_integration_test)
	{
		int w = 50;
		int h = 50;
		int dronSize = 5;
		double m_res = (w * h - dronSize) / dronSize;

		cv::Mat input = cv::Mat(w, h, CV_8UC1, cv::Scalar(0));
		cv::Mat gt = cv::Mat(w, h, CV_8UC1, cv::Scalar(0));
		input.at<unsigned char>(cv::Point(10, 10)) = 255;
		gt.at<unsigned char>(cv::Point(10, 10)) = 255;

		imageErrors fitness = GTest_graph::checkBlock(input, gt, w, h, dronSize);
		Logger->debug("tpError :{}", fitness.tpError);
		Logger->debug("fpError :{}", fitness.fpError);
		Logger->debug("fnError :{}", fitness.fnError);
		Logger->debug("tnError :{}", fitness.tnError);
		EXPECT_EQ(fitness.tpError, m_res);
		EXPECT_EQ(fitness.fpError, 0);
		EXPECT_EQ(fitness.fnError, 0);
		EXPECT_EQ(fitness.tnError, 2499);

		cv::Mat input2 = cv::Mat(50, 50, CV_8UC1, cv::Scalar(0));
		cv::Mat gt2 = cv::Mat(50, 50, CV_8UC1, cv::Scalar(0));
		input2.at<unsigned char>(cv::Point(10, 10)) = 255;
		gt2.at<unsigned char>(cv::Point(20, 20)) = 255;

		imageErrors fitness2 = GTest_graph::checkBlock(input2, gt2, w, h, dronSize);
		Logger->debug("tpError :{}", fitness2.tpError);
		Logger->debug("fpError :{}", fitness2.fpError);
		Logger->debug("fnError :{}", fitness2.fnError);
		Logger->debug("tnError :{}", fitness2.tnError);
		EXPECT_EQ(fitness2.tpError, 0);
		EXPECT_EQ(fitness2.fpError, 1);
		EXPECT_EQ(fitness2.fnError, m_res);
		EXPECT_EQ(fitness2.tnError, 2498);
		Logger->debug("Case::process() graph[{}] Processing: block->process", 1);

		cv::Mat input3 = cv::Mat(50, 50, CV_8UC1, cv::Scalar(255));
		cv::Mat gt3 = cv::Mat(50, 50, CV_8UC1, cv::Scalar(0));
		gt3.at<unsigned char>(cv::Point(20, 20)) = 255;

		imageErrors fitness3 = GTest_graph::checkBlock(input3, gt3, w, h, dronSize);
		Logger->debug("tpError :{}", fitness3.tpError);
		Logger->debug("fpError :{}", fitness3.fpError);
		Logger->debug("fnError :{}", fitness3.fnError);
		Logger->debug("tnError :{}", fitness3.tnError);
		EXPECT_EQ(fitness3.tpError, m_res);
		EXPECT_EQ(fitness3.fpError, 0);
		EXPECT_EQ(fitness3.fnError, 1247001);
		EXPECT_EQ(fitness3.tnError, 0);
		Logger->debug("Case::process() graph[{}] Processing: block->process", 1);
	}

	imageErrors GTest_graph::checkBlock(cv::Mat &input, cv::Mat &gt, int w, int h, int dronSize)
	{
		QJsonObject obj{{"Name", "CodeStats2014"}, {"Width",w},{"Height",h}, {"DronSize",dronSize}};
		PostProcess* _block = PostProcess::make("Comapre");
		_block->configure(obj);
		QJsonObject obj2{{"Name", "BGFitness"}, {"FitnessFunction","Accuracy_Recall_Precision_FMeasure"}};
		PostProcess* _block2 = PostProcess::make("Fitness");
		_block2->configure(obj2);
		_postData data{input.clone()};
		_postData data2{gt.clone()};
		std::vector<_postData> dataVec{data, data2};
		_block->process(dataVec);
		_block->endProcess(dataVec);
		_block2->process(dataVec);
		_block2->endProcess(dataVec);
		return dataVec[0].ie;
	}

	QJsonObject GTest_graph::readConfig(QString name)
	{
		QString configName{ name };
		std::shared_ptr<ConfigReader> cR = std::make_shared<ConfigReader>();
		QJsonObject jObject;
		if (!cR->readConfig(configName, jObject))
		{
			Logger->error("File {} read confif failed", configName.toStdString());
			EXPECT_EQ(0,1);
		}
		qDebug() << name << ":" << jObject << "\n";
		return jObject;
	}

	QJsonArray GTest_graph::readArray(QString name)
	{
		QString configName{ name };
		std::shared_ptr<ConfigReader> cR = std::make_shared<ConfigReader>();
		QJsonArray jarray;
		if (!cR->readConfig(configName, jarray))
		{
			Logger->error("File {} read confif failed", configName.toStdString());
			EXPECT_EQ(0,1);
			
		}
		qDebug() << name << ":" << jarray <<  "\n";
		return jarray;
	}

}  // namespace gtest_configreader
