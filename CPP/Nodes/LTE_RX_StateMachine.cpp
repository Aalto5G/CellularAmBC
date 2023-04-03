/*
  LTE receiver and the main interface to USRP 
  Kalle Ruttik
  21.09.2022
 */

//  g++ -std=c++17 LTE_RX_StateMachine.cpp -lfftw3f -lfftw3 -luhd -lboost_system -o rxSTM
//  run the code by
//  ./rxSTM -f 486e6 -s 327AAA6 -g 45

// #include "Headers.hpp"
// #include "Hardware.hpp"

/*

  Class for handling USRP samples 
  
  Kalle Ruttik
  21.09.2022
 */

#include <log4cxx/logger.h>
// #include "Hardware.hpp"
#include "../RAN/PHY/Headers.hpp"
#include "../RAN/PHY/Hardware.hpp"
#include "../RAN/PHY/SSS.hpp"
#include "../RAN/PHY/PSS.hpp"
#include "../RAN/PHY/Pilots.hpp"
#include "../RAN/PHY/SYNC_RX.hpp"
#include "../RAN/PHY/SF.hpp"
#include "../RAN/PHY/OUT_INTERFACE.hpp"
/*
#include "../../Code/pcr_4g/C/RAN/PHY/Headers.hpp"
#include "../../Code/pcr_4g/C/RAN/PHY/Hardware.hpp"
#include "../../Code/pcr_4g/C/RAN/PHY/SSS.hpp"
#include "../../Code/pcr_4g/C/RAN/PHY/PSS.hpp"
#include "../../Code/pcr_4g/C/RAN/PHY/Pilots.hpp"
#include "../../Code/pcr_4g/C/RAN/PHY/SYNC.hpp"
#include "../../Code/pcr_4g/C/RAN/PHY/SF.hpp"
#include "../../Code/pcr_4g/C/RAN/PHY/OUT_INTERFACE.hpp"
*/


using namespace std;
 

/***********************************************************************
 * Signal handlers
 **********************************************************************/
static bool stop_signal_called = false;
void sig_int_handler(int){stop_signal_called = true;}

/***********************************************************************
 * Main function
 **********************************************************************/
int main(int argc, char* argv[])
{

      int retVal = EXIT_SUCCESS;

      // double txRate;
      // double rxRate;

    /******************************************************************/

      int option = 0;
     //double tx_freq = 2.48e9;
     // double dl_freq = 486e6;
     //double dl_freq = 681e6;
     // double dl_freq = 796e6; // DNA&Telia 796 Telia 806 Elisa 816
     double dl_freq = 806e6;
     // double dl_freq = 816e6;
      double spectrum_downscale = 2.0;
      int fftSize = 2048/spectrum_downscale;
      int syncSearchSize = fftSize*8;
      double sampling_rate = 30.72e6/spectrum_downscale; //15.36e6; //30.72e6;
      float rx_gain = 45;
      size_t channel = 0;
      char* device_args;
      char error_string[512];      

  
     // sync
     // parameters 
     int fft_size = 1024;
     int fft_size_for_sync = 8*1024;

     //uhd::device_addr_t args =  (uhd::device_addr_t)"serial=308F980";
     uhd::device_addr_t args =  (uhd::device_addr_t)"serial=327AAA6";
     // uhd::device_addr_t args =  (uhd::device_addr_t)"serial=32711E0";
     // uhd::device_addr_t args =  (uhd::device_addr_t)"serial=3131082";

    std::string s = "serial=";
    for (int i = 1; i < argc; ++i)
        {
        if(strcmp(argv[i], "-f") == 0)
          {
          // std::cout << argv[i] <<" freq:"<<argv[i+1]<< "\n";          
          dl_freq = atof(argv[i+1]);          
          i++;
          }
        if(strcmp(argv[i], "-s") == 0)
          {
          // std::cout << argv[i] <<" serial nr:"<<argv[i+1]<< "\n";
          s = s + argv[i+1];
          // std::cout<<s<<std::endl;
          args =  (uhd::device_addr_t)s;          
          i++;
          }
        if(strcmp(argv[i], "-g") == 0)
          {
          // std::cout << argv[i] <<" rx_gain:"<<argv[i+1]<< "\n";
          rx_gain = atof(argv[i+1]);
          std::cout<<s<<std::endl;
          i++;
          }         
        }
        
     std::cout <<" freq:"<< dl_freq<< " usrp serial id: "<<(string) s<< " rx_gain:"<<rx_gain<<"\n";
        

     //
     Hardware hw(args,dl_freq,spectrum_downscale,rx_gain);     
     
     SYNC_RX sync( fft_size, fft_size_for_sync,hw);
     // Pilots p;
     // SF sf(&sync);
     SF sf;
     OUT_INTERFACE out;

     // make the data vector to be sent over UDP
     int msg_length = 1 + (2 + 7*sizeof(double))*20;

     char msg[msg_length];
     memset(&msg[0],0,msg_length);
     uint8_t tmpHeader = 0;
     tmpHeader =(1<<1)&0xF; 
     msg[0]=tmpHeader;
     
     SYNC_STATE tmpStateVar = SYNC_STATE::CELL_SEARCH;
          
     int onePacketLen = 2+7*sizeof(double);
     char msgTmp[onePacketLen];

     
     std::signal(SIGINT, &sig_int_handler);
     std::cout << "Press Ctrl + C to stop streaming..." << std::endl;

     sync.startUSRP();
     while(true)
     {
     if (stop_signal_called) break;

       switch(sync.getSyncState())
       // switch(tmpStateVar)
      {
        case SYNC_STATE::CELL_SEARCH:
        {
          sync.cellSearch();
          //  tmpStateVar = SYNC_STATE::CELL_ID_SEARCH;
          //  tmpStateVar = sync.getSyncState();
          break;
        }
        case SYNC_STATE::CELL_ID_SEARCH:
        {
        
          sync.cellIdSearch();
          sf.initRx(sync.getCellID(),sync.getNRBDL());
          // tmpStateVar = sync.getSyncState();
          break;
        }
        case SYNC_STATE::CELL_TRACKING:
        {
        
          sync.getSF();
      int tmpFrameInx = sync.getFrameNumber();
      int tmpSlotInx = sync.getSlotNumber();
          sf.receiveSF(sync.getFrameNumber(),sync.getSlotNumber(),sync.getNRBDL(), sync.getSFStart(), sync.getFFTSize());
      // copy received data into tx msg
      msgTmp[0]=(uint8_t)tmpFrameInx;
      msgTmp[1]=(uint8_t)tmpSlotInx;
      memcpy(&msgTmp[2],sf.getChannelCoeff(),7*sizeof(double));

      memcpy(&msg[1+(tmpFrameInx*2+tmpSlotInx)*onePacketLen],&msgTmp[0],onePacketLen);
     	  
      if(tmpFrameInx ==9 & tmpSlotInx == 1)
       {
         out.sendData(msg,msg_length);
         memset(&msg[0],0,msg_length);
         msg[0]=tmpHeader;
         std::cout<<" data sent out:"<< tmpFrameInx<<" "<< tmpSlotInx<<std::endl;
       }  
      // tmpStateVar = sync.getSyncState();
      break;     	     
          }
        // default: 
        //  tmpStateVar = SYNC_STATE::CELL_ID_SEARCH;
       }
     }

     return retVal;
}
