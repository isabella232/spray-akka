#include "main.h"
#include "im.h"
#include "jzon.h"
#include <opencv2/gpu/gpu.hpp>

using namespace eigengo::akka;

Main::Main(const std::string queue, const std::string exchange, const std::string routingKey) :
RabbitRpcServer::RabbitRpcServer(queue, exchange, routingKey) {
	
}

std::string Main::handleMessage(const AmqpClient::BasicMessage::ptr_t message, const AmqpClient::Channel::ptr_t channel) {
	ImageMessage imageMessage(message);
	
	Jzon::Object responseJson;
	try {
		Jzon::Array coinsJson;
		auto imageData = imageMessage.headImage();
		auto imageMat = cv::imdecode(cv::Mat(imageData), 1);
		// ponies & unicorns
		auto coins = coinCounter.count(imageMat);
		
		for (auto i = coins.begin(); i != coins.end(); ++i) {
			auto coin = *i;
			Jzon::Object coinJson;
			coinJson.Add("center", coin.center);
			coinJson.Add("radius", coin.radius);
			coinsJson.Add(coinJson);
		}
		responseJson.Add("coins", coinsJson);
		responseJson.Add("succeeded", true);
	} catch (std::exception &e) {
		// bantha poodoo!
		std::cerr << e.what() << std::endl;
		responseJson.Add("succeeded", false);
	}
	Jzon::Writer writer(responseJson, Jzon::NoFormat);
	writer.Write();

	return writer.GetResult();
}

void Main::inThreadInit() {
	cv::gpu::setDevice(0);
}

int main(int argc, char** argv) {
	Main main("count", "amq.direct", "count.key");
	main.runAndJoin(8);
	return 0;
}