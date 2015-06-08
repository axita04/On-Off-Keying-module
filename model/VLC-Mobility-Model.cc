/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Gustavo Carneiro  <gjc@inescporto.pt>
 */
#include "VLC-Mobility-Model.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (VlcMobilityModel);

TypeId VlcMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VlcMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<VlcMobilityModel> ();
  return tid;
}

VlcMobilityModel::VlcMobilityModel ()
{
}

VlcMobilityModel::~VlcMobilityModel ()
{
}

inline Vector
VlcMobilityModel::DoGetVelocity (void) const
{
  double t = (Simulator::Now () - m_baseTime).GetSeconds ();
  return Vector (m_baseVelocity.x + m_acceleration.x*t,
                 m_baseVelocity.y + m_acceleration.y*t,
                 m_baseVelocity.z + m_acceleration.z*t);
}

inline Vector
VlcMobilityModel::DoGetPosition (void) const
{
  double t = (Simulator::Now () - m_baseTime).GetSeconds ();
  double half_t_square = t*t*0.5;
  return Vector (m_basePosition.x + m_baseVelocity.x*t + m_acceleration.x*half_t_square,
                 m_basePosition.y + m_baseVelocity.y*t + m_acceleration.y*half_t_square,
                 m_basePosition.z + m_baseVelocity.z*t + m_acceleration.z*half_t_square);
}

void 
VlcMobilityModel::DoSetPosition (const Vector &position)
{
  m_baseVelocity = DoGetVelocity ();
  m_baseTime = Simulator::Now ();
  m_basePosition = position;
  NotifyCourseChange ();
}

void 
VlcMobilityModel::SetVelocityAndAcceleration (const Vector &velocity,
                                                               const Vector &acceleration)
{
  m_basePosition = DoGetPosition ();
  m_baseTime = Simulator::Now ();
  m_baseVelocity = velocity;
  m_acceleration = acceleration;
  NotifyCourseChange ();
}

void VlcMobilityModel::SetAzimuth(double angle)
{
  m_azimuth = angle;
}

void VlcMobilityModel::SetElevation(double angle)
{
  m_elevation = angle;
}

double VlcMobilityModel::GetAzimuth(void)
{
  return m_azimuth;
}
double VlcMobilityModel::GetElevation(void)
{
  return m_elevation;
}

} // namespace ns3
