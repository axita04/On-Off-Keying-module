
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

namespace ns3 {


//
// AErrorModel
//
NS_LOG_COMPONENT_DEFINE("AErrorModel");


NS_OBJECT_ENSURE_REGISTERED (AErrorModel);

double No = 0;
double Eb = 0;
double Rx = 0;
double BER = 0;
double Rb = 0;
double Res =1;
double Tb = 0; 
double SNR = 0;


AErrorModel::AErrorModel ()
{
  NS_LOG_FUNCTION (this);
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

bool AErrorModel::DoCorrupt(Ptr<Packet> p){
	NS_LOG_FUNCTION(this << p);
	BER = calculateBER();
	double per = 1.0 - (double)std::pow((double)(1.0 - BER), static_cast<double>(8*p->GetSize()));
	double rnd  = (double) rand()/(double)(RAND_MAX);
	return (rnd < per);
}
void AErrorModel::DoReset(void){


}

double AErrorModel::calculateBER (){
calculateEb();
SNR = std::pow((Rx*Res),2)/No;
double correctSNR = 10 * std::log10(SNR);
double BER = 0.5*erfc(std::sqrt(correctSNR/2));
//std::cout<< " SNR : " <<(SNR) << std::endl;
return BER;
}

void AErrorModel::calculateEb(){
	//Eb = Rx;
  
}
void AErrorModel::setNo (double n){
 No = n;
}

void AErrorModel::setRx (double x){
Rx = x;
}

void AErrorModel::setRb (double b){
Rb =b;
Tb = 1/Rb;
}

void AErrorModel::setRes (double r){
Res = r;
}
double AErrorModel::getEb(void){
return Eb;
}

double AErrorModel::getNo(void){
return No;
}

double AErrorModel::getBER(void){
return BER;
}
double AErrorModel::getSNR(void){
return 10*std::log10(SNR);
}

} // namespace ns3
