#include "genetic/mainloop.h"

constexpr auto GENERAL{ "General" };
constexpr auto GENETIC{ "Genetic" };
constexpr auto GRAPH_TYPE{ "GraphType" };
constexpr auto BOUNDS_TYPE{ "BoundsType" };
constexpr auto CONFIG{ "Config" };
constexpr auto NAME{ "Name" };
constexpr auto STANDARD_DEVIATION{ "StandardDeviation" };
constexpr auto DRON_NOISE{ "Noise" };
constexpr auto DRON_CONTRAST{ "Contrast" };
constexpr auto DRON_TYPE{ "DronType" };

#define DEBUG


void MainLoop::readConfig(QString configName, QJsonObject& jObject, QString graphType)
{
	ConfigReader* configReader = new ConfigReader();
	QJsonObject _jObject{};
	if (!configReader->readConfig(configName, _jObject))
	{
		Logger->error("MainLoop::readConfig() Open {} failed", configName.toStdString());
	}
	jObject = _jObject[graphType].toObject();
	
	delete configReader;
}

void MainLoop::readConfig(QString configName, QJsonArray& jArray, QString graphType)
{
	ConfigReader* configReader = new ConfigReader();
	QJsonObject _jObject{};
	if (!configReader->readConfig(configName, _jObject))
	{
		Logger->error("MainLoop::readConfig() Open {} failed", configName.toStdString());
	}
	jArray = _jObject[graphType].toArray();
	delete configReader;
}

void MainLoop::loadConfigs(QJsonObject configPaths, QString graphType, QString boundsType)
{
    Logger->trace("MainLoop::loadConfigs()");

	m_geneticConfigName.dataset = configPaths["Dataset"].toString();
	m_geneticConfigName.graph = configPaths["Graph"].toString();
	m_geneticConfigName.bounds = configPaths["Bounds"].toString();
	m_geneticConfigName.preprocess = configPaths["Preprocess"].toString();
	m_geneticConfigName.postprocess = configPaths["Postprocess"].toString();

	MainLoop::readConfig(m_geneticConfigName.dataset, m_geneticConfig.dataset, graphType);
	MainLoop::readConfig(m_geneticConfigName.graph, m_geneticConfig.graph, graphType);
	MainLoop::readConfig(m_geneticConfigName.bounds, m_geneticConfig.bounds, graphType);
	MainLoop::readConfig(m_geneticConfigName.preprocess, m_geneticConfig.preprocess, graphType);
	MainLoop::readConfig(m_geneticConfigName.postprocess, m_geneticConfig.postprocess, graphType);

	m_geneticConfig.bounds = m_geneticConfig.bounds[boundsType].toObject();

	#ifdef DEBUG
		qDebug() << "MainLoop::loadConfigs() m_geneticConfig.dataset:" << m_geneticConfig.dataset;
		qDebug() << "MainLoop::loadConfigs() m_geneticConfig.bounds:" << m_geneticConfig.bounds;
	#endif
}

void MainLoop::createConfig(QJsonObject const& a_config)
{
	Logger->trace("MainLoop::createConfig()");
    #ifdef _WIN32
    QJsonObject configPaths = a_config["ConfigWin"].toObject();
    #endif // _WIN32
    #ifdef __linux__
    QJsonObject configPaths = a_config["ConfigUnix"].toObject();
    #endif // __linux__
	#ifdef DEBUG
		qDebug() << "MainLoop::createConfig(a_config) a_config:" << a_config;
	#endif
	
	std::vector<QString> grafConfigs{"Graph_estimator_with_filters"};
	//std::vector<QString> grafConfigs{"Graph_estimator_with_filters", "Graph_estimator"};
	//std::vector<QString> dronConfigs{"BLACK", "BLACK_WHITE",  "WHITE"};
	std::vector<QString> dronConfigs{  "WHITE"};
	//std::vector<QString> boundConfigs{"ViBe", "MOG2", "CNT", "NONE", "MOG", "KNN", "GMG"};

	//std::vector<QString> boundConfigs{"ViBe"};
	//std::vector<QString> boundConfigs{ "KNN", "MOG"};
	std::vector<QString> boundConfigs{"MOG2", "CNT", "NONE", "MOG", "KNN", "GMG"};

	for (int graf = 0 ; graf < grafConfigs.size() ; graf++)
	{
		for (int dron = 0 ; dron < dronConfigs.size() ; dron++)
		{
			for (int bounds = 0 ; bounds < boundConfigs.size() ; bounds++)
			{
				QJsonObject obj = m_config[GENETIC].toObject();
				obj[BOUNDS_TYPE] = boundConfigs[bounds];
				obj[DRON_TYPE] = dronConfigs[dron];
				obj[GRAPH_TYPE] = grafConfigs[graf];
				//qDebug() << "boundConfigs:" <<boundConfigs[bounds]; 
				qDebug() << "genetic:" << obj; 
				m_config[GENETIC] = obj;
				
				m_geneticConfig.config = m_config;
				
				MainLoop::loadConfigs(configPaths, m_graphType, boundConfigs[bounds]);

				//fill dron config:

				for (int i = 0; i < 101; i += 10)
				{
					for(int j = 0 ; j < m_geneticConfig.preprocess.size() ; j++)
					{
						if(m_geneticConfig.preprocess[j].toObject()[CONFIG].toObject()[NAME].toString() == "AddMultipleDron")
						{
							QJsonArray arrObj = m_geneticConfig.preprocess;
							QJsonObject obj = arrObj[j].toObject();
							QJsonObject config = obj[CONFIG].toObject();

							config[DRON_NOISE] = i;
							config[DRON_CONTRAST] = 100;
							config[DRON_TYPE] = dronConfigs[dron];

							obj[CONFIG] = config;
							arrObj[j] = obj;
							m_geneticConfig.preprocess = arrObj;
							qDebug() << "config[DRON_NOISE]:" << config[DRON_NOISE];
							qDebug() << "config[DRON_CONTRAST]:" << config[DRON_CONTRAST];
							qDebug() << "config[DRON_TYPE]:" << config[DRON_TYPE];
						}
					}
					m_geneticConfigs.push_back(m_geneticConfig);
				}
			}
		}
	}
	Logger->info("MainLoop::createConfig() createConfig() m_geneticConfigs.size():{}", m_geneticConfigs.size());
}

void MainLoop::createStartupThreads()
{
    Logger->trace("MainLoop::createStartupThreads()");
	m_dataMemoryThread = new QThread();
	m_dataMemory = new DataMemory();
	connect(m_dataMemory, &DataMemory::memoryLoaded, this, &MainLoop::onMemoryLoaded);
	m_dataMemory->moveToThread(m_dataMemoryThread);
	connect(m_dataMemoryThread, &QThread::finished, m_dataMemory, &QObject::deleteLater);
	m_dataMemoryThread->start();

	for (int i = 0; i < m_threadsMax; i++)
    {
		m_threadsVector.push_back(new QThread());
		m_caseVector.push_back(new Case(m_dataMemory));
	}

	for (int i = 0; i < m_threadsMax; i++)
    {
		m_caseVector[i]->moveToThread(m_threadsVector[i]);
		m_threadsVector[i]->start();
		connect(m_threadsVector[i], &QThread::finished, m_caseVector[i], &QObject::deleteLater);
	}
	
	m_geneticThread = new QThread();
	m_genetic = new Genetic(m_caseVector, m_dataMemory);
	m_genetic->moveToThread(m_geneticThread);
	connect(m_geneticThread, &QThread::finished, m_genetic, &QObject::deleteLater);
	m_geneticThread->start();
	connect(m_genetic, &Genetic::geneticConfigured, this, &MainLoop::onGeneticConfigured);
	for (int i = 0; i < m_threadsMax; i++)
    {
		connect(m_caseVector[i], &Case::signalOk, m_genetic, &Genetic::onSignalOk);
	}


	m_fileLoggerThread = new QThread();
	m_fileLogger = new FileLogger() ;
	m_fileLogger->moveToThread(m_fileLoggerThread);
	connect(m_fileLoggerThread, &QThread::finished, m_fileLogger, &QObject::deleteLater);
	m_fileLoggerThread->start();

	m_fileLoggerJSONThread = new QThread();
	m_fileLoggerJSON = new FileLogger();
	m_fileLoggerJSON->moveToThread(m_fileLoggerJSONThread);
	connect(m_fileLoggerJSONThread, &QThread::finished, m_fileLoggerJSON, &QObject::deleteLater);
	m_fileLoggerJSONThread->start();

	connect(m_genetic, &Genetic::appendToFileLogger, m_fileLogger, &FileLogger::appendFileLogger);
	connect(m_genetic, &Genetic::logJsonBest, m_fileLoggerJSON, &FileLogger::logJsonBest);

	connect(m_genetic, &Genetic::configureLogger, m_fileLogger, &FileLogger::configure);
	connect(m_genetic, &Genetic::configureLoggerJSON, m_fileLoggerJSON, &FileLogger::configure);

	m_timer = new QTimer(this);
	m_timer->start(1000);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onUpdate()));
	connect(m_genetic, &Genetic::newConfig, this, &MainLoop::onNextConfig);
}

void MainLoop::createThreads()
{ }

MainLoop::MainLoop(QJsonObject a_config)
	:m_config{ a_config },
	m_threadsMax{ a_config[GENERAL].toObject()[THREADSMAX].toInt() },
	m_dataMemoryLoaded{false},
	m_geneticConfigured{ false },
	m_register{false},
	m_recvTask{false},
	m_validTask{false},
	m_firstTime{true},
	m_iterationGlobal{ 0 },
	m_geneticRun{ true },
	m_graphType{ a_config[GENETIC].toObject()[GRAPH_TYPE].toString()},
	m_boundsType{ a_config[GENETIC].toObject()[BOUNDS_TYPE].toString()}
{
	#ifdef DEBUG
		qDebug() << "MainLoop::MainLoop() a_config:" << a_config;
		Logger->debug("MainLoop::MainLoop() m_threadsMax:{}", m_threadsMax);
		Logger->debug("MainLoop::MainLoop() m_graphType:{}", m_graphType.toStdString());
		Logger->debug("MainLoop::MainLoop() m_boundsType:{}", m_boundsType.toStdString());
	#endif
	
	MainLoop::createStartupThreads();
}

void MainLoop::onCase(QJsonObject json)
{
}

void MainLoop::on_broadcast()
{
}

void MainLoop::on_task(QJsonObject json)
{
}


void MainLoop::on_register(QJsonObject json)
{
}


void MainLoop::onUpdate()
{
	if (m_firstTime)
	{
        Logger->trace("MainLoop::onUpdate() firstTime");
		m_firstTime = false;
		createConfig(m_config);
		m_validTask = true;
		if (m_geneticConfigs.size() > 0)
		{
			m_dataMemory->loadData(m_geneticConfigs[0].dataset);
		}
	}

	if (m_validTask)
	{
        Logger->trace("MainLoop::onUpdate() m_validTask");
		if (m_geneticConfigs.size() > 0)
		{
			m_dataMemory->preprocess(m_geneticConfigs[0].preprocess);
            
			m_genetic->configure(m_geneticConfigs[0].config , m_geneticConfigs[0].bounds, m_geneticConfigs[0].graph, m_geneticConfigs[0].postprocess,
								 m_geneticConfigs[0].preprocess, m_iterationGlobal);
			m_iterationGlobal++;
			m_validTask = false;
		}
	}
	if (m_dataMemoryLoaded && m_geneticConfigured)
	{
		Logger->trace("MainLoop: emit process to genetic");
		m_genetic->process();
	}
}

void MainLoop::onNextConfig()
{
	qDebug() << "MainLoop::onNextConfig():"<< m_geneticConfigs.size();
	m_dataMemoryLoaded = false;
	m_geneticConfigured = false;
	if (m_geneticConfigs.size() > 0)
	{
		qDebug() << "m_geneticConfigs.size() > 0:"<< m_geneticConfigs.size() ;
		m_geneticConfigs.pop_front();
		m_validTask = true;
	}
	else
	{
		qDebug() << "MainLoop::exit!!():"<< m_geneticConfigs.size();
	}

	if (m_geneticConfigs.size() == 0)
	{
		qDebug() << "No config left!:" << m_geneticConfigs.size();
		createConfig(m_config);
	}
}

void MainLoop::onQuit()
{
	qDebug() << "onQuit:";
	Logger->error("MainLoop::onQuit()");
	emit(quit());
}

void MainLoop::onMemoryLoaded()
{
	Logger->info("MainLoop::onMemoryLoaded()");
	m_dataMemoryLoaded = true;
}

void MainLoop::onGeneticConfigured()
{
	Logger->info("MainLoop::onGeneticConfigured()");
	m_geneticConfigured = true;
}

void MainLoop::configure(QJsonObject const& a_config)
{
	Logger->info("MainLoop::configure()");
}
