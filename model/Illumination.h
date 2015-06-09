
#ifndef ILLUMINATION_H
#define ILLUMINATION_H

#include <cmath>
#include <list>
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-value.h"
#include "ns3/mobility-model.h"
#include "ns3/VLC-Mobility-Model.h"


namespace ns3 {

class Packet;



//
//AErrorModel
//
class Illumination
{
public:

  Illumination ();
   ~Illumination ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
 // static TypeId GetTypeId (void);

  double calculateIlluminance (Ptr<VlcMobilityModel> a, Ptr<VlcMobilityModel> b);

  static void setEfficacy (double n);

  static void setPower (double x);

  static void setLambertianOrder (double b);

  static double getLambertianOrder(void);

  double getRadianceAngle(Ptr<VlcMobilityModel> a, Ptr<VlcMobilityModel> b) const;

  static double getPower(void);

  static double getEfficacy(void);


   
private:

  
};


} // namespace ns3
#endif
