
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

  static void setNo (double n);

  static void setRx (double x);

  static void setRb (double b);

  static void setRes (double r);

  static double getEb(void);

  static double getBER(void);

   
private:

virtual bool DoCorrupt(Ptr<Packet> p);

virtual void DoReset(void);

virtual bool DoCorruptPkt(Ptr<Packet> p);

virtual bool DoCorruptByte(Ptr<Packet> p);

virtual bool DoCorruptBit(Ptr<Packet> p);

enum ErrorUnit m_unit;

double m_rate;

Ptr<RandomVariableStream> m_ranvar;
  
};


} // namespace ns3
#endif
