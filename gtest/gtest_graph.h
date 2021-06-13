#include "gtest/gtest.h"
#include "gmock/gmock.h" 

#include <QDebug>

#include "genetic/graph.h"
#include "genetic/data.h"
#include "utils/includespdlog.h"
#include "utils/configreader.h"

using ::testing::AtLeast;


namespace gtest_graph
{
	class GTest_graph : public ::testing::Test
	{
		protected:
			GTest_graph(){}
			~GTest_graph() override {}
			void SetUp() override{}
			void TearDown() override {}

			QJsonObject readConfig(QString name);
			QJsonArray readArray(QString name);
			imageErrors checkBlock(cv::Mat &input, cv::Mat &gt, int w, int h, int dronSize);
			
	};

}  // namespace gtest_graph
