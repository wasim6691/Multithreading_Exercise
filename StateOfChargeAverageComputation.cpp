#include "StateOfChargeAverageComputation.h"
#include <chrono>
#include <fstream>
#include <sstream>

/*!
  * Constructor Initializor
*/
StateOfChargeAverageComputation::StateOfChargeAverageComputation() :
	m_startLineOfReading(0),
	m_numberOfTimesFileHasBeenRead(0),
	m_numberOfTimesFileHasBeenWritten(0),
	m_inputFileName(),
	m_outputFileName(),
	m_lockingSocBuffer(),
	m_isReadyToWrite(false),
    m_isReadyToRead(true),
    m_isOperationCompleted(false)
{
	m_stateOfChargeValues.reserve(m_numberOfSocValuesToRead);

	for (uint8_t i = 0; i < m_numberOfSocValuesToRead; i++)
	{
		m_stateOfChargeValues.push_back(0);
	}

}

/*!
  *  Setting File names and checking the correctness
  *  @ Params: (Input File Name, Output File Name)
*/
void StateOfChargeAverageComputation::setFileNames(std::string inputFileName, std::string outputFileName)
{
	uint8_t indexOfDot = inputFileName.find_last_of(".");
	using namespace std;

	if (inputFileName.substr(indexOfDot + 1) == "csv")
	{
		m_inputFileName = inputFileName;
	}
	else
	{
		cout << "Invalid input file name" << endl;
		return;
	}

	indexOfDot = outputFileName.find_last_of(".");

	if (outputFileName.substr(indexOfDot + 1) == "csv")
	{
		m_outputFileName = outputFileName;
	}
	else
	{
		cout << "Invalid output file name" << endl;
		return;
	}

	m_outputFileName = outputFileName;
}

/*!
  * Resetting the (Buffer --> resetStateOfChargeBuffer) values
*/
void StateOfChargeAverageComputation::resetStateOfChargeBuffer()
{
	for (uint8_t i = 0; i < m_numberOfSocValuesToRead; i++)
	{
		m_stateOfChargeValues[i] = 0;
	}
}

/*! 
  * Reading values from the input file and storing 10 values in m_stateOfChargeValues buffer.
  * Implementing sleeps and lock for 100 ms.
  * m_isReadyToRead and  m_isReadyToWrite are reading and writing bools to make reading and writing more synchronized.
*/

void StateOfChargeAverageComputation::readStateOfChargeValues()
{
	
	while(1)
	{
		using namespace std;
		std::chrono::milliseconds timeout(100);
		
		if (m_isReadyToRead)
		{
			if (m_lockingSocBuffer.try_lock_for(timeout))
			{
				///< Resetting buffer for every new read
				resetStateOfChargeBuffer();

				string lineToRead;
				vector<string> BatteryData;
                
				///< Reading Binary file data
				ifstream batteryDataStringStream;
				batteryDataStringStream.open(m_inputFileName, ifstream::in);
				uint8_t iteration = 0;
				
				///< For Terminating the thread when complete file is read
				uint8_t noOfValidValues = 0;
			
				while (getline(batteryDataStringStream, lineToRead))
				{
					iteration++;
					if ((iteration >= m_startLineOfReading) && ((m_startLineOfReading) > 0) && (m_startLineOfReading > (m_numberOfTimesFileHasBeenRead * m_numberOfSocValuesToRead)))
					{
						
					    ///< Extracting the second column (Soc Values) from the "battery_data.csv" file
						stringstream lineStream(lineToRead);
						string data;
						getline(lineStream, data, ',');
						getline(lineStream, data, '\n');
						
						///< Saving 10 Values of soc values in buffer
						m_stateOfChargeValues[(m_startLineOfReading - 1) % m_numberOfSocValuesToRead] = (stod(data));
							
						m_startLineOfReading++;
						noOfValidValues++;

						if ((m_startLineOfReading - 1) % m_numberOfSocValuesToRead == 0)
						{
							break;
						}
					}
					else if (m_startLineOfReading == 0)
					{
						m_startLineOfReading++;
					}
					else
					{
						// Do Nothing
					}

				}  // End While 

				m_numberOfTimesFileHasBeenRead++;
				m_isReadyToRead = false;
				m_isReadyToWrite = true;

				///<   close the output file
				///<   batteryDataStringStream.close();
				cout << "Data is Read!" << endl;

				m_lockingSocBuffer.unlock();
				
				///< Terminating the Thread after complete file is read
				if (noOfValidValues < m_numberOfSocValuesToRead)
				{
					m_isOperationCompleted = true;
					break;
				}
			}     // End Locking

			else
				///< Sleep duration of 100ms
				std::this_thread::sleep_for(timeout);
		}

	} // End While
}

/*! 
  * Writing Average values to the output file.
  * Implementing sleeps and lock for 100 ms.
*/

void StateOfChargeAverageComputation::writeStateOfChargeValues()
{
	
	while (1)
	{
		if (m_isReadyToWrite)
		{
			std::chrono::milliseconds timeout(100);
			if (m_lockingSocBuffer.try_lock_for(timeout))
			{
				  using namespace std;

				    double averageValueOfStateOfCharge = 0.0;

				    ///< Open the output file to write the  average values
				    ofstream outputFile;

					if (m_numberOfTimesFileHasBeenWritten == 0)
					{
						outputFile.open(m_outputFileName, ofstream::trunc);
					}
					else
					{
						outputFile.open(m_outputFileName, std::ios::app);
					}
			
					if (m_numberOfTimesFileHasBeenWritten == 0)
					{
						///< OutPut File Header
						outputFile << "Index" << "," << "Average Values (10 Data Points)" << "," << "Timestamps (milliseconds)" << endl;
					}

                    ///<  Average Value Calculation of 10 Data points
					///<  Assumption: Rest of the values for the very last calculation as assumed as Zeros
					for (uint8_t bufferEntries = 0; bufferEntries < m_numberOfSocValuesToRead; bufferEntries++)
					{
						averageValueOfStateOfCharge += m_stateOfChargeValues[bufferEntries];
					}

					averageValueOfStateOfCharge = averageValueOfStateOfCharge / m_numberOfSocValuesToRead;
					
					m_numberOfTimesFileHasBeenWritten++;

					///< UNIX timestamps in milliseconds
					using namespace std::chrono;
					uint64_t timeInMilliSeconds = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

					///<  Storing Average Values in the Output File
					outputFile << to_string(m_numberOfTimesFileHasBeenWritten) << "," << to_string(averageValueOfStateOfCharge) << "," <<to_string(timeInMilliSeconds) << endl;

					m_lockingSocBuffer.unlock();

				///<  close the output file
				outputFile.close();
				
				m_isReadyToWrite = false;
				m_isReadyToRead = true;
				cout << "Data is Stored!" << endl;

			} // End Locking
			else
				///< Sleep duration of 100 ms
				std::this_thread::sleep_for(timeout);
		}

		///< Terminating the thread when all values are written
		if (m_isOperationCompleted == true)
		{
			break;
		}

	} // End While 
}

