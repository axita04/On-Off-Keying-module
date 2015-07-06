
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

  //static TypeId GetTypeId (void);

  double No;   // Noise power in A^2
  double Rx;   // Received Power in dbm
  double BER;  // Bit Error Rate
  double Rb;   // Data Rate
  double Res;   //Responsitivity of Receiver
  double Tb;   // 1 / data rate
  double SNR;  //Signal to noise ratio

  double calculateBER ();

  void calculateEb();

  double SpectralRadiance(int wavelength, double temperature);

  double integralLum();

  double integralPlanck();

  double integralRes();

  void setWavelengths(int lower, int upper);

  void setTemperature(int T);

  double getWavelengthUpper();

  double getWavelengthLower();

  double getTemperature();

  void setNo (double n, double a);

  void setRx (double x);

  void setRb (double b);

  void setRes (double r);

  double getEb(void);

  double getBER(void);
  
  double getNo(void);

  double getSNR(void);
   
private:

virtual bool DoCorrupt(Ptr<Packet> p);

virtual void DoReset(void);


double m_rate;



Ptr<RandomVariableStream> m_ranvar;
  
};


} // namespace ns3
#endif
