#include "genetic/case.h"
//#include <QPolygonF>

//#define DEBUG 
//#define DEBUG_CASE
//#define DEBUG_SINGLE_CASE

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

Case::Case(DataMemory* data)
	: m_dataMemory(data),
	m_firstTime{ true }
{
	connect(this, &Case::configureAndStartSlot, this, &Case::onConfigureAndStartSlot);
}

void Case::onUpdate() {}

void Case::configure(QJsonArray a_graph, QJsonArray a_config, QJsonArray a_postprocess)
{
	m_graph_config = a_graph;
	m_config = a_config;
	m_postprocess_config = a_postprocess;

	if (m_dataMemory->getSize() < 1) 
	{
		Logger->error("Case::onConfigure() m_data do not have any images!");
		return;
	}

	if (m_firstTime)
	{
		m_firstTime = false;
		if (m_graph_config.size() != a_config.size())
		{
			Logger->error("config not match with graph");
			return;
		}
		m_graph_processing.loadGraph(m_graph_config, m_config, m_block);
		m_graph_postprocessing.loadGraph(m_postprocess_config, m_blockPostprocess);
	}
	else // TODO: Dynamic graph creation:
	{
		for (int i = 0; i < m_graph_config.size(); i++)
		{
			m_block[i]->configure(a_config[i].toObject());
		}
		for (int i = 0; i < m_postprocess_config.size(); i++)
		{
			QJsonObject _obj = m_postprocess_config[i].toObject();
			m_blockPostprocess[i]->configure(_obj[CONFIG].toObject());
		}
	}
	#ifdef MEMORY_CHECK
	Logger->info("Case::onConfigure() m_blockPostprocess,size:{}", m_blockPostprocess.size());
	Logger->info("Case::onConfigure() m_block,size:{}", m_block.size());
	#endif
}


void Case::onConfigureAndStartSlot(QJsonArray a_graph, QJsonArray a_config, QJsonArray a_postprocess, int processSlot)
{
	configure(a_graph, a_config, a_postprocess);
	fitness fs = Case::process();
	emit(signalOk(fs, processSlot));
	Logger->trace("Case::onConfigureAndStart() done");
}

fitness Case::onConfigureAndStart(QJsonArray a_graph, QJsonArray a_config, QJsonArray a_postprocess)
{
	#ifdef DEBUG_SINGLE_CASE
		Logger->debug("Case::onConfigureAndStart()");
	#endif
	configure(a_graph, a_config, a_postprocess);
	fitness fs = Case::process();
	#ifdef DEBUG_SINGLE_CASE
		qDebug() << "fitness.fitness:" << fs.fitness;
	#endif
	return fs;
}

void Case::clearDataForNextIteration()
{
	Logger->trace("Case::clearDataForNextIteration()");
	m_data.clear();
	m_outputData.clear();
	//m_outputDataVector.clear();
}

fitness Case::process()
{
	#ifdef DEBUG_CASE
		Logger->debug("Case::onConfigureAndStart()");
		Logger->debug("Case::process() start processing {} frames", m_dataMemory->getSize());
		Logger->debug("Case::process() m_block,size:{}", m_block.size());
	#endif

	
	double time = 0;
	for (int iteration = 0; iteration < m_dataMemory->getSize(); iteration++)
	{
		Case::clearDataForNextIteration();
		cv::Mat input = m_dataMemory->input(iteration).clone();
		Logger->trace("Case::process() iteration:{}", iteration);

		#ifdef DEBUG_CASE
			Logger->debug("Case::process() graph[{}] Processing:", iteration);
		#endif
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
				#ifdef DEBUG_CASE
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
		
			time += m_block[i]->getElapsedTime();
			dataVec.clear();
		}
		#ifdef DEBUG_CASE
			Logger->debug("Case::process() graph[{}] Intephase:", iteration);
		#endif
		
		//m_outputData.clear();
		cv::Mat gt = m_dataMemory->gt(iteration).clone();
		//m_outputData.push_back(gt.clone());
		m_outputData.push_back(gt.clone());
		cv::Mat inputImage = m_dataMemory->input(iteration).clone();
		m_outputData.push_back(inputImage.clone());
		#ifdef DEBUG_CASE
			Logger->debug("Case::process() m_outputData.size():{}", m_outputData.size());
			if(m_outputData.size() != 2 )
			{
				Logger->error("Case::process() m_outputData.size():{}", m_outputData.size());
			}
		#endif
		
		#ifdef DEBUG
		cv::imshow("output", m_outputData[0]);
		cv::imshow("gt-case", m_outputData[1]);
		cv::waitKey(1);
		#endif

		if (iteration>50)
		{
			// POSTPROCESSING:
			m_dataPostprocess.clear();

			#ifdef DEBUG_CASE
				Logger->debug("Case::process() graph[{}] PostProcessing:", iteration);
			#endif
			for (int i = 0; i < m_postprocess_config.size(); i++)
			{
				//qDebug() << "loop1 : m_dataPostprocess["<< i << "].testStr:" << m_dataPostprocess[i].testStr;
				std::vector<_postData> dataVec;
				const QJsonObject _obj = m_postprocess_config[i].toObject();
				const QJsonArray _prevActive = _obj[PREV].toArray();
				const QJsonArray _nextActive = _obj[NEXT].toArray();
				bool _flagNotStart = true;
				bool _flagReturnData = false;

				if (m_graph_postprocessing.checkIfLoadInputs(_prevActive, dataVec, m_outputData, i))
				{
					m_graph_postprocessing.loadInputs(_prevActive, dataVec, m_graph_config, m_dataPostprocess);				
				}

				try
				{
					#ifdef DEBUG_CASE
					Logger->debug("Case::process() graph[{}] postProcessing: block[{}]->process", iteration, i);
					#endif
					m_blockPostprocess[i]->process((dataVec));
				}
				catch (cv::Exception& e)
				{
					const char* err_msg = e.what();
					qDebug() << "exception caught: " << err_msg;
				}
				m_dataPostprocess.push_back((dataVec));
			}
		}
	}
	#ifdef DEBUG_CASE
	for (int z = 0; z < m_dataPostprocess.size(); z++)
	{
		for (int zz = 0; zz < m_dataPostprocess[z].size(); zz++)
		{
			Logger->debug("pre [{}][{}].():{}", z, zz, m_dataPostprocess[z][zz].processing.cols);
		}

	}
	#endif
	#ifdef DEBUG_CASE
	Logger->debug("Case::process() Calculate fitness:");
	#endif
	struct fitness fs;
	for (int i = 0; i < m_postprocess_config.size(); i++)
	{
		const QJsonObject _obj = m_postprocess_config[i].toObject();
		QString _type = _obj[TYPE].toString();
		if (_type == "Fitness")
		{
			#ifdef DEBUG_CASE
			Logger->debug("Case::process() Calculate Fitness endProcess:");
			#endif
			m_blockPostprocess[i]->endProcess(m_dataPostprocess[i]);
			fs = m_dataPostprocess[i][0].fs;
			#ifdef DEBUG_CASE
			Logger->debug("Case::process() fs:{}", fs.fitness);
			#endif
		}
		if (_type == "Encoder")
		{
			#ifdef DEBUG_CASE
			Logger->debug("Case::process() Calculate Encoder endProcess:");
			#endif
			m_blockPostprocess[i]->endProcess(m_dataPostprocess[i]);
			//fs = m_dataPostprocess[i][0].fs;
		}
	}
	#ifdef DEBUG_CASE
	for (int z = 0; z < m_dataPostprocess.size(); z++)
	{
		for (int zz = 0; zz < m_dataPostprocess[z].size(); zz++)
		{
			Logger->debug("m_dataPostprocess [{}][{}].():{}", z, zz, m_dataPostprocess[z][zz].processing.cols);
			Logger->debug("m_dataPostprocess [{}][{}].():{}", z, zz, m_dataPostprocess[z][zz].testStr.toStdString());
		}

	}
	#endif
	#ifdef DEBUG_CASE
	Logger->debug("Case::process() deletes:");
	#endif
	for (int i = 0; i < m_block.size(); i++)
	{
		//m_block[i]->deleteLater();
		//delete m_block[i];
	}
	for (int i = 0; i < m_blockPostprocess.size(); i++)
	{
		//m_block[i]->deleteLater();
		//delete m_blockPostprocess[i];
	}
	/*
	m_data.clear();
	m_outputData.clear();
	m_outputDataVector.clear();*/

	#ifdef DEBUG__CASE
		qDebug() << "fitness.fitness:" << fs.fitness;
	#endif

	return fs;
}
