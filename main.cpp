#include "StateOfChargeAverageComputation.h"
#include <thread>

int main()
{
	///< Input and Output File names
	std::string inputFileName = "battery_data.csv";
	std::string outputFileName = "AverageValuesSoc.csv";
    
	///< Class Instantiation 
	StateOfChargeAverageComputation* stateOfChargeComputation = new StateOfChargeAverageComputation();  
	
	stateOfChargeComputation->setFileNames(inputFileName, outputFileName);

	///< Creating thread by non-static member function
	std::thread readerThread(&StateOfChargeAverageComputation::readStateOfChargeValues , stateOfChargeComputation);
	std::thread writerThread(&StateOfChargeAverageComputation::writeStateOfChargeValues, stateOfChargeComputation);

	///< Wait for thread reader to finish 
	readerThread.join();

	///< Wait for thread writer to finish.
	writerThread.join();


	getchar();
	return 0;
}