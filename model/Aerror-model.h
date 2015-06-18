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

  AErrorModel ();      //Constructor
   ~AErrorModel ();    //Deconstructor

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void); //Method for all NS 3 objects sets up traces and such 
                                  //Not really used for Aerror-model

  static double calculateBER (); //Calculate Bit Error Rate

  static void setNo (double n); //sets Noise Power

  static void setRx (double x); //sets Received Power

  static void setRb (double b); //Sets bit rate and sets and inverse

  static void setRes (double r); //sets Responcivity 

  static double getBER(void); //return BER
  
  static double getNo(void); //return Noise Power

  static double getSNR(void); //return SNR

   
private:
//Virtual Methods need for Error model
virtual bool DoCorrupt(Ptr<Packet> p); //Determines what pacekts are corrupt

virtual void DoReset(void); // Not used

};


} // namespace ns3
#endif
