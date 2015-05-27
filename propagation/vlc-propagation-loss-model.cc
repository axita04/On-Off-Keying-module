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
#include "vlc-propagation-loss-model.h"

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

VLCPropagationLossModel::VLCPropagationLossModel ()
{
}

void
VLCPropagationLossModel::SetTxPower (double dBm)
{
  m_TxPower = dBm;
}

double
VLCPropagationLossModel::GetTxPower ()
{
  return m_TxPower;
}

void
VLCPropagationLossModel::SetLambertianOrder (double semiangle)
{
  m_LambertianOrder = ((-1) * (std::log(2))) / (std::log(std::cos(semiangle * (M_PI / 180))));
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

double
VLCPropagationLossModel::GetRadianceAngle(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
double x = std::abs(a->GetPosition().x - b->GetPosition().x);
double y = std::abs(a->GetPosition().y - b->GetPosition().y);
return std::atan((x / y) * (M_PI / 180));
}

double
VLCPropagationLossModel::GetIncidenceAngle(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
return (GetRadianceAngle(a,b));
}

double
VLCPropagationLossModel::DoCalcRxPower(double TxPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{

  return (m_TxPower) * (((m_LambertianOrder + 1) * m_PhotoDetectorArea) / (2 * M_PI * std::pow(GetDistance(a,b),2))) * (std::pow(std::cos(GetRadianceAngle(a,b)),m_LambertianOrder)) * m_FilterGain * m_ConcentratorGain * std::cos(GetIncidenceAngle(a,b));
}

double
VLCPropagationLossModel::GetRxPower(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  
  return DoCalcRxPower(m_TxPower, a, b);
}

int64_t
VLCPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

}/*ns3 namespace*/
