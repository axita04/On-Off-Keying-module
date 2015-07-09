
#ifndef OOK_2Int_ERROR_MODEL_H
#define OOK_2Int_ERROR_MODEL_H

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
class OOK2IntErrorModel : public ErrorModel
{
public:

  OOK2IntErrorModel ();
   ~OOK2IntErrorModel ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static double V_lambda[] ;

static double Response[] ;


  static TypeId GetTypeId (void);

  double No;   // Noise power in A^2
  double Rx;   // Received Power in dbm
  double BER;  // Bit Error Rate
  double res;  //Responsitivity of Receiver
  double SNR;  //Signal to noise ratio
  double temp;  // Blackbody temp of LED
  double Inttemp; 
  double IntRx;
  double Intres; 
  double IntNo;    
  double INR;  

  double calculateBER ();

  void calculateEb();

  double SpectralRadiance(int wavelength, double temperature);

  double integralLum(int lower, int upper);

  double integralPlanck(int lower, int upper);

  double integralRes(int lower, int upper);

  double getTemperature();

  void setNo (int lower, int upper, int T ,double n, double a, double rx);

   void setIntNo (int lower, int upper, int T ,double n, double a , double rx);

  double getEb(void);

  double getBER(void);
  
  double getNo(void);

  double getSNR(void);
  double getINR(void);
   
private:

virtual bool DoCorrupt(Ptr<Packet> p);

virtual void DoReset(void);


double m_rate;



Ptr<RandomVariableStream> m_ranvar;
  
};


} // namespace ns3
#endif
