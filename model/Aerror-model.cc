
/* 
 *
 * Author: Ryan Ackerman   <rea9@njit.edu>
 *         
 *         
*/

#include <cmath>

#include "ns3/Aerror-model.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include <time.h>
#include <stdlib.h>

namespace ns3 {


//
// AErrorModel
//
NS_LOG_COMPONENT_DEFINE("AErrorModel");


NS_OBJECT_ENSURE_REGISTERED (AErrorModel);

double No = 0;   // Noise power in A^2
double Rx = 0;   // Received Power in dbm
double BER = 0;  // Bit Error Rate
double Rb = 0;   // Data Rate
double Res =1;   //Responsitivity of Receiver
double Tb = 0;   // 1 / data rate
double SNR = 0;  //Signal to noise ratio

//Constructor
AErrorModel::AErrorModel ()
{
  NS_LOG_FUNCTION (this);
  srand(time(NULL));  //Seeds Random Number Generator
}

AErrorModel::~AErrorModel () 
{
  NS_LOG_FUNCTION (this);
}

TypeId AErrorModel::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::AErrorModel")

    .SetGroupName("Network")
    .AddConstructor<AErrorModel> ()  
     ;
  return tid;
}
// Virtual method from Error Model.  Determinds what packets to be dropped.
bool AErrorModel::DoCorrupt(Ptr<Packet> p){
	NS_LOG_FUNCTION(this << p);
	
	BER = calculateBER();
	//Caculated the Packet Error Rate by finding the complement of the probablility 
	//that a packets is not corrupted
	double per = 1.0 - (double)std::pow((double)(1.0 - BER), static_cast<double>(8*p->GetSize()));
        //Randomizies a number and if its less than the PER the packet is rejected
	double rnd  = (double) rand()/(double)(RAND_MAX);
        return (rnd < per);
}
void AErrorModel::DoReset(void){


}
//Calculates BER from SNR
double AErrorModel::calculateBER (){
//SNR calculation
//std::cout << "RX : " << Rx << std::endl;
SNR = (std::pow((Rx*Res),2)/No);
//std::cout <<SNR << " : SNR" <<std::endl;
double BER;
if(SNR > 0){
//BER calculation
BER = 0.5*erfc(std::sqrt(SNR/2));
}else{
BER = 1;
}
BER = 0.00003;
return BER;
}
//Set Noise power
void AErrorModel::setNo (double n){
 No = n;
}
//Set Rx Power
void AErrorModel::setRx (double x){
Rx = x;
}
//Sets bit rate and inverse of bit rate
void AErrorModel::setRb (double b){
Rb =b;
Tb = 1/Rb;
}
//Sets resposetivity
void AErrorModel::setRes (double r){
Res = r;
}
//Gets Noise power
double AErrorModel::getNo(void){
return No;
}
//Gets BER
double AErrorModel::getBER(void){
return BER;
}
//Gets SNR
double AErrorModel::getSNR(void){
return SNR;
}

} // namespace ns3
