
#ifndef A_ERROR_MODEL_H
#define A_ERROR_MODEL_H

#include <list>
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/error-model.h"


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

  static void setNo (double n);

  static void setEb (double e);

  //static double N0;
  
private:

virtual bool DoCorrupt(Ptr<Packet> p);

virtual void DoReset(void);


  
};


} // namespace ns3
#endif
