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

#ifndef VLC_PROPAGATION_LOSS_MODEL_H
#define VLC_PROPAGATION_LOSS_MODEL_H

#include "ns3/nstime.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/VLC-Mobility-Model.h"
#include <vector>

namespace ns3 {

class VLCPropagationLossModel : public PropagationLossModel
{

public:
 struct vectorA;
  /**
   * \breif Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  //Constructor
  VLCPropagationLossModel ();
  void SetTxPower(double dBm);
  double GetTxPower();
  double dotProduct(std::vector<double> v1, std::vector<double> v2);
  double magnitude(std::vector<double> v);
  void SetLambertianOrder(double semiangle);
  double GetLambertianOrder();
  void SetFilterGain(double gain);
  double GetFilterGain();
  void SetConcentratorGain(double fov, double refracIndex);
  
  double GetConcentratorGain();
  void SetPhotoDetectorArea(double area);
  double GetPhotoDetectorArea();
  double GetDistance(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  double GetRadianceAngle(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  double GetIncidenceAngle(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  double GetRxPower(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
/**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  VLCPropagationLossModel (const VLCPropagationLossModel &);
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  VLCPropagationLossModel & operator = (const VLCPropagationLossModel &);

  virtual double DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  virtual int64_t DoAssignStreams (int64_t stream);
  double m_TxPower;
  double m_LambertianOrder;
  double m_FilterGain;
  double m_ConcentratorGain;
  double m_PhotoDetectorArea;
  double m_RxPower;

};

}

#endif /* VLCPROPAGATIONMODEL*/
