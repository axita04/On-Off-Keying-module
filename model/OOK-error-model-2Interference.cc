
/* 
 *
 * Author: Ryan Ackerman   <rea9@njit.edu>
 *         
 *         
*/

#include <cmath>

#include "ns3/OOK-error-model-2Interference.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include <time.h>
#include <stdlib.h>


namespace ns3 {


//
// OOKErrorModel
//
NS_LOG_COMPONENT_DEFINE("OOK2IntErrorModel");


NS_OBJECT_ENSURE_REGISTERED (OOK2IntErrorModel);


double OOK2IntErrorModel::V_lambda[] = {
0.000039, 0.000120, 0.000396, 0.001210, 0.004000, 0.011600, 0.023000, 
0.038000, 0.060000, 0.090980, 0.139020, 0.208020, 0.323000,  0.503000, 
0.710000, 0.862000, 0.954000, 0.994950,  0.995000, 0.952000, 0.870000, 
0.757000, 0.631000, 0.503000, 0.381000, 0.265000, 0.175000, 0.107000, 
0.061000, 0.032000, 0.017000, 0.008210, 0.004102, 0.002091, 0.001047, 
0.000520, 0.000249, 0.000120, 0.000060, 0.000030 };

double OOK2IntErrorModel::Response[] = { 
0.150, 0.160, 0.170, 0.190, 0.200, 0.220, 0.230, 0.240, 0.250, 0.260, 
0.270, 0.280, 0.300, 0.320, 0.330, 0.350, 0.360, 0.370, 0.375, 0.380, 
0.390, 0.400, 0.415, 0.420, 0.430, 0.440, 0.450, 0.460, 0.470, 0.475,
0.480, 0.485, 0.490, 0.495, 0.500, 0.505, 0.510, 0.520, 0.526, 0.532 };

//Constructor
OOK2IntErrorModel::OOK2IntErrorModel ()
{
  NS_LOG_FUNCTION (this);
  srand(time(NULL));  //Seeds Random Number Generator
}

OOK2IntErrorModel::~OOK2IntErrorModel () 
{
  NS_LOG_FUNCTION (this);
}

TypeId OOK2IntErrorModel::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::OOK2IntErrorModel")

    .SetGroupName("Network")
    .AddConstructor<OOK2IntErrorModel> ()  
     ;
  return tid;
}

double OOK2IntErrorModel::SpectralRadiance( int wavelength, double temperature){
        double spectral_rad;
        double h = 6.62606957e-34; //Planck's constant
        double c = 299792458;      //speed of light
        double k = 1.3806488e-23;  //Boltzmann constant
        double waveLength = wavelength * 1e-9; //nm
        return spectral_rad = 15*((std::pow((h*c)/(M_PI*k*temperature), 4)))/((std::pow(waveLength, 5)) * ((std::exp((h*c)/(waveLength*k*temperature)))-1));
}

//Definite integral of the Luminosity Function(wavelength)*Spectral Radiance(wavelength, temperature) d(wavelength)
double OOK2IntErrorModel::integralLum(int lower, int upper){
        double integral = 0;
        int waveLower = lower;
        int waveUpper = upper;

        while(waveLower <= waveUpper)
        {
                integral += V_lambda[(waveLower-380)/10] * SpectralRadiance(waveLower, temp) * 10e-9;
                waveLower += 10;
        }
        
        return integral;
}


//Definite integral of the Spectral Radiance(wavelength, temperature) d(wavelength)
double OOK2IntErrorModel::integralPlanck(int lower, int upper){
        double integral = 0;
        int waveLower = lower;
        int waveUpper = upper;       

        while(waveLower <= waveUpper)
        {
                integral += SpectralRadiance(waveLower, temp) * 10e-9;
                waveLower += 10;
        }
        
        return integral;
}

//Definite integral of the Response(wavelength)*Spectral Radiance(wavelength, temperature) d(wavelength)
double OOK2IntErrorModel::integralRes(int lower, int upper){
        double integral = 0;
        int waveLower = lower;
        int waveUpper = upper;
        while(waveLower <= waveUpper)
        {
                integral += Response[(waveLower-380)/10] * SpectralRadiance(waveLower, temp) * 10e-9;
                waveLower += 10;
        }
        
        return integral;
}


double OOK2IntErrorModel::getTemperature()
{
  return temp;
}

// Virtual method from Error Model.  Determinds what packets to be dropped.
bool OOK2IntErrorModel::DoCorrupt(Ptr<Packet> p){
	NS_LOG_FUNCTION(this << p);
	
	BER = calculateBER();
	//Caculated the Packet Error Rate by finding the complement of the probablility 
	//that a packets is not corrupted
	double per = 1.0 - (double)std::pow((double)(1.0 - BER), static_cast<double>(8*p->GetSize()));
        //Randomizies a number and if its less than the PER the packet is rejected
	double rnd  = (double) rand()/(double)(RAND_MAX);
        return (rnd < per);
}
void OOK2IntErrorModel::DoReset(void){


}

//Calculates BER from SNR
double OOK2IntErrorModel::calculateBER (){
//SNR calculation
SNR = (std::pow((Rx*res),2)/No);
INR = (std::pow((IntRx*Intres),2)/IntNo);

double BER;
if(SNR > 0){
//BER calculation
BER = 0.25*erfc((std::sqrt(SNR) + std::sqrt(INR))/std::sqrt(2)) + 0.25*erfc((std::sqrt(SNR) - std::sqrt(INR))/std::sqrt(2));
}else{
BER = 1;
}


return BER;
}
//Set Noise power

void OOK2IntErrorModel::setNo (double rx, int lower, int upper, int T ,double B, double A){	//B is the Bandwidth of the electrical filter  [b/s] and photodetector Area	[cm^2

       temp = T;
       Rx = rx; 

       res = integralRes(lower, upper)/integralPlanck(lower, upper);
       
   
	double q = 1.60217e-19;	//electronic charge [Coulombs]
	double k = 1.38064e-23;	//Boltzmann constant	[m^2 kg s^-2 K^-1]
	double I2 = 0.5620;	//noise bandwidth factor
	double I3 = 0.0868;	//noise bandwidth factor
	double Ib = 100e-6;	//photocurrent due to background radiation  [microA]
	double Gol = 10;	//open-loop voltage gain
	double Cpd = 112e-12; 	//fixed capacitance of photodetector per unit area  [pF/cm^2]
	double gm = 30e-3;	//FET transconductance	[mS]
	double	gamma = 1.5;	//FET channel noise factor
	double abs_temp = 295;	//Absolute temperature [K]
	double shot_var, thermal_var;

	//shot variance
	shot_var = 2*q*res*Rx*B + 2*q*Ib*I2*B;
       // std::cout<<"RES : " << res << std::endl;        
        //std::cout<<"SHOT : " << shot_var << std::endl;   

	//thermal variance
	thermal_var = ((8*M_PI*k*abs_temp)/Gol)*Cpd*A*I2*(std::pow(B, 2)) + ((16*(std::pow(M_PI, 2))*k*abs_temp*gamma)/gm)*(std::pow(Cpd, 2))*(std::pow(A, 2))*I3*(std::pow(B, 3));
	
	No = shot_var + thermal_var;
}

void OOK2IntErrorModel::setIntNo (double rx, int lower, int upper, int T ,double B, double A){	//B is the Bandwidth of the electrical filter  [b/s] and photodetector Area	[cm^2

       Inttemp = T;
       IntRx = rx; 
       Intres = integralRes(lower, upper)/integralPlanck(lower, upper);
       
   
	double q = 1.60217e-19;	//electronic charge [Coulombs]
	double k = 1.38064e-23;	//Boltzmann constant	[m^2 kg s^-2 K^-1]
	double I2 = 0.5620;	//noise bandwidth factor
	double I3 = 0.0868;	//noise bandwidth factor
	double Ib = 100e-6;	//photocurrent due to background radiation  [microA]
	double Gol = 10;	//open-loop voltage gain
	double Cpd = 112e-12; 	//fixed capacitance of photodetector per unit area  [pF/cm^2]
	double gm = 30e-3;	//FET transconductance	[mS]
	double	gamma = 1.5;	//FET channel noise factor
	double abs_temp = 295;	//Absolute temperature [K]
	double shot_var, thermal_var;

	//shot variance
	shot_var = 2*q*Intres*IntRx*B + 2*q*Ib*I2*B;
       // std::cout<<"RES : " << res << std::endl;        
        //std::cout<<"SHOT : " << shot_var << std::endl;   

	//thermal variance
	thermal_var = ((8*M_PI*k*abs_temp)/Gol)*Cpd*A*I2*(std::pow(B, 2)) + ((16*(std::pow(M_PI, 2))*k*abs_temp*gamma)/gm)*(std::pow(Cpd, 2))*(std::pow(A, 2))*I3*(std::pow(B, 3));
	
	IntNo = shot_var + thermal_var;
}


//Gets Noise power
double OOK2IntErrorModel::getNo(void){
return No;
}
//Gets BER
double OOK2IntErrorModel::getBER(void){
return BER;
}
//Gets SNR
double OOK2IntErrorModel::getSNR(void){
return SNR;
}


} // namespace ns3
