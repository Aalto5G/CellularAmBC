/*************************************************************************
*
* Matches IQ samples to the frame structure in BS 
* keeps track of frame and subframe numbers
* 
**************************************************************************/

#pragma once 

#include "Headers.hpp"

// class doing initial synchronization and later tracking
class SYNC_RX{
	public:
	// w_fft_size         - fft_size (max 2048 for 20MHz transmission) if less than max spectrum is scaled down
	// w_search_fft_size  - how large fft size is used for searching PSS. Seach is done by correlation is spectral domain
	// w_hw               - instance handling samples transmission to/from USRP
	//
	SYNC_RX(int w_fft_size, int w_search_fft_size, Hardware &w_hw):m_fft_size(w_fft_size),m_search_fft_size(w_search_fft_size),m_hw(w_hw)
	{
	
	 m_spectrum_scaling = 2048/m_fft_size;
	 
	 m_subframe_size = 30720/m_spectrum_scaling;
	 // m_slot_size = 30720/2/m_spectrum_scaling;
 
	 for( int i1=0; i1<7; i1++)
 	   cp[i1] = cp_max[i1]/m_spectrum_scaling; 
	 
	 m_pss = new PSS(m_fft_size,m_search_fft_size);
	 
	 m_rx_long.resize(m_search_fft_size);
	 
	 m_tmp_subframeT = new cFloat*[7];
	 m_tmp_subframeF = new cFloat*[7];
	  for(int i1 =0;i1<7;i1++)
	  {
		m_tmp_subframeF[i1]= new cFloat[m_fft_size];
		m_tmp_subframeT[i1]= new cFloat[m_fft_size];
	  }
	 m_fft_SF_T2F=(fftwf_plan*) malloc(sizeof(fftwf_plan*)*7);
	  for(int i1 =0;i1<7;i1++)
	  {
      		m_fft_SF_T2F[i1] = fftwf_plan_dft_1d(m_fft_size,(fftwf_complex*)&m_tmp_subframeT[i1][0],(fftwf_complex*)&m_tmp_subframeF[i1][0],FFTW_FORWARD,FFTW_ESTIMATE);
	  }
	 	 
	};
	~SYNC_RX()
	{
		delete(m_pss);
		for(int i1=0;i1<7;i1++)
		{
			delete(m_tmp_subframeT[i1]);
			delete(m_tmp_subframeF[i1]);
			fftwf_destroy_plan(m_fft_SF_T2F[i1]);
		}
		delete(m_tmp_subframeT);
		delete(m_tmp_subframeF);
		free(m_fft_SF_T2F);
	};
	
	int startUSRP()
	{
	  m_hw.startUSRP();	
	  return 0;
	};

	int cellSearch()
	{

      int decLevel = 7; // arbitrary level for deciding that there is correlation
	  int maxLocation=-1;

	  m_rx_long.resize(m_search_fft_size);

      m_hw.getSamples(m_rx_long,m_search_fft_size);
	
      // correlate
	  
	  // FFT into freq domain
	  m_pss->cellSearchRxFFT(&m_rx_long[0]);

      
      if(m_candidate_seq_inx == -1)
	  // no candidate yet correlate with all the sequences 
	  {

        int tmp_max = 0;
        for(int inx =0; inx < 3; inx++)
	    {
		  
	      // Correlation for PSS seq_inx and search for the max
	      m_pss->cellSearchCorrelateAndGetMaxForOneSeq(inx);
		  if( m_pss->getMaxCorrValue(inx) > decLevel)
		    {
			  if( m_pss->getMaxCorrValue(inx) > tmp_max)
			  {
	  	        maxLocation = m_pss->getMaxCorrValueLocation(inx);

			    m_candidate_seq_inx = inx;
                m_startsCount = 0;  
	  	        m_oldPeakLocation = (int)(m_hw.getRxTicks() - (m_search_fft_size - maxLocation));
	  	        m_startsCount++;
				tmp_max = m_pss->getMaxCorrValue(inx);
			  }
		    }
	    }
	  }
	  else
	  {
		int seq_inx = m_candidate_seq_inx;
	    m_pss->cellSearchCorrelateAndGetMaxForOneSeq(seq_inx);
	
	    // std::cout<<"found a candidate:"<<m_pss->getMaxCorrValue(seq_inx)<<std::endl;
	    if(m_pss->getMaxCorrValue(seq_inx)>decLevel)
	    {
	  	  maxLocation = m_pss->getMaxCorrValueLocation(seq_inx);

		   std::cout<<"found a candidate group :"<<seq_inx<<" max value:"<<m_pss->getMaxCorrValue(seq_inx)<<" location:"<<maxLocation<<std::endl;
		
		  int currentPeakLoc = (int)(m_hw.getRxTicks()  -(m_search_fft_size - maxLocation));
		  int decVariable = currentPeakLoc - m_oldPeakLocation;
		  if( abs(decVariable - 76800) > 5*m_spectrum_scaling)
               // resets counter
		  {
            m_startsCount=0;
			m_candidate_seq_inx = -1;
			 std::cout<<"candidate not good"<<std::endl;
		  }
		  else
                // everything fine 
		  {
		    m_startsCount++;
		  }
		  m_oldPeakLocation = currentPeakLoc;
		}
/*		else if(((m_hw.getRxTicks()+m_search_fft_size)-m_oldPeakLocation) > (2*(76800 +5*m_spectrum_scaling)))
		{
            m_startsCount=0;
			m_candidate_seq_inx = -1;
			//std::cout<<"candidate not good"<<std::endl;
		}
*/
	  }

      
      if(m_startsCount>2)
      {
	    // PSS id set
	    N_id_1 = m_candidate_seq_inx;

        // align peak to be in the right location 
	    // burn samples till next subframe with PSS such that the starting sample is in beginning of the SF
	    int tmpLocations = m_search_fft_size - maxLocation;
	    //int samples_per_subframe = m_samples_per_TTI/2;
	    int m_samples_per_TTI = 30720/m_spectrum_scaling;
	    int samples_to_burn = (m_samples_per_TTI/2*9+m_fft_size)-tmpLocations;
	    unsigned long long expected_rx_Ticks = m_hw.getRxTicks() + samples_to_burn;
	   
	    m_hw.burnSamples(samples_to_burn);	
	    if(m_hw.getRxTicks() != expected_rx_Ticks)
	    {
	  	  samples_to_burn = m_samples_per_TTI/m_spectrum_scaling*10 - (m_hw.getRxTicks() -expected_rx_Ticks);
	  	  m_hw.burnSamples(samples_to_burn);	
	    }
	  
 	    int subframe_size = 30720/2;
	    m_rx_long.resize(subframe_size);

	    std::cout<<"found a candidate group:"<<N_id_1<<std::endl;


	    m_sync_state = SYNC_STATE::CELL_ID_SEARCH;
	    m_cell_search = CELL_SEARCH_STATE_PSS::PSS_FOUND;
	    m_cell_id_search = CELLID_SEARCH_STATE_SSS::SSS_SEARCH;

	    // resets the search temporal parameters.
        m_startsCount = 0;
		m_candidate_seq_inx = -1;
	  }
	  else 
	  {
		m_sync_state = SYNC_STATE::CELL_SEARCH;
	    m_cell_search = CELL_SEARCH_STATE_PSS::PSS_NOT_FOUND;
	    m_cell_id_search = CELLID_SEARCH_STATE_SSS::SSS_NOT_FOUND;
	  }

    return 0;
	}


	int cellSearchv2()
	{

	  int m_startsCount = 0;
	  int oldPeakLocation = 0;

	  int maxLocation=-1;
	  int decLevel = 7; // arbitrary level for deciding that there is correlation
	  int decVariable =0;

	  m_rx_long.resize(m_search_fft_size);
	  
	  int seq_inx = 0;

	  bool syncCandidate = true;
	  while(syncCandidate)
	  //for(int i3=0;i3<10;i3++)
	  {
	  // get samples
	  m_hw.getSamples(m_rx_long,m_search_fft_size);
	  
	  // correlate
	  // FFT into freq domain
	  m_pss->cellSearchRxFFT(&m_rx_long[0]);


	  // Correlation for PSS seq_inx and search for the max
	  m_pss->cellSearchCorrelateAndGetMaxForOneSeq(seq_inx);
	
	  // std::cout<<"found a candidate:"<<m_pss->getMaxCorrValue(seq_inx)<<std::endl;
	  if(m_pss->getMaxCorrValue(seq_inx)>decLevel)
	  {
	  	maxLocation = m_pss->getMaxCorrValueLocation(seq_inx);
	  	
               std::cout<<"found a candidate group :"<<seq_inx<<" max value:"<<m_pss->getMaxCorrValue(seq_inx)<<" location:"<<maxLocation<<std::endl;
         
               // startsCount defines how many times the correlation peak has been in expected location      
	  	if(m_startsCount==0)
	  	{
	  	  oldPeakLocation = (int)(m_hw.getRxTicks() - (m_search_fft_size - maxLocation));
	  	  m_startsCount++;
	  	}
	  	else
	  	{
	  	int currentPeakLoc = (int)(m_hw.getRxTicks()  -(m_search_fft_size - maxLocation));
		decVariable = currentPeakLoc - oldPeakLocation;
		if( abs(decVariable - 76800) > 5*m_spectrum_scaling)
               // resets counter
		{
                 m_startsCount=0;
		}
		else
                // everything fine 
		{
		  m_startsCount++;
		}
		oldPeakLocation = currentPeakLoc;
	  	}
               //std::cout<<" starts count:"<<m_startsCount<< " PeakLoc:"<<oldPeakLocation<<" decVariable:"<<decVariable<<std::endl;
	  }
        if(m_startsCount>2)
        // there has been peak sufficiently many times in right position
        {
		  syncCandidate = false;
	    }
      }

	  // PSS id set
	  N_id_1 = seq_inx;

      // align peak to be in the right location 
	  // burn samples till next subframe with PSS such that the starting sample is in beginning of the SF
	  int tmpLocations = m_search_fft_size - maxLocation;
	  //int samples_per_subframe = m_samples_per_TTI/2;
	  int m_samples_per_TTI = 30720/m_spectrum_scaling;
	  int samples_to_burn = (m_samples_per_TTI/2*9+m_fft_size)-tmpLocations;
	  unsigned long long expected_rx_Ticks = m_hw.getRxTicks() + samples_to_burn;
	   
	  m_hw.burnSamples(samples_to_burn);	
	  if(m_hw.getRxTicks() != expected_rx_Ticks)
	  {
	  	samples_to_burn = m_samples_per_TTI/m_spectrum_scaling*10 - (m_hw.getRxTicks() -expected_rx_Ticks);
	  	m_hw.burnSamples(samples_to_burn);	
	  }
	  
	  int subframe_size = 30720/2;
	  m_rx_long.resize(subframe_size);

	  m_sync_state = SYNC_STATE::CELL_ID_SEARCH;
	  m_cell_search = CELL_SEARCH_STATE_PSS::PSS_FOUND;
	  m_cell_id_search = CELLID_SEARCH_STATE_SSS::SSS_SEARCH;

	  // resets the search temporal parameters.

	  return 0;
	};
	
	int cellIdSearch()
	{
	  
	  m_subframe_size = m_subframe_size/2;
	  //m_rx_long.resize(subframe_size);
	  
 	  //m_rx_long.resize(1024+80);
          // map to symbols
          int cp_len = cp[0];
          int nr_of_samples;
          for(int i1=0;i1<7;i1++)
          {
          	nr_of_samples = cp_len + m_fft_size;
//          	std::cout<<"sf_reception:"<<nr_of_samples<<std::endl;
          	m_hw.getSamples(m_rx_long,nr_of_samples);
		std::copy(&m_rx_long[cp_len],&m_rx_long[cp_len]+m_fft_size,&m_tmp_subframeT[i1][0]);
		cp_len=cp[i1];          
          }

	int frame_number = -1;
	int estSequence = -1;
	int maxLoc = -1;
	
	m_pss->cellTracking(&m_tmp_subframeT[6][0],0);
        
       double max_value = m_pss->getMaxCorrValue(N_id_1);
       int max_location = m_pss->getMaxCorrValueLocation(N_id_1);

       std::cout<<" pss max:"<< max_value<<" pss loc:"<< max_location<<std::endl;

	m_sss.searchFrameNumberFromCorrWithSSS(&m_tmp_subframeT[5][0], N_id_1, &frame_number , &estSequence, &maxLoc);
	std::cout<<"N_id_1:"<<N_id_1<<" estimated frameNr"<<frame_number <<" Estimated SSS sequence Nr:"<<estSequence<<" Maximum Location in Time:"<<maxLoc<<std::endl;

	cell_id = estSequence;
	N_id_2 = (cell_id-N_id_1)/3;

       // has to assign slot_count and slot_number by +1 since we handle slot 0 at moment
	m_frame_number = frame_number;
	m_slot_number = 1;
	m_slot_count = frame_number*2+m_slot_number;
	std::cout<<"frame nr:"<<m_frame_number<<std::endl;
	
	m_sync_state = SYNC_STATE::CELL_TRACKING;
	m_cell_id_search = CELLID_SEARCH_STATE_SSS::SSS_FOUND;
		
	return 0;
	};
	
	int getSF()
	{
	  // getting SF 
	  m_subframe_size = m_subframe_size/2;
	  //m_rx_long.resize(subframe_size);
	  
 	  //m_rx_long.resize(1024+80);
          // map to symbols
          // int cp_len = cp[0]; //80;
          
          int nr_of_samples;
          for(int i1=0;i1<7;i1++)
          {
            int cp_len = cp[i1];
            if( (m_tracking_adjustment != 0) & (i1 ==0))
            {
              cp_len = cp[i1] + m_tracking_adjustment;
              m_tracking_adjustment = 0;
            }
            else
            {
             cp_len = cp[i1];
            }


            nr_of_samples = cp_len + m_fft_size;
            //std::cout<<"sf_reception:"<<nr_of_samples<<std::endl;
            m_hw.getSamples(m_rx_long,nr_of_samples);
	    std::copy(&m_rx_long[cp_len],&m_rx_long[cp_len]+m_fft_size,&m_tmp_subframeT[i1][0]);
	    fftwf_execute(m_fft_SF_T2F[i1]);   
          }

          // TODO: Tracking adjustment is based on the correlation with the RX signal

	  // does not work for now 

	  if( this->isPSSSubframe() == true)
	  {
	   // correlation
	   // peak search 
	   // delta peak difference 
           m_pss->cellTracking(&m_rx_long[0],N_id_1);
           // m_pss->cellTracking(&m_tmp_subframeT[6][0],0);
          
           double max_value = m_pss->getMaxCorrValue(N_id_1);
	   int max_location = m_pss->getMaxCorrValueLocation(N_id_1);
	    
	   m_tracking_adjustment = max_location - cp[6]+1;

           // if correlation less than decision level and location is totally off 
           // assume we lost the cell and should go into search if that is too many times
           if((max_value<10) & (m_tracking_adjustment > 20))
           {
             m_cell_lost++;
             if(m_cell_lost>10)
             {
               std::cout<<" we are lost"<<std::endl;
               m_sync_state = SYNC_STATE::CELL_SEARCH;
               m_cell_lost = 0;
             }
           }
           else
           {
             m_cell_lost = 0;
           }
           // tmp hack since tracking does not work
           if(m_tracking_adjustment >0)
             m_tracking_adjustment = 1 ;
           if(m_tracking_adjustment <0)
             m_tracking_adjustment = -1;
             
	   //std::cout<<" N_id_1:"<<N_id_1<<" Tracking max_v: "<<max_value<<" max_loc: "<<max_location<<" adjustment: "<< m_tracking_adjustment;
	   //std::cout<<" frame nr: "<<m_frame_number<<" slot nr: "<<m_slot_number<<" slot count: "<<m_slot_count<<std::endl;

	   }
	  
	  // Update SF counters 
	  m_slot_number = this->updateSlotNumberByOneStep();
	  m_frame_number = this->updateFrameNumberByOneStep();
	  m_slot_count = (m_slot_count%20);

         //std::cout<<"frame nr:"<<m_frame_number<<" slot nr:"<<m_slot_number<<" slot count"<<m_slot_count<<std::endl;


	  // if no sync set counter for going to cell search. 

	  return 0;
	};	
	
	//
	int updateSlotNumberByOneStep()
	{
	  m_slot_number++;
	  m_slot_number = (m_slot_number%2);
	  m_slot_count++;
	  return m_slot_number;
	};
	
	//
	int updateFrameNumberByOneStep()
	{
	  if(m_slot_number == 0)
	    {		  
	      m_frame_number++;
	      m_frame_number = (m_frame_number%10);
	    } 
	  return m_frame_number;
	};


	/////////////////////////////////////////////////////////////////////
	// various internal parameters 	
	//
	int getNRBDL()
	{
		return NRBDL;
	};
	int getCellID()
	{
		return cell_id;
	};

	int getN_id_1()
	{
		return N_id_1;
	};
	int getN_id_2()
	{
		return N_id_2;
	};
	int getFrameNumber()
	{
		return m_frame_number;
	};
	int getSlotNumber()
	{
		return m_slot_number;
	};
	
	// returns pointer to the fft of the sybmol w_symbol_nr
	cFloat** getSFStart()
	{
		return m_tmp_subframeF;
	};

	cFloat* getSFSymbolfreqComponents(int w_symbol_nr)
	{
		return &m_tmp_subframeF[w_symbol_nr][0];
	};
	
	int getFFTSize()
	{
		return m_fft_size;
	};


	bool isPSSSubframe()
	{

//          if(m_slot_count == 0)
//	  if(((m_frame_number == 2) & (m_slot_number ==0)))
	  if(((m_frame_number == 0) & (m_slot_number ==0))
            || ((m_frame_number == 5) & (m_slot_number ==0)))
	  {
	    return true;
	  }
	  return false;
	};

  SYNC_STATE getSyncState()
  {
		return m_sync_state;
	};
  CELL_SEARCH_STATE_PSS getCellSearchState()
  {
		return m_cell_search;
	};
	
  CELLID_SEARCH_STATE_SSS getCellIDSearchState()
  {
		return m_cell_id_search;
	};
	
	public:
	Hardware m_hw;
	int m_fft_size = 2048;
	int m_search_fft_size = 2048*8;
	
	std::vector<std::complex<float>> m_rx_long;
	
	PSS* m_pss;
	SSS m_sss;
	int NRBDL = 50;
	
	int N_id_1 = -1;
	int N_id_2 = -1;
	int cell_id = -1;
	cFloat **m_tmp_subframeT;
	cFloat **m_tmp_subframeF;
	int m_frame_number=-1; // 0 to 9
	int m_slot_number=-1;  // 0 or 1
	int m_slot_count=-1;   // slot nr in frame

	int m_tracking_adjustment = 0;

	// internal trackers
	int m_subframe_size = 30720;
	int m_slot_size = 30720/2;
	int m_spectrum_scaling = 1;
	
	int cp_max[7] = {160,144,144,144,144,144,144}; 		
	int cp[7] = {160,144,144,144,144,144,144}; 	
	
	fftwf_plan *m_fft_SF_T2F;
	
	// states 
	int m_cell_lost = 0; // tracks if sync is lost

    int m_startsCount = 0;
	int m_candidate_seq_inx = -1;
	int m_oldPeakLocation = 0;
	SYNC_STATE m_sync_state = SYNC_STATE::CELL_SEARCH;
	CELL_SEARCH_STATE_PSS m_cell_search = CELL_SEARCH_STATE_PSS::PSS_NOT_FOUND;
	CELLID_SEARCH_STATE_SSS m_cell_id_search = CELLID_SEARCH_STATE_SSS::SSS_NOT_FOUND;
	
};

