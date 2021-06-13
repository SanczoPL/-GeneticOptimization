#include "genetic/geneticoperation.h"
#include <QDebug>

constexpr auto GRAPH{ "Graph" };
constexpr auto GENETIC{ "Genetic" };
constexpr auto POPULATION_SIZE{ "PopulationSize" };
constexpr auto RESULTS_PATH{"ResultsPath"};


#define DEBUG
//#define GENETIC_OPERATION_DEBUG

GeneticOperation::~GeneticOperation()
{
	Logger->info(" GeneticOperation::~GeneticOperation()");
}

GeneticOperation::GeneticOperation()
: m_randomGenerator{ new QRandomGenerator(123)}
{

}

void GeneticOperation::configure(QJsonObject const& a_config, QJsonObject  const& a_boundsGraph, QJsonArray  const& a_graph)
{
    m_optimizationTypes.clear();
    m_boundsGraph = a_boundsGraph;

    m_populationSize = a_config[GENETIC].toObject()[POPULATION_SIZE].toInt();

    for (int i = 0; i < a_graph.size(); i++)
	{
		QJsonObject obj = a_graph[i].toObject();
		m_optimizationTypes.push_back(obj["Type"].toString());
	}
    
    m_vectorBits = createRandomProcessingPopulation(m_populationSize, m_optimizationTypes, m_boundsGraph);
    #ifdef DEBUG
        qDebug() << "vectorBits.size()" << m_vectorBits.size();
    #endif
}

void GeneticOperation::mutate() 
{
	Logger->trace("GeneticOperation::mutate() ");
	for (qint32 man = 0; man < m_populationSize; man++)
    {
		double y = m_randomGenerator->bounded(0, 100) / 100.0;
		if (y < 0.2) {
			Logger->trace("mutate: change:{} man", man);
			m_vectorBits[man] = createRandomProcessing(m_optimizationTypes, m_boundsGraph);
		}
	}
}

void GeneticOperation::mutate(int men) 
{
	#ifdef GENETIC_OPERATION_DEBUG
	Logger->info("mutate: change:{} man", men);
	#endif
	m_vectorBits[men] = createRandomProcessing(m_optimizationTypes, m_boundsGraph);
}

void GeneticOperation::crossover()
{
#ifdef GENETIC_OPERATION_DEBUG
	Logger->trace("crossover");
#endif
	// double x;
	for (qint32 man1 = 0; man1 < m_populationSize; man1++)
    {
		for (qint32 man2 = 0; man2 < m_populationSize; man2++)
        {
			if (man1 != man2) {
				double x = m_randomGenerator->bounded(0, 10) / 10.0;
				Logger->trace("m_randomGenerator GeneticOperation::crossover() x:{}", x);
				if (x < 0.5) {
					xOver(man1, man2);
					break;
				}
			}
		}
	}
	Logger->trace("crossover done");
}

void GeneticOperation::crossover(int men)
{
#ifdef GENETIC_OPERATION_DEBUG
	Logger->info("crossover");
#endif
	for (qint32 man2 = m_populationSize; man2 >= 0; man2--)
	{
		if (men != man2) {
			double x = m_randomGenerator->bounded(0, 10) / 10.0;
			Logger->trace("m_randomGenerator GeneticOperation::crossover() x:{}", x);
			if (x < 0.5) {
				xOver(men, man2);
				break;
			}
		}
	}
	Logger->trace("crossover done");
}

void GeneticOperation::gradient()
{
	Logger->trace("gradient");
	for (qint32 man = 0; man < m_vectorBits.size(); man++)
	{
		double y = m_randomGenerator->bounded(0, 100) / 100.0;
		if (y < 0.5) {
			Logger->trace("gradient: change:{} man", man);
			//m_vectorBits[man] = createRandomProcessing(m_optimizationTypes, m_boundsGraph);
			qint32 sizeOfBit = m_vectorBits[man].size();
			int prob = m_randomGenerator->bounded(0, sizeOfBit);
			//m_optimizationTypes
			Logger->trace("prob:{}, max:{}", prob, m_vectorBits[man].size());
			QString filterType = m_vectorBits[man][prob].toObject()["Type"].toString(); // Filter
			QJsonObject config = m_vectorBits[man][prob].toObject()["Config"].toObject();
			QString filterName = config["Name"].toString();// Threshold
			for(int opt = 0; opt < m_optimizationTypes.size(); opt++)
			{
				if (m_optimizationTypes[opt] == filterType)
				{
					Logger->trace("gradient change filter type:{}", filterType.toStdString());
					QJsonObject dataObject = m_boundsGraph[m_optimizationTypes[opt]].toObject();
					QJsonArray dataUsed = dataObject["Used"].toArray();
					for (int par = 0; par < dataUsed.size(); par++)
					{

						if (dataUsed[par].toObject()["Name"].toString() == filterName) // if  Threshold == Threshold
						{
							QJsonArray bounds = dataUsed[par].toObject()["Parameters"].toArray();
							int probParam = m_randomGenerator->bounded(0, bounds.size() + 1);
							QString parameter =  bounds[probParam].toObject()["Type"].toString();
							//QJsonObject parameterObj = gradientOnConfig(bounds[probParam].toObject());
						}
					}
				}
			}
		}
	}
	Logger->trace("gradient done");
}


bool GeneticOperation::gradient(int men) {
#ifdef GENETIC_OPERATION_DEBUG
	Logger->info("gradient: change:{} man", men);
#endif
	
	qint32 sizeOfBit = m_vectorBits[men].size();
	//int prob = m_randomGenerator->bounded(0, sizeOfBit);


	// TO co jest w caonfigu:
	QString filterType = m_vectorBits[men][0].toObject()["Type"].toString(); // Filter
	QJsonObject config = m_vectorBits[men][0].toObject();
	QString filterName = config["Name"].toString();// Threshold
#ifdef GENETIC_OPERATION_DEBUG
/*
	qDebug() << "m_vectorBits[men]:" << m_vectorBits[men];
	qDebug() << "m_vectorBits[men][0].toObject():" << m_vectorBits[men][0].toObject();
	qDebug() << "config:" << config;*/

#endif

	for (int opt = 0; opt < m_optimizationTypes.size(); opt++)
	{
		if (m_optimizationTypes[opt] == filterType)
		{
#ifdef GENETIC_OPERATION_DEBUG
			Logger->info("gradient change filter type:{}", filterType.toStdString());
#endif
			
			QJsonObject dataObject = m_boundsGraph[m_optimizationTypes[opt]].toObject();
			QJsonArray dataUsed = dataObject["Used"].toArray();
#ifdef GENETIC_OPERATION_DEBUG
			Logger->info("dataUsed:{}", dataUsed.size());
#endif

			for (int par = 0; par < dataUsed.size(); par++)
			{
#ifdef GENETIC_OPERATION_DEBUG
				Logger->info("check dataUsed[par].toObject()[Name].toString():{}", dataUsed[par].toObject()["Name"].toString().toStdString());
				Logger->info("filterName:{}", filterName.toStdString());
#endif
				if (dataUsed[par].toObject()["Name"].toString() == filterName) // if  Threshold == Threshold
				{
#ifdef GENETIC_OPERATION_DEBUG
					Logger->info("name == name");
					/*
					qDebug() << "dataUsed[par].toObject():" << dataUsed[par].toObject();
					qDebug() << "dataUsed[par].toObject()[Parameters].toArray():" << dataUsed[par].toObject()["Parameters"].toArray();
*/
#endif
					QJsonArray bounds = dataUsed[par].toObject()["Parameters"].toArray();
					int probParam = m_randomGenerator->bounded(0, bounds.size() );
#ifdef GENETIC_OPERATION_DEBUG
					Logger->info("probParam:{}", probParam);
#endif
					QString parameter = bounds[probParam].toObject()["Type"].toString(); // KernelSizeX
#ifdef GENETIC_OPERATION_DEBUG
					Logger->info("parameter:{}", parameter.toStdString());
#endif
					//QJsonObject parameterObj = gradientOnConfig(bounds[probParam].toObject(), parameter, config);
					if (gradientOnConfig(bounds[probParam].toObject(), config))
					{
#ifdef GENETIC_OPERATION_DEBUG
						Logger->info("gradientOnConfig return true");
#endif					
						return true;
						
						//qDebug() << "parameterObj:" << parameterObj;
					}
					else {
#ifdef GENETIC_OPERATION_DEBUG
						Logger->info("gradientOnConfig return false");
#endif	
						return false;
					}
				}
			}
		}
	}
	Logger->trace("gradient done");
}


bool  GeneticOperation::gradientOnConfig(QJsonObject bounds, QJsonObject config)
{
	int min = bounds["Min"].toInt();
	int max = bounds["Max"].toInt();
	int isDouble = bounds["IsDouble"].toInt();
	QString parameter = bounds["Type"].toString();
	int value{ 1 };
	//QJsonObject parameterObj;
	if (bounds["IsBool"].toBool() == true)
	{
#ifdef GENETIC_OPERATION_DEBUG
		Logger->info("Gradient initial bool:{}", config[parameter].toBool());
#endif
		bool actualData = config[parameter].toBool();
		//qDebug() << "IsBool!!";
		//bool boolValue = m_randomGenerator->bounded(0, 1);
		bool boolValue = !actualData;
		//parameterObj.insert(bounds["Type"].toString(), boolValue);
		config[parameter] = boolValue;
#ifdef GENETIC_OPERATION_DEBUG
		Logger->info("Gradient change bool:{}", config[parameter].toBool());
#endif
		return true;
	}
	else if (isDouble > 0)
	{
#ifdef GENETIC_OPERATION_DEBUG
		Logger->info("Gradient initial double:{}", int(config[parameter].toDouble() * isDouble));
#endif
		int actualData = int(config[parameter].toDouble() * isDouble);
		int minValue{ 1 };
		if (min == 0)
		{
			minValue = 1;
		}
		else {
			minValue = double(min / isDouble);
		}
		double doubleValue = actualData;
		double y = m_randomGenerator->bounded(0, 10) / 10.0;
		if (y < 0.5) {
			doubleValue = actualData + minValue;
		}
		else {
			doubleValue = actualData - minValue;
		}
		if (doubleValue > max)
		{
			if (doubleValue <min)
			{
#ifdef GENETIC_OPERATION_DEBUG
				Logger->error("Gradient Min Max error");
				return false;
#endif
			}
			else {
				//parameterObj.insert(bounds["Type"].toString(), doubleValueMinus);
				config[parameter] = doubleValue;
#ifdef GENETIC_OPERATION_DEBUG
				Logger->info("Gradient doubleValueMinus change to double:{}", int(config[parameter].toDouble() * isDouble));
#endif
				return true;
			}
		}
		else
		{
			//parameterObj.insert(bounds["Type"].toString(), doubleValuePlus);
			config[parameter] = doubleValue;
#ifdef GENETIC_OPERATION_DEBUG
			Logger->info("Gradient doubleValuePlus change to double:{}", int(config[parameter].toDouble() * isDouble));
#endif
			return true;
		}
	}
	else if (bounds["IsOdd"].toBool() == true)
	{
#ifdef GENETIC_OPERATION_DEBUG
		Logger->info("Gradient initial IsOdd:{}", config[parameter].toInt());
#endif
		//value = m_randomGenerator->bounded(bounds["Min"].toInt(), bounds["Max"].toInt() + 1);
		value = config[parameter].toInt();
		double y = m_randomGenerator->bounded(0, 10) / 10.0;
		if (y < 0.5) {
			value++;
		}
		else {
			value--;
		}
		if (value % 2 == 0)
		{
			double y = m_randomGenerator->bounded(0, 10) / 10.0;
			if (y < 0.5) {
				value++;
			}
			else {
				value--;
			}
		}
		
	}
	else
	{
#ifdef GENETIC_OPERATION_DEBUG
		Logger->info("Gradient initial normal operation:{}", config[parameter].toInt());
#endif
		//value = m_randomGenerator->bounded(bounds["Min"].toInt(), bounds["Max"].toInt() + 1);
		value = config[parameter].toInt();
		double y = m_randomGenerator->bounded(0, 10) / 10.0;
		if (y < 0.5) {
			value++;
		}
		else {
			value--;
		}
		
	}
#ifdef GENETIC_OPERATION_DEBUG
	Logger->info("Gradient try to change normal operation:{}", value);
#endif
	if (value < min && value > max)
	{
		return false;
	}
	config[parameter] = value;
#ifdef GENETIC_OPERATION_DEBUG
	Logger->info("Gradient change to:{}", config[parameter].toInt());
#endif
	return true;
}

void GeneticOperation::xOver(qint32 one, qint32 two)
{
	int x = m_randomGenerator->bounded(0, m_vectorBits[one].size());

#ifdef GENETIC_OPERATION_DEBUG
	Logger->info("xOver {}:{}", one, two);

#endif
	QJsonArray tempArray1 = m_vectorBits[one];
	QJsonArray tempArray2 = m_vectorBits[two];

	//QJsonObject temp1 = tempArray1[x].toObject();
	//QJsonObject temp2 = tempArray2[x].toObject();

#ifdef GENETIC_OPERATION_DEBUG
/*
	qDebug() << "Try to change:" << tempArray1[x] << " into:" << tempArray2[x].toObject();*/
#endif

	tempArray1[x] = tempArray2[x].toObject();

	m_vectorBits[one] = tempArray1;
	//m_vectorBits[two].replace(x, m_vectorBits[one].at(x));

	Logger->trace("xOver done");
}


void GeneticOperation::elitist()
{
#ifdef GENETIC_OPERATION_DEBUG
	Logger->debug("elitist");
#endif
	double d_best = m_fitness[0].fitness;
	double d_worst = m_fitness[0].fitness;
	qint32 best_men = 0;
	qint32 worst_men = 0;
	for (qint32 man = 0; man < m_populationSize - 1; man++) {
		if (m_fitness[man].fitness > m_fitness[man + 1].fitness) {
			if (m_fitness[man].fitness >= d_best) {
				d_best = m_fitness[man].fitness;
				best_men = man;
			}
			if (m_fitness[man + 1].fitness <= d_worst) {
				d_worst = m_fitness[man + 1].fitness;
				worst_men = man + 1;
			}
		}
		else {
			if (m_fitness[man].fitness <= d_worst) {
				d_worst = m_fitness[man].fitness;
				worst_men = man;
			}
			if (m_fitness[man + 1].fitness >= d_best) {
				d_best = m_fitness[man + 1].fitness;
				best_men = man + 1;
			}
		}
	}

	if (d_best >= m_fitness[m_populationSize].fitness) {
		m_vectorBits[m_populationSize] = m_vectorBits[best_men];
		m_fitness[m_populationSize] = m_fitness[best_men];
	}
	else {
		m_vectorBits[worst_men] = m_vectorBits[m_populationSize];
		m_fitness[worst_men] = m_fitness[m_populationSize];
	}
}


void GeneticOperation::select()
{
#ifdef GENETIC_OPERATION_DEBUG
	Logger->debug("select");
#endif
	// ca\B3kowite dopasowanie populacji
	double sum = 0;
	for (qint32 man = 0; man < m_populationSize; man++) {
		sum += m_fitness[man].fitness;
	}
	m_fitnessAllPopulation = sum;
	for (qint32 man = 0; man < m_populationSize; man++) {
		m_fitness[man].rfitness = m_fitness[man].fitness / sum;
	}
	m_fitness[0].cfitness = m_fitness[0].rfitness;
	for (qint32 man = 1; man < m_populationSize; man++) {
		m_fitness[man].cfitness =
			(m_fitness[man - 1].cfitness + m_fitness[man].rfitness);
	}
	Logger->trace("select() loop:");
	std::vector<QJsonArray> m_vectorBitsNew = m_vectorBits;

	for (qint32 man = 0; man < m_populationSize; man++) {
		double p = m_randomGenerator->bounded(0, 1000) / 1000.0;
		Logger->trace("select m_randomGen GeneticOperation::select() :{}", p);
		if (p < m_fitness[0].cfitness) {
			m_vectorBitsNew[man] = m_vectorBits[0];
		}
		else {
			Logger->trace("select else:");
			for (qint32 manNew = 0; manNew < m_populationSize; manNew++) {
				if (p >= m_fitness[manNew].cfitness &&
					p < m_fitness[manNew + 1].cfitness) {
					m_vectorBitsNew[man] = m_vectorBits[manNew + 1];
				}
			}
		}
	}
	Logger->trace("select() copy population");
	for (qint32 man = 0; man < m_populationSize; man++) {
		m_vectorBits[man] = m_vectorBitsNew[man];
		//m_populationJson[man] = m_populationJsonNew[man];
	}
	Logger->trace("select() done");
}



std::vector<QJsonArray> GeneticOperation::createRandomProcessingPopulation(qint32 populationSize, const std::vector<QString>& optimizationTypes, const QJsonObject& m_boundsGraph)
{
#ifdef DEBUG
	Logger->info(" GeneticOperation::createRandomProcessingPopulation");
#endif
	std::vector<QJsonArray> out;
	for (int i = 0; i <= populationSize; i++)
	{
		out.push_back(createRandomProcessing(optimizationTypes, m_boundsGraph));
	}
	return out;
}

QJsonArray GeneticOperation::createRandomProcessing(const std::vector<QString>& optimizationTypes, const QJsonObject& m_boundsGraph)
{
	QJsonArray vectorBits;
	//qDebug() << optimizationTypes;

	for (qint32 i = 0; i < optimizationTypes.size(); i++) {
		//qDebug() << "optimizationTypes[i]:" << optimizationTypes[i];

		QJsonObject dataObject = m_boundsGraph[optimizationTypes[i]].toObject();
		QJsonArray dataUsed = dataObject["Used"].toArray();
		qint32 usedFiltersSize = dataUsed.size();
		//qDebug() << "usedFiltersSize:" << usedFiltersSize;

		for (qint32 j = 0; j < usedFiltersSize; j++) {
			//QJsonObject dataUsedObj = dataUsed[j].toObject();
			//qDebug() << "dataUsedObj[Name]:" << dataUsedObj["Name"].toString();
		}
		//qDebug() << "m_randomGenerator\n";
		int blockType = m_randomGenerator->bounded(0, usedFiltersSize);
		//qDebug() << "blockType:" << blockType;
		QJsonObject bounds = dataUsed[blockType].toObject();
		//qDebug() << "bounds:" << bounds;

		QJsonObject parameterObj = createRandomProcessingBlock(bounds);
		// Add filter information in configs:
		parameterObj["Type"] = optimizationTypes[i];
		vectorBits.append(parameterObj);

		//qDebug() << "parameterObj:" << parameterObj;
		//qDebug() << "\n";
	}
	return vectorBits;
}

QJsonObject GeneticOperation::createRandomProcessingBlock(const QJsonObject& bounds)
{
	QJsonObject parameterObj;
	parameterObj.insert("Name", bounds["Name"].toString());
	QJsonArray parameters = bounds["Parameters"].toArray();
	for (qint32 j = 0; j < parameters.size(); j++) {
		QJsonObject parameterIter = parameters[j].toObject();
		if (parameterIter["IsBool"].toBool() == true)
		{
			//qDebug() << "IsBool!!";
			bool boolValue = m_randomGenerator->bounded(0, 1);
			parameterObj.insert(parameterIter["Type"].toString(), boolValue);
			continue;
		}

		int value = m_randomGenerator->bounded(parameterIter["Min"].toInt(), parameterIter["Max"].toInt() + 1);
		if (parameterIter["IsDouble"].toInt() > 0)
		{
			//qDebug() << "IsDouble!!:" << parameterIter["IsDouble"].toDouble();

			double doubleValue = value / parameterIter["IsDouble"].toDouble();
			parameterObj.insert(parameterIter["Type"].toString(), doubleValue);
			continue;
		}
		if (parameterIter["IsOdd"].toBool() == true)
		{
			//qDebug() << "IsOdd!!";
			if (value % 2 == 0)
			{
				value++;
			}
			parameterObj.insert(parameterIter["Type"].toString(), value);
			continue;
		}
		parameterObj.insert(parameterIter["Type"].toString(), value);
	}
	return parameterObj;
}
