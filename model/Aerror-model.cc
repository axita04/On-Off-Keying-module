
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

namespace ns3 {


//
// AErrorModel
//
NS_LOG_COMPONENT_DEFINE("AErrorModel");


NS_OBJECT_ENSURE_REGISTERED (AErrorModel);

double No = 0;
double Eb = 0;

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
	double BER = calculateBER();
	double per = 1.0 - (double)std::pow((double)(1.0 - BER), static_cast<double>(8*p->GetSize()));
	double rnd  = (double) rand()/(double)(RAND_MAX);
	return (rnd < per);
}
void AErrorModel::DoReset(void){


}

double AErrorModel::calculateBER (){
double BER = 0.5*erfc(std::sqrt(Eb/No));
//std::cout<<(Eb/No) << std::endl;
return BER;
}

void AErrorModel::setNo (double n){
 No = n;
}

void AErrorModel::setEb (double e){
Eb = e;
}

} // namespace ns3
