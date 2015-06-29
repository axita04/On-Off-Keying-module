
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008, 2009 INRIA, UDcast
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: James Basuino       <jb392@njit.edu>
 *                              
 */

#include "ns3/propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include <cmath>
#include <vector>
#include "vlc-propagation-loss-model.h"
#include "ns3/VLC-Mobility-Model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VLCPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (VLCPropagationLossModel);

TypeId
VLCPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VLCPropagationLossModel")

    .SetParent<PropagationLossModel> ()

    .AddConstructor<VLCPropagationLossModel> ()

    .AddAttribute ("TxPower",
		   "The power transmitted (default is 0dBm = 1mW).",
                   DoubleValue (0),
                   MakeDoubleAccessor
(&VLCPropagationLossModel::m_TxPower),
		   MakeDoubleChecker<double> ())

    .AddAttribute ("LambertianOrder",
		   "LambertianOrder (default is -ln(2)).",
		   DoubleValue(std::log(2) * -1),
                   MakeDoubleAccessor
(&VLCPropagationLossModel::m_LambertianOrder),
		   MakeDoubleChecker<double> ())

    .AddAttribute ("FilterGain",
		   "FilterGain (default is 0).",
		   DoubleValue(0),
                   MakeDoubleAccessor
(&VLCPropagationLossModel::m_FilterGain),
		   MakeDoubleChecker<double> ())

    .AddAttribute ("PhotoDetectorArea",
		   "Area of the photo detector (default is 0).",
		   DoubleValue(0),
                   MakeDoubleAccessor
(&VLCPropagationLossModel::m_PhotoDetectorArea),
		   MakeDoubleChecker<double> ())

    .AddAttribute ("ConcentratorGain",
		   "ConcentratorGain (default is 0).",
		   DoubleValue(0),
                   MakeDoubleAccessor
(&VLCPropagationLossModel::m_ConcentratorGain),
		   MakeDoubleChecker<double> ());
  return tid;
}


double VLCPropagationLossModel::Fov = 0;
VLCPropagationLossModel::VLCPropagationLossModel ()
{
}



void
VLCPropagationLossModel::SetTxPower (double dBm)
{
  m_TxPower = dBm;
}


void 
VLCPropagationLossModel::setEfficacy (double x){
m_efficacy = x;
}


double 
VLCPropagationLossModel::getEfficacy(void)
{
  return m_efficacy;
}

double
VLCPropagationLossModel::GetTxPower ()
{
  return m_TxPower;
}

void
VLCPropagationLossModel::SetLambertianOrder (double semiangle)
{
  m_LambertianOrder = ((-1) * (std::log(2))) / (std::log(std::cos(semiangle * (M_PI / 180)))); //the pi/180 is to make it into radians
}

double
VLCPropagationLossModel::GetLambertianOrder ()
{
  return m_LambertianOrder;
}

void
VLCPropagationLossModel::SetFilterGain (double gain)
{
  m_FilterGain = gain;
}

double
VLCPropagationLossModel::GetFilterGain ()
{
  return m_FilterGain;
}

void
VLCPropagationLossModel::SetConcentratorGain (double fov, double refracIndex)//,Ptr<MobilityModel> a, Ptr<MobilityModel> b)
{
  Fov = fov * (M_PI/180);
  /*if(fov < GetIncidenceAngle(a,b)) {
        m_ConcentratorGain = 0;
  }else{*/
        m_ConcentratorGain = std::pow(refracIndex,2) / std::pow(std::sin(fov),2);
  //}
}

double
VLCPropagationLossModel::GetConcentratorGain ()
{
  return m_ConcentratorGain;
}

void
VLCPropagationLossModel::SetPhotoDetectorArea(double area)
{
  m_PhotoDetectorArea = area;
}

double
VLCPropagationLossModel::GetPhotoDetectorArea()
{
  return m_PhotoDetectorArea;
}

double
VLCPropagationLossModel::GetDistance(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
return std::sqrt((std::pow((b->GetPosition().x - a->GetPosition().x),2)) + (std::pow((b->GetPosition().y - a->GetPosition().y),2)) + (std::pow((b->GetPosition().z - a->GetPosition().z),2)));
}

//---------------------------------------------------
double VLCPropagationLossModel::dotProduct(std::vector<double> v1, std::vector<double> v2) const
{

return ((v1.at(0) * v2.at(0)) + (v1.at(1) * v2.at(1)) + (v1.at(2) * v2.at(2)));
}
double VLCPropagationLossModel::magnitude(std::vector<double> v) const
{
 // std::cout << "MORE STUFF : " << std::sqrt(std::pow(v.at(0),2) + std::pow(v.at(1),2) + std::pow(v.at(2),2)) << std::endl;   
  
  return std::sqrt(std::pow(v.at(0),2) + std::pow(v.at(1),2) + std::pow(v.at(2),2));
}

//----------------------------------------------------
double
VLCPropagationLossModel::GetRadianceAngle(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
Ptr<VlcMobilityModel> x = DynamicCast<VlcMobilityModel >(a);
Ptr<VlcMobilityModel> y = DynamicCast<VlcMobilityModel >(b);

std::vector<double> v1,v2;

v1.push_back((a->GetPosition().x - b->GetPosition().x));
v1.push_back((a->GetPosition().y - b->GetPosition().y));
v1.push_back((a->GetPosition().z - b->GetPosition().z));

v2.push_back(std::sin(x->GetElevation()) * std::cos(x->GetAzimuth()));
v2.push_back(std::sin(x->GetElevation()) * std::sin(x->GetAzimuth()));
v2.push_back(std::cos(x->GetElevation()));
//std::cout << "v1 0 : " << v1.at(0) << " v1 : 1 " << v1.at(1) << " v1 : 2 " << v1.at(2) << std::endl; 
//std::cout << "v2 0 : " << v2.at(0) << " v2 : 1 " << v2.at(1) << " v2 : 2 " << v2.at(2) << std::endl;
//std::cout<< "Stuff : " << (dotProduct(v1,v2)/(magnitude(v1)*magnitude(v2))) << std::endl;
double angle = std::acos((dotProduct(v1,v2)/(magnitude(v1)*magnitude(v2))));
//std::cout << "RAD ANGLE: " << angle << std::endl;
return (angle);
}

double
VLCPropagationLossModel::GetIncidenceAngle(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
Ptr<VlcMobilityModel> x = DynamicCast<VlcMobilityModel >(a);
Ptr<VlcMobilityModel> y = DynamicCast<VlcMobilityModel >(b);

std::vector<double> v1,v2;

v1.push_back((a->GetPosition().x - b->GetPosition().x)*-1);
v1.push_back((a->GetPosition().y - b->GetPosition().y)*-1);
v1.push_back((a->GetPosition().z - b->GetPosition().z)*-1);

v2.push_back(std::sin(y->GetElevation()) * std::cos(y->GetAzimuth()));
v2.push_back(std::sin(y->GetElevation()) * std::sin(y->GetAzimuth()));
v2.push_back(std::cos(y->GetElevation()));

double angle = std::acos(dotProduct(v1,v2)/(magnitude(v1)*magnitude(v2) ));
//std::cout << "INC ANGLE : " << angle << std::endl;
return (angle);
}

double
VLCPropagationLossModel::DoCalcRxPower(double TxPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
//Ptr<VlcMobilityModel> x = DynamicCast<VlcMobilityModel >(a);
//Ptr<VlcMobilityModel> y = DynamicCast<VlcMobilityModel >(b);

if(Fov < ((M_PI/2)-GetIncidenceAngle(a,b))){
  return 0;
}else{
  return (m_TxPower) * (((m_LambertianOrder + 1) * m_PhotoDetectorArea) / (2 * M_PI * std::pow(GetDistance(a,b),2))) * (std::pow(std::cos(GetRadianceAngle(a,b)),m_LambertianOrder)) * m_FilterGain * m_ConcentratorGain * std::cos(GetIncidenceAngle(a,b));//the equation for getting power received and it is is dBm
}
}

double
VLCPropagationLossModel::GetRxPower(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const //is necessary to access the private function that calculates the received power
{
  
  return DoCalcRxPower(m_TxPower, a, b);
}

int64_t
VLCPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

double VLCPropagationLossModel::calculateIlluminance(Ptr<MobilityModel> a, Ptr<MobilityModel> b){
//distance formula from point a to point b
double distance = std::sqrt((std::pow((b->GetPosition().x - a->GetPosition().x),2)) + (std::pow((b->GetPosition().y - a->GetPosition().y),2)) + (std::pow((b->GetPosition().z - a->GetPosition().z),2)));
illuminance = GetTxPower()*(GetLambertianOrder()+1) * (std::pow(std::cos(GetRadianceAngle(a,b)),GetLambertianOrder())) * (std::cos(GetIncidenceAngle(a,b))) * getEfficacy()/ (2*M_PI*(std::pow(distance, 2)));
return illuminance;
}




}/*ns3 namespace*/
