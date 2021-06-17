#include "genetic/graph.h"

//#define DEBUG 
//#define MEMORY_CHECK 
//#define DEBUG_POST_PROCESSING 

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


template<typename T, typename T_data>
Graph<T, T_data>::Graph()
{
	Logger->trace("Graph<T, T_data>::Graph()");
}

template<typename T, typename T_data>
void Graph<T, T_data>::loadGraph(QJsonArray &a_graph, QJsonArray &a_config, std::vector<T*> &a_block)
{
	a_block.clear();
	for (int i = 0; i < a_graph.size(); i++)
	{
		QJsonObject _obj = a_graph[i].toObject();
		QString _type = _obj[TYPE].toString();
		QJsonArray _prevActive = _obj[PREV].toArray();
		T* _block = T::make(_type);
		_block->configure(a_config[i].toObject());
		a_block.push_back(_block);
	}
}

template<typename T, typename T_data>
void Graph<T, T_data>::loadGraph(QJsonArray &a_graph, std::vector<T*> &a_block)
{
	#ifdef DEBUG_POST_PROCESSING
		Logger->debug("Graph<T, T_data>::loadGraph(graph, config, block)");
		qDebug()<< "a_graph:" << a_graph;
	#endif

	a_block.clear();
	for (int i = 0; i < a_graph.size(); i++)
	{
		QJsonObject _obj = a_graph[i].toObject();
		QString _type = _obj[TYPE].toString();
		QJsonArray _prevActive = _obj[PREV].toArray();
		T* _block = T::make(_type);
		_block->configure(_obj[CONFIG].toObject());
		a_block.push_back(_block);
	}
}

template<typename T, typename T_data>
bool Graph<T, T_data>::checkIfLoadInputs(const QJsonArray & prevActive, std::vector<T_data> & dataVec, cv::Mat &input)
{
	#ifdef DEBUG
	Logger->debug("Graph<T, T_data>::checkIfLoadInputs()");
	#endif
	bool _flagNotStart{true};
	for (int j = 0; j < prevActive.size(); j++)
	{
		if (prevActive[j].toObject()[ACTIVE].toInt() == -1)
		{
			_flagNotStart = false;
			T_data data{ input.clone(), "temp1" };
			dataVec.push_back(data);
		}
	}
	#ifdef DEBUG
	Logger->debug("Graph<T, T_data>::checkIfLoadInputs() return:{}", _flagNotStart);
	#endif
	return _flagNotStart;
}

template<typename T, typename T_data>
bool Graph<T, T_data>::checkIfLoadInputs(const QJsonArray & prevActive, std::vector<T_data> & dataVec, std::vector<cv::Mat> &input, int i)
{
	#ifdef DEBUG
	Logger->debug("Graph<T, T_data>::checkIfLoadInputs()");
	#endif
	bool _flagNotStart{true};
	for (int j = 0; j < prevActive.size(); j++)
	{
		if (prevActive[j].toObject()[ACTIVE].toInt() == -1)
		{
			if( i < input.size())
			{
				_flagNotStart = false;
				T_data data{ input[i].clone(), "test1" };
				dataVec.push_back(data);
			}
			else
			{
				Logger->error("Graph<T, T_data>::checkIfLoadInputs() input too short:{}", i);
			}
		}
	}
	#ifdef DEBUG
	Logger->debug("Graph<T, T_data>::checkIfLoadInputs() return:{}", _flagNotStart);
	#endif
	return _flagNotStart;
}

template<typename T, typename T_data>
void Graph<T, T_data>::loadInputs(const QJsonArray &prevActive, std::vector<T_data> & dataVec, QJsonArray &a_graph,
					 std::vector<std::vector<T_data>>& a_data)
{
	#ifdef DEBUG
		Logger->debug("Graph<T, T_data>::loadInputs()");
		Logger->debug("Graph<T, T_data>::loadInputs() prevActive.size():{}", prevActive.size());
		Logger->debug("Graph<T, T_data>::loadInputs() dataVec.size():{}", dataVec.size());
		qDebug()<< "prevActive:" << prevActive;
		qDebug()<< "a_graph:" << a_graph;
	#endif

	for (int prevIter = 0; prevIter < prevActive.size(); prevIter++)
	{
		int _prevIterator = prevActive[prevIter].toObject()[ACTIVE].toInt();
		int signalToCopy = prevActive[prevIter].toObject()[COPY_SIGNAL].toInt();
		if(_prevIterator < 0 || signalToCopy < 0)
		{
			Logger->error("Graph<T, T_data>::loadInputs() load unsupported graph: _prevIterator:{}, signalToCopy:{}", 
			_prevIterator, signalToCopy);
		}
		#ifdef DEBUG
			Logger->debug("Graph<T, T_data>::loadInputs() _prevIterator:{}", _prevIterator);
			qDebug()<< "a_graph[_prevIterator].toObject():" << a_graph[_prevIterator].toObject();
			qDebug()<< "a_graph[_prevIterator].toObject()[NEXT].toArray():" << a_graph[_prevIterator].toObject()[NEXT].toArray();
		#endif
		const QJsonArray _nextActivePrevArray = a_graph[_prevIterator].toObject()[NEXT].toArray();
		#ifdef DEBUG
		qDebug()<<  "_nextActivePrevArray:" << _nextActivePrevArray;
		#endif
		if (_nextActivePrevArray.size() > 1)
		{
			T_data data;
			data = a_data[_prevIterator][signalToCopy];
			data.processing = a_data[_prevIterator][signalToCopy].processing.clone();
			dataVec.push_back(data);
			#ifdef DEBUG
			Logger->debug("Graph<T, T_data>::loadInputs() copy()");
			#endif
		}
		else
		{
			#ifdef DEBUG
			Logger->debug("Graph<T, T_data>::loadInputs() move()");
			#endif
			dataVec.push_back(std::move(a_data[_prevIterator][signalToCopy]));
		}
	}
	#ifdef DEBUG
	Logger->debug("Graph<T, T_data>::loadInputs() done");
	#endif
}

template<typename T, typename T_data>
bool Graph<T, T_data>::checkIfReturnData(const QJsonArray &nextActive)
{
	#ifdef DEBUG
	Logger->debug("Graph<T, T_data>::checkIfReturnData()");
	#endif
	bool _flagReturnData{false};
	for (int j = 0; j < nextActive.size(); j++)
	{
		if (nextActive[j].toObject()[ACTIVE].toInt() == -1)
		{
			_flagReturnData = true;
		}
	}
	#ifdef DEBUG
	Logger->debug("Graph<T, T_data>::checkIfReturnData() return:{}", _flagReturnData);
	#endif
	return _flagReturnData;
}

template<typename T, typename T_data>
void Graph<T, T_data>::returnData(int i, std::vector<cv::Mat> & outputData, std::vector<std::vector<T_data>> &outputDataVector,
							std::vector<std::vector<T_data>>& data)
{
	Logger->trace("Graph<T, T_data>::returnData()");
	for (int ii = 0; ii < data[i].size(); ii++)
	{
		outputData.push_back(data[i][ii].processing.clone());
	}
	outputDataVector.push_back(data[i]);
	Logger->trace("Graph<T, T_data>::returnData() done");
}

template<typename T, typename T_data>
void Graph<T, T_data>::returnData(int i, std::vector<cv::Mat> & outputData, std::vector<std::vector<T_data>>& data)
{
	Logger->trace("Graph<T, T_data>::returnData()");
	for (int ii = 0; ii < data[i].size(); ii++)
	{
		outputData.push_back(data[i][ii].processing.clone());
	}
	Logger->trace("Graph<T, T_data>::returnData() done");
}

template class Graph<Processing, _data>;
template class Graph<PostProcess, _postData>;
