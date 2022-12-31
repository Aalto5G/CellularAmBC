/***************************************************************************
*
* reading USRP samples 
*
****************************************************************************/

/*
  Kalle Ruttik
  21.09.2022
 */

#pragma once

#include "Headers.hpp"
#include <log4cxx/logger.h>

class Hardware
{
  public:
  // w_args     - device id or ethernet address 
  // w_dl_freq  - rx frequency
  // w_rx_rate  - rx sampling rate 
  // w_rx_gain  - rx amplifier gain	

  Hardware(uhd::device_addr_t w_args, double w_dl_freq, double w_spectrum_downscale, float w_rx_gain):
	m_dl_freq(w_dl_freq), m_spectrum_downscale(w_spectrum_downscale), m_rx_gain(w_rx_gain)
	{
	
	    m_subframe_size = (uint64_t) ((double)tmp_buffer_size)/m_spectrum_downscale;
	    m_tmp_buff.resize(m_subframe_size);
	    m_tmp_buffs.push_back(&m_tmp_buff.front());
	
	    m_sampling_rate = m_max_sampling_rate/m_spectrum_downscale;

	    // should be some writing into log file		
	    //std::cout<<m_sampling_rate<<std::endl;
	
	    m_args = w_args;	
	    m_dev_addrs = uhd::device::find(w_args);
	    if (m_dev_addrs.size()==0) std::cout<<"\n No device found!";
    	    m_usrp = uhd::usrp::multi_usrp::make(m_args);

            //usrp->set_rx_antenna("TX/RX");
	    m_usrp->set_rx_antenna("RX2");
            m_usrp->set_rx_agc(false, 0);// disable agc
	    m_usrp->set_rx_gain(m_rx_gain);
	    m_usrp->set_rx_freq((double)m_dl_freq);
	    m_usrp->set_rx_rate(m_sampling_rate);

            uhd::stream_args_t stream_args("fc32", "sc16");
            std::vector<size_t> channel_nums;
            channel_nums.push_back(0);
            stream_args.channels = channel_nums;
	    m_rx_stream = m_usrp->get_rx_stream(stream_args);
	    
	    std::string ref("internal");
           // Lock mboard clocks
           // std::cout << boost::format("Lock mboard clocks: %f") % ref << std::endl;
            m_usrp->set_clock_source(ref);
	    //m_sampling_rate = m_usrp->get_rx_rate();
	    //m_spectrum_downscale = 30.72e6/m_sampling_rate;
	    //m_samples_per_TTI = 30720/m_spectrum_downscale;
	
	};

  ~Hardware()
  {};

  // first intializes the USRP into streaming mode
  // reads out the samples from the RX buffer
  //   the last readout start gives the sample number 
  //   in USRP. All samples numbered by (unsigned long long)
  //
  int startUSRP()
  {

    // reset into streaming mode
    m_cmd = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
    m_usrp->issue_stream_cmd(m_cmd);
    m_rx_stream->issue_stream_cmd(m_cmd);

    float delay =0.5;
    m_cmd.num_samps = tmp_buffer_size;             // hwo many samples will be read
    m_cmd.stream_now = false;                      // no start yet
    m_cmd.time_spec  = uhd::time_spec_t(delay);    // Arbitrary delay before to start streaming.
    m_usrp->set_time_now(uhd::time_spec_t(0.0));   //

    m_cmd = uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS;
    m_usrp->issue_stream_cmd(m_cmd);	
    m_rx_stream->issue_stream_cmd(m_cmd);

    // reads the rx buffer till it is empty
    m_rx_Ticks = 0;
    int iter_inx = 0;
    while(true)
    {
      m_rx_stream->recv(m_tmp_buffs, 1, m_rx_md, m_cmd.time_spec.get_full_secs() + 2.1);
      printf("buffer clearing iteration %i \n",iter_inx);

      if(m_rx_md.error_code == uhd::rx_metadata_t::ERROR_CODE_NONE)
      {
        m_rx_Ticks = m_rx_md.time_spec.to_ticks(m_sampling_rate) + 1;
        break;
      }
      //break;
      iter_inx++;
    }

    std::cout<<std::endl<<"rx_Ticks:"<<m_rx_Ticks<<std::endl;
    std::cout<<"USRP started:"<<std::endl;
    return 0;
  };
  
  // Reads out samples from USRP without doing anything to them
  // used for getting first RX sample to have certain Ticks (count number)
  // w_samples_to_burn - nr of samples read out	
  //
  int burnSamples(int w_samples_to_burn)
  {
    int samples_to_burn = w_samples_to_burn;
	  
	  
    // expected Ticks after burning samples 	  
    m_rx_Ticks_expected = m_rx_Ticks + w_samples_to_burn;
    //std::cout<<"w_samples_to_burn:"<<w_samples_to_burn<<" m_rx_Ticks:"<<m_rx_Ticks<<" expected ticks:"<<m_rx_Ticks_expected<<" buffer size:"<<m_tmp_buff.size()<<std::endl;
    
    // readout buffer size
    if(m_tmp_buff.size() == 0)
      m_tmp_buff.resize(m_subframe_size/2);	  	
    
    auto lenTmpBuffSize = m_tmp_buff.size();

    // iterates till the number of read out samples is the same as w_samples_to_burn 
    //    return 0
    // If the cout get messed up (like lost packet 
    //    return 1
    unsigned long long init_rx_Ticks = m_rx_Ticks;
    int tmpSamps = 0;
    int tmp_buffer_size = m_tmp_buff.size();
    while((w_samples_to_burn-tmpSamps) > 0)
      {
        if(tmp_buffer_size > (w_samples_to_burn-tmpSamps))
	  {
	    tmp_buffer_size = w_samples_to_burn-tmpSamps;
	  }
        auto num_rx_samps = m_rx_stream->recv(m_tmp_buffs, tmp_buffer_size, m_rx_md, m_cmd.time_spec.get_full_secs() + 0.0001);		  
	 
        if(num_rx_samps>0)
	  {
	    m_rx_Ticks = (m_rx_md.time_spec.to_ticks(m_sampling_rate)+num_rx_samps);
	    tmpSamps = m_rx_Ticks - init_rx_Ticks;
	  }
	  std::cout<<" Ticks:"<<m_rx_Ticks<<"  tmp-samps:"<<tmpSamps<<" received samps:"<<num_rx_samps<<std::endl;
	  	
      }
    std::cout<<" m_rx_Ticks"<<m_rx_Ticks<<" expected ticks:"<<m_rx_Ticks_expected<<std::endl;

    // something went wrong
    if(m_rx_Ticks_expected != m_rx_Ticks)
      return 1;
    
    // we are good
    return 0;
   };
    
    
    // gets buffer full of IQ samples 
    // o_buff     - sample buffer 
    // number_of_samples_to_be_received
    
    /*
    Return state
     - No error 
         return 0
     - samples_lost_in_the_middle
         return 1
     - samples_lost_over_packet_border !!NOT HANDLING FOR NOW
         return 2
    */

  int getSamples(std::vector<cFloat> &o_buff,int number_of_samples_to_be_received)
  {
    //std::cout<<&o_buff<<" buffer to get size:"<<number_of_samples_to_be_received<<std::endl;

    // TODO: handle the case when some packets are lost.

    // the ticks count after successful reception of samples
    m_rx_Ticks_expected = m_rx_Ticks + number_of_samples_to_be_received;

    // buffer will fit all the samples
    if(o_buff.size()<number_of_samples_to_be_received)
      {
        o_buff.resize(number_of_samples_to_be_received);
      }	
    std::fill(&o_buff[0],&o_buff[0]+number_of_samples_to_be_received, (cFloat)(0.0, 0.0));

	auto samples_to_be_received = number_of_samples_to_be_received;
	if(m_tmp_buff.size()!=samples_to_be_received)
		m_tmp_buff.resize(samples_to_be_received);
		
    // reads number_of_samples_to_be_received IQ samples from  from USRP frontend into o_buffer 	
    unsigned long long init_rx_Ticks = m_rx_Ticks;
    int tmpSamps = 0;

    int expected_location_in_the_frame =0;
    bool some_samples_lost = false;

    while(tmpSamps < samples_to_be_received)
      {
	 auto num_rx_samps = m_rx_stream->recv(m_tmp_buffs, samples_to_be_received - tmpSamps, m_rx_md, m_cmd.time_spec.get_full_secs() + 0.0001);

         unsigned long long tmp_rx_Ticks = m_rx_md.time_spec.to_ticks(m_sampling_rate);
		 	  
         if(num_rx_samps>0)
         {
           // ideally the Ticks count should align with the location in the frame
           int location_in_the_frame = tmp_rx_Ticks-init_rx_Ticks;

           // it seems sometimes the counter in USRP is reset -> we use tmpSamps then
           if(location_in_the_frame < 0)
		  location_in_the_frame = tmpSamps;

	    expected_location_in_the_frame += num_rx_samps;
	    if(expected_location_in_the_frame!=location_in_the_frame)
             {
               some_samples_lost = true;
             }

           // if no packet/smaples lost the location/amount of received samples is always less that required amount    
           if(location_in_the_frame < samples_to_be_received)
             {
               // the samples count does not go over the packet border 
               if( (location_in_the_frame+num_rx_samps)<= samples_to_be_received)
		  {
                  //std::copy(&m_tmp_buffs[0][0],&m_tmp_buffs[0][num_rx_samps],&o_buff[tmpSamps]);
                  std::copy(&m_tmp_buffs[0][0],&m_tmp_buffs[0][num_rx_samps],&o_buff[location_in_the_frame]);
                  }
               else
               {
                 int num_rx_samples_copied = samples_to_be_received - location_in_the_frame;
                 std::copy(&m_tmp_buffs[0][0],&m_tmp_buffs[0][num_rx_samples_copied],&o_buff[location_in_the_frame]);  		      
               }
             }	

             // new timing moment
             m_rx_Ticks = tmp_rx_Ticks + num_rx_samps;
             unsigned long long rx_TicksOld = m_rx_Ticks; 

	     tmpSamps +=num_rx_samps; 
	   }
    }

    // std::cout<<m_rx_Ticks<<std::endl;

    if(some_samples_lost == true)
      return 1;
    if(m_rx_Ticks_expected < m_rx_Ticks)
      return 2;
    return 0;	
   };


   // current Ticks count 
   unsigned long long getRxTicks()
   {
      return m_rx_Ticks;
   }	

public:
  /**********************************************************************/
  size_t m_txrx_chain_delay = 0;


   uhd::device_addr_t m_args; // =  (uhd::device_addr_t)"serial=308F980";
   uhd::device_addrs_t m_dev_addrs;

   /*******************************************************/
   // Create USRP structure 
   //uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(dev_addrs[0]);
   uhd::usrp::multi_usrp::sptr m_usrp;
   uhd::rx_streamer::sptr m_rx_stream;

   uhd::stream_cmd_t m_cmd = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
   uhd::rx_metadata_t m_rx_md;

   //usrp = uhd::usrp::multi_usrp::make(args);

   //usrp->set_rx_antenna("TX/RX");	
   float m_tx_gain = 0;
   float m_rx_gain = 0;
   double m_dl_freq;
   double m_max_sampling_rate = 30720e3;
   double m_sampling_rate;
   double m_spectrum_downscale;
   uint64_t tmp_buffer_size = 30720;
   uint64_t m_subframe_size = 30720;
   
  unsigned long long m_rx_Ticks = 0;
  unsigned long long m_rx_Ticks_expected; 
  

  std::vector<std::complex<float>> m_tmp_buff;
  std::vector<std::complex<float>*> m_tmp_buffs;
  
  };
  
