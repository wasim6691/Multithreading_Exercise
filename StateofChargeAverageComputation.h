/*****************************************************************************
 * Multithreading Task
 *****************************************************************************
 *
 * Author: Muhammad Wasim Akhtar Khan
 *
 * Description: The code should fit in a C++ object-orientated environment runnable on a Raspberry Pi. 
   Attached you see a csv file with voltage and soc     values of a battery. Please implement a software with two threads. 
   One thread reads out the values and sends them to the other thread. This thread should get the arithmetic average value of 10 data points 
   and should store them with the UNIX timestamp in milliseconds in a second csv file. To see two parallel working threads I recommend to implement
   sleeps between two read values.
 *
 ****************************************************************************/

#include <iostream> 
#include <string>
#include <vector>
#include <mutex>

/* 
  * This program is implementing multithreading by introducing two threads: one for reading the data from the file and the other one is writng 
    the data into the output file.
  * @ Input Param: battery_data.csv file
  * @ Output Param: Average_Values.csv file (Storing Average values of 10 Data Points with the Unix Timestamps in millisecond)  
*/
class StateOfChargeAverageComputation
{

public:

    ///< Constructor
	StateOfChargeAverageComputation();
    
	///< Function for checking and setting the filenames
	void setFileNames(std::string inputFileName, std::string outputFileName);

	///< Function for reading the soc values from input file
	void readStateOfChargeValues();
	
    ///< Function for writing the average values to the output file
	void writeStateOfChargeValues();
	
    ///< Function for resetting the buffer values to zero
	void resetStateOfChargeBuffer();

private:
	///< Number of SOC values to read from the csv file.
	const uint8_t m_numberOfSocValuesToRead = 10;
	
	///< Keeps the account of input line number that is being read
	uint8_t m_startLineOfReading;
	
	///< Iterator for the number of times input file is being read
	uint8_t m_numberOfTimesFileHasBeenRead;
	
	///< Iterator for the number of times averge values are being written to the output file
	uint8_t m_numberOfTimesFileHasBeenWritten;


	///< Buffer containing the SOC values read from the CSV file.
	std::vector<double> m_stateOfChargeValues;

	///< Input and Output File Variables
	std::string m_inputFileName;
	std::string m_outputFileName;

	///< Mutex to implement locking between two threads
	std::timed_mutex m_lockingSocBuffer;

	///< Variables to implement locking between the read and write functions
	bool m_isReadyToWrite;
	bool m_isReadyToRead;
	
	///< Variable for Thread termination 
	bool m_isOperationCompleted;
};
