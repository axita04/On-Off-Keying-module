
#ifndef A_ERROR_MODEL_H
#define A_ERROR_MODEL_H

#include <list>
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/error-model.h"
#include "ns3/traced-value.h"


namespace ns3 {

class Packet;



//
//AErrorModel
//
class AErrorModel : public ErrorModel
{
public:

  AErrorModel ();
   ~AErrorModel ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  static double calculateBER ();

  static void calculateEb();

  static double SpectralRadiance(int wavelength, double temperature);

  static double integralLum();

  static double integralPlanck();

  static double integralRes();

  static void setWavelengths(int lower, int upper);

  static void setTemperature(int T);

  static double getWavelengthUpper();

  static double getWavelengthLower();

  static double getTemperature();

  static void setNo (double n);

  static void setRx (double x);

  static void setRb (double b);

  static void setRes (double r);

  static double getEb(void);

  static double getBER(void);
  
  static double getNo(void);

  static double getSNR(void);

   
private:

virtual bool DoCorrupt(Ptr<Packet> p);

virtual void DoReset(void);


double m_rate;



Ptr<RandomVariableStream> m_ranvar;
  
};


} // namespace ns3
#endif
