
/* 
 * This script calculates the Illuminance of a light source on a surface
 *
 * Author: Walter Berreta   <web3@njit.edu>
 *         
 *         
*/

#include <cmath>
#include "VLC-Mobility-Model.h"
#include "ns3/Illumination.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/traced-value.h"
#include "ns3/mobility-model.h"
#include "ns3/trace-source-accessor.h"
#include <time.h>
#include <stdlib.h>

namespace ns3 {

//structure called vector with i,j,k components
struct vector{
   double i;
   double j;
   double k;
};
//takes the dot product of 2 vectors
double dotProduct(vector v1, vector v2){
   return v1.i*v2.i+v1.j*v2.j+v1.k*v2.k;
};
//takes the magnitude of a vector
double magnitude(vector v){
   return std::sqrt(std::pow(v.i , 2) + std::pow(v.j , 2) + std::pow(v.k , 2));
};

//
// Illumination
//
NS_LOG_COMPONENT_DEFINE("Illumination");


//NS_OBJECT_ENSURE_REGISTERED (Illumination);

double efficacy = 0;	//luminous efficacy
double Power = 0;       //optical power
double LambertianOrder = 0;
double illuminance = 0;	


Illumination::Illumination ()
{
  NS_LOG_FUNCTION (this);
  srand(time(NULL));
}

Illumination::~Illumination () 
{
  NS_LOG_FUNCTION (this);
}
/*
TypeId Illumination::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::Illumination")

    .SetGroupName("Network")
    .AddConstructor<Illumination> ()  
     ;
  return tid;
}*/


void Illumination::setEfficacy (double x){
efficacy = x;
}

void Illumination::setPower (double b){
Power = b;
}

// Semi-angle can be usually be obtained from the specs of the light source
void
Illumination::setLambertianOrder (double semiangle)
{
  LambertianOrder = ((-1) * (std::log(2))) / (std::log(std::cos(semiangle * (M_PI / 180))));
}

double
Illumination::getLambertianOrder ()
{
  return LambertianOrder;
}

double Illumination::getEfficacy(void){
return efficacy;
}

double Illumination::getPower(void){
return Power;
}

double Illumination::getRadianceAngle(Ptr<VlcMobilityModel> a, Ptr<VlcMobilityModel> b) const
{
vector vec1, vec2;
double angle;
//rectangular coordinates
vec1.i = ((a->GetPosition().x - b->GetPosition().x))*-1;
vec1.j = ((a->GetPosition().y - b->GetPosition().y))*-1;
vec1.k = ((a->GetPosition().z - b->GetPosition().z))*-1;

//spherical coordinates with r = 1 conversion to rectangular coordinates
vec2.i = (std::sin(a->GetElevation())) * (std::cos(a->GetAzimuth()));
vec2.j = (std::sin(a->GetElevation())) * (std::sin(a->GetAzimuth()));
vec2.k = (std::cos(a->GetElevation()));

//finds the angle between vec1 and vec2
//in other words finds the radiance angle, which is the angle between the normal vector of the transmitter and the vector pointing to the radiated surface
angle = std::acos(dotProduct(vec1, vec2) / magnitude(vec1) * magnitude(vec2));

return angle;
}

double Illumination::getIncidenceAngle(Ptr<VlcMobilityModel> a, Ptr<VlcMobilityModel> b) const
{
vector vec1, vec2;
double angle;
//rectangular coordinates
vec1.i = ((a->GetPosition().x - b->GetPosition().x));
vec1.j = ((a->GetPosition().y - b->GetPosition().y));
vec1.k = ((a->GetPosition().z - b->GetPosition().z));

//spherical coordinates with r = 1 conversion to rectangular coordinates
vec2.i = (std::sin(b->GetElevation())) * (std::cos(b->GetAzimuth()));
vec2.j = (std::sin(b->GetElevation())) * (std::sin(b->GetAzimuth()));
vec2.k = (std::cos(b->GetElevation()));

//finds the angle between vec1 and vec2
//in other words finds the incidence angle, which is the angle between the normal vector of the surface and the vector pointing to the transmitting light source
angle = std::acos(dotProduct(vec1, vec2) / magnitude(vec1) * magnitude(vec2));

return angle;
}

double Illumination::calculateIlluminance(Ptr<VlcMobilityModel> a, Ptr<VlcMobilityModel> b){
  //distance formula from point a to point b
  double distance = std::sqrt((std::pow((b->GetPosition().x - a->GetPosition().x),2)) + (std::pow((b->GetPosition().y - a->GetPosition().y),2)) + (std::pow((b->GetPosition().z - a->GetPosition().z),2)));

  illuminance = Power*(LambertianOrder+1) * (std::pow(std::cos(getRadianceAngle(a,b)),LambertianOrder)) * (std::cos(getIncidenceAngle(a,b))) * getEfficacy()/ (2*M_PI*(std::pow(distance, 2)));  

  return illuminance;
}




} // namespace ns3
