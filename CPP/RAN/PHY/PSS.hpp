/******************************************************************
*
* PSS generation as in LTE 36.211
*
* contains correlation in Fourier domain
*    - for Long sequence in cell search state
*    - for short sequnce in cell tracking state
*
*******************************************************************/
#pragma once 

#include "Headers.hpp"

// class containing PSS sequences and making correlation with them
class PSS{
	public:
	PSS(int w_fft_size, int w_search_fft_size):m_fft_size(w_fft_size),m_search_fft_size(w_search_fft_size)
	{
	
	m_spectrum_scaling = 2048/m_fft_size;
	m_pss_F_conj_search = new cFloat*[3];
	m_pss_F_conj_tracking = new cFloat*[3];
	
	auto tmpInF = new cFloat[m_fft_size];
	auto tmpInT = new cFloat[m_fft_size];
	auto tmpInT_long = new cFloat[m_search_fft_size];
	auto tmpInF_long = new cFloat[m_search_fft_size];
	auto tmpF2T = fftwf_plan_dft_1d(m_fft_size,(fftwf_complex*)&tmpInF[0],(fftwf_complex*)&tmpInT[0],FFTW_BACKWARD,FFTW_ESTIMATE);
	auto tmpT2F_long = fftwf_plan_dft_1d(m_search_fft_size,(fftwf_complex*)&tmpInT_long[0],(fftwf_complex*)&tmpInF_long[0],FFTW_FORWARD,FFTW_ESTIMATE);
	
	for( int i1=0; i1<3; i1++)
        {

	    m_pss_F_conj_tracking[i1] = (cFloat*) fftw_malloc(m_fft_size * sizeof(cFloat));
	    std::fill(&m_pss_F_conj_tracking[i1][0],&m_pss_F_conj_tracking[i1][0]+m_fft_size, (cFloat)(0.0, 0.0));
			
	    std::copy(&m_pss_seq[i1][0],&m_pss_seq[i1][32],&m_pss_F_conj_tracking[i1][m_fft_size-1-30]);
	    std::copy(&m_pss_seq[i1][31],&m_pss_seq[i1][62],&m_pss_F_conj_tracking[i1][1]);

	    std::copy(&m_pss_F_conj_tracking[i1][0],&m_pss_F_conj_tracking[i1][0]+m_fft_size,&tmpInF[0]);
	    
	    fftwf_execute(tmpF2T);

	    std::copy(&tmpInT[0],&tmpInT[0]+m_fft_size,&tmpInT_long[0]);

	    fftwf_execute(tmpT2F_long);
	    	    
	    m_pss_F_conj_search[i1] = (cFloat*) fftw_malloc(m_search_fft_size * sizeof(cFloat));
	    std::fill(&m_pss_F_conj_search[i1][0],&m_pss_F_conj_search[i1][0]+m_search_fft_size, (cFloat)(0.0, 0.0));
	    std::copy(&tmpInF_long[0],&tmpInF_long[0]+m_search_fft_size,&m_pss_F_conj_search[i1][0]);

	    for(int i2=0;i2<m_fft_size;i2++)
		m_pss_F_conj_tracking[i1][i2] = std::conj(m_pss_F_conj_tracking[i1][i2]);

	    for(int i2=0;i2<m_search_fft_size;i2++)
		m_pss_F_conj_search[i1][i2] = std::conj(m_pss_F_conj_search[i1][i2]);

	 }
	 
	 delete(tmpInF);
	 delete(tmpInT);
	 delete(tmpInF_long);
	 delete(tmpInT_long);
	 fftwf_destroy_plan(tmpF2T);
	 fftwf_destroy_plan(tmpT2F_long);
	 

	 m_rx = new cFloat[m_fft_size];
	 m_rxF = new cFloat[m_fft_size];

	 m_corrF = new cFloat*[3];
	 m_corrT = new cFloat*[3];
	 m_corrTAbs = new double*[3];
	 for(int i1=0;i1<3;i1++)
	 {
	   m_corrF[i1] = new cFloat[m_fft_size];
	   m_corrT[i1] = new cFloat[m_fft_size];
	   m_corrTAbs[i1] = new double[m_fft_size];
	}
	 m_rx2F = fftwf_plan_dft_1d(m_fft_size,(fftwf_complex*)&m_rx[0],(fftwf_complex*)&m_rxF[0],FFTW_FORWARD,FFTW_ESTIMATE);

	 m_F2rx = (fftwf_plan*) malloc(sizeof(fftwf_plan*)*3);

	 for(int i1=0;i1<3;i1++)
		 m_F2rx[i1] = fftwf_plan_dft_1d(m_fft_size,(fftwf_complex*)&m_corrF[i1][0],(fftwf_complex*)&m_corrT[i1][0],FFTW_BACKWARD,FFTW_ESTIMATE);

	 
	 m_rx_long = new cFloat[m_search_fft_size];
	 m_rxF_long = new cFloat[m_search_fft_size];


	 m_corrF_long = new cFloat*[3];
	 m_corrT_long = new cFloat*[3];
	 m_corrTAbs_long = new double*[3];
	 for(int i1=0;i1<3;i1++)
	 {
           m_corrF_long[i1] = new cFloat[m_search_fft_size];
	   m_corrT_long[i1] = new cFloat[m_search_fft_size];
	   m_corrTAbs_long[i1] = new double[m_search_fft_size];
	 }
	 m_rx2F_long = fftwf_plan_dft_1d(m_search_fft_size,(fftwf_complex*)&m_rx_long[0],(fftwf_complex*)&m_rxF_long[0],FFTW_FORWARD,FFTW_ESTIMATE);
	 
	 m_F2rx_long = (fftwf_plan*) malloc(sizeof(fftwf_plan*)*3);
	 for(int i1=0;i1<3;i1++)
		 m_F2rx_long[i1] = fftwf_plan_dft_1d(m_search_fft_size,(fftwf_complex*)&m_corrF_long[i1][0],(fftwf_complex*)&m_corrT_long[i1][0],FFTW_BACKWARD,FFTW_ESTIMATE);
	 
	};
	~PSS()
	{
		delete(m_rx);
		delete(m_rxF);
		delete(m_corrF);
		delete(m_corrT);
		delete(m_corrTAbs);

		delete(m_rx_long);
		delete(m_rxF_long);
		delete(m_corrF_long);
		delete(m_corrT_long);
		delete(m_corrTAbs_long);
		
		for( int i1=0; i1<3; i1++)
        	{
        		delete(m_pss_F_conj_tracking[i1]);
        		delete(m_pss_F_conj_search[i1]);
                  fftwf_destroy_plan(m_F2rx[i1]);
                  fftwf_destroy_plan(m_F2rx_long[i1]);

        	}
        	delete(m_pss_F_conj_tracking);
        	delete(m_pss_F_conj_search);
        	free(m_F2rx);
        	free(m_F2rx_long);
        	
                fftwf_destroy_plan(m_rx2F);
                fftwf_destroy_plan(m_rx2F_long);


	};
	
	// makes only FFT of the received samples  
	// 
	int cellSearchRxFFT(cFloat* w_rx_long)
	{
	
	  std::copy(&w_rx_long[0],&w_rx_long[m_search_fft_size],&m_rx_long[0]);
	  fftwf_execute(m_rx2F_long);
	  return 0;
	};
	
	// Correlation and max search 
	// for One PSS sequence in FFT domain
	//
	int cellSearchCorrelateAndGetMaxForOneSeq(int w_seq)
	{
	int i1=w_seq;
	
	for(int i2=0;i2<m_search_fft_size;i2++)
		{
			m_corrF_long[i1][i2]=m_rxF_long[i2]*m_pss_F_conj_search[i1][i2];
		}
        fftwf_execute(m_F2rx_long[i1]);
            
        // Max search
        // 
        // Normalizes the sequence get abs mean abs divide
        double tmpMean =0;
	for(int i2=0;i2<m_search_fft_size;i2++)
	  {
	    m_corrTAbs_long[i1][i2] = (double) std::abs(m_corrT_long[i1][i2]);
	    tmpMean += m_corrTAbs_long[i1][i2];
	  }
	tmpMean /= m_search_fft_size;
	
	// search for the max 
	m_maxValue[i1] = 0;
	m_maxLocation[i1] = -1;
	for(int i2=0;i2<m_search_fft_size;i2++)
         {
           m_corrTAbs_long[i1][i2] /= tmpMean;
           if(m_corrTAbs_long[i1][i2]> m_maxValue[i1])
           {
             m_maxValue[i1] = m_corrTAbs_long[i1][i2];
             m_maxLocation[i1] = i2;
           }
          }
	return 0;
	};

	// correlation in tracking mode only in symbol where PSS is 
	// 
	int cellTracking(cFloat* w_rx, int N_id_1)
	{

	  std::copy(&w_rx[0],&w_rx[m_fft_size],&m_rx[0]);
	  fftwf_execute(m_rx2F);

 	  int i1 = N_id_1;
	  
	  for(int i2=0;i2<m_fft_size;i2++)
	    {
			m_corrF[N_id_1][i2]=m_rxF[i2]*m_pss_F_conj_tracking[N_id_1][i2];
	    }
	  fftwf_execute(m_F2rx[N_id_1]);
	  
	  // search for the max
	  // get abs and normalize
	  // get abs
          
          double tmpMean =0;
	  for(int i2=0;i2<m_fft_size;i2++)
	  {
	    m_corrTAbs[i1][i2] = (double) std::abs(m_corrT[i1][i2]);
	    tmpMean += m_corrTAbs[i1][i2];
	  }
	  tmpMean /= m_fft_size;
	 
	  // search for the max 
	  m_maxValue[i1] = 0;
	  m_maxLocation[i1] = -1;
	  for(int i2=0;i2<m_fft_size;i2++)
         {
           m_corrTAbs[i1][i2] /= tmpMean;
           if(m_corrTAbs[i1][i2]> m_maxValue[i1])
           {
             m_maxValue[i1] = m_corrTAbs[i1][i2];
             m_maxLocation[i1] = i2;
           }
          }


          return 0;
	};	
	double getMaxCorrValue(int w_seq)
	{
	  return m_maxValue[w_seq];
	};

	double getMaxCorrValueLocation(int w_seq)
	{
	  return m_maxLocation[w_seq];
	};

        static int getPSS(int w_pssNr, cFloat* w_pss)
        {
                std::copy(&m_pss_seq[w_pssNr][0],&m_pss_seq[w_pssNr][62],w_pss);
                return 0;
        };
  
	public:
	int m_fft_size = 2048;
	int m_search_fft_size = 2048*8;

	constexpr static cFloat m_pss_seq[3][62] = {{{1.00000,0.00000},{-0.79713,-0.60380},{0.36534,-0.93087},{-0.73305,-0.68017},{0.98017,0.19815},{0.95557,0.29476},{-0.50000,-0.86603},{0.76604,-0.64279},{-0.22252,-0.97493},{0.62349,0.78183},{0.45621,0.88987},{0.36534,-0.93087},{0.95557,0.29476},{0.76604,-0.64279},{-0.50000,0.86603},{-0.73305,0.68017},{0.98017,0.19815},{-0.22252,0.97493},{0.62349,0.78183},{-0.79713,-0.60380},{-0.50000,-0.86603},{-0.50000,0.86603},{-0.79713,-0.60380},{-0.98883,0.14904},{0.95557,-0.29476},{0.98017,0.19815},{-0.22252,-0.97493},{1.00000,-0.00000},{0.76604,-0.64279},{-0.73305,0.68017},{-0.98883,0.14904},{-0.98883,0.14904},{-0.73305,0.68017},{0.76604,-0.64279},{1.00000,-0.00000},{-0.22252,-0.97493},{0.98017,0.19815},{0.95557,-0.29476},{-0.98883,0.14904},{-0.79713,-0.60380},{-0.50000,0.86603},{-0.50000,-0.86603},{-0.79713,-0.60380},{0.62349,0.78183},{-0.22252,0.97493},{0.98017,0.19815},{-0.73305,0.68017},{-0.50000,0.86603},{0.76604,-0.64279},{0.95557,0.29476},{0.36534,-0.93087},{0.45621,0.88987},{0.62349,0.78183},{-0.22252,-0.97493},{0.76604,-0.64279},{-0.50000,-0.86603},{0.95557,0.29476},{0.98017,0.19815},{-0.73305,-0.68017},{0.36534,-0.93087},{-0.79713,-0.60380},{1.00000,0.00000}},
		{{1.00000,0.00000},{-0.96908,-0.24676},{-0.73305,-0.68017},{0.07473,0.99720},{-0.79713,0.60380},{0.82624,0.56332},{-0.50000,0.86603},{0.76604,0.64279},{-0.90097,0.43388},{-0.22252,0.97493},{-0.41129,-0.91151},{-0.73305,-0.68017},{0.82624,0.56332},{0.76604,0.64279},{-0.50000,-0.86603},{0.07473,-0.99720},{-0.79713,0.60380},{-0.90097,-0.43388},{-0.22252,0.97493},{-0.96908,-0.24676},{-0.50000,0.86603},{-0.50000,-0.86603},{-0.96908,-0.24676},{0.95557,-0.29476},{0.82624,-0.56332},{-0.79713,0.60380},{-0.90097,0.43388},{1.00000,-0.00000},{0.76604,0.64279},{0.07473,-0.99720},{0.95557,-0.29476},{0.95557,-0.29476},{0.07473,-0.99720},{0.76604,0.64279},{1.00000,0.00000},{-0.90097,0.43388},{-0.79713,0.60380},{0.82624,-0.56332},{0.95557,-0.29476},{-0.96908,-0.24676},{-0.50000,-0.86603},{-0.50000,0.86603},{-0.96908,-0.24676},{-0.22252,0.97493},{-0.90097,-0.43388},{-0.79713,0.60380},{0.07473,-0.99720},{-0.50000,-0.86603},{0.76604,0.64279},{0.82624,0.56332},{-0.73305,-0.68017},{-0.41129,-0.91151},{-0.22252,0.97493},{-0.90097,0.43388},{0.76604,0.64279},{-0.50000,0.86603},{0.82624,0.56332},{-0.79713,0.60380},{0.07473,0.99720},{-0.73305,-0.68017},{-0.96908,-0.24676},{1.00000,0.00000}},
		{{1.00000,0.00000},{-0.96908,0.24676},{-0.73305,0.68017},{0.07473,-0.99720},{-0.79713,-0.60380},{0.82624,-0.56332},{-0.50000,-0.86603},{0.76604,-0.64279},{-0.90097,-0.43388},{-0.22252,-0.97493},{-0.41129,0.91151},{-0.73305,0.68017},{0.82624,-0.56332},{0.76604,-0.64279},{-0.50000,0.86603},{0.07473,0.99720},{-0.79713,-0.60380},{-0.90097,0.43388},{-0.22252,-0.97493},{-0.96908,0.24676},{-0.50000,-0.86603},{-0.50000,0.86603},{-0.96908,0.24676},{0.95557,0.29476},{0.82624,0.56332},{-0.79713,-0.60380},{-0.90097,-0.43388},{1.00000,0.00000},{0.76604,-0.64279},{0.07473,0.99720},{0.95557,0.29476},{0.95557,0.29476},{0.07473,0.99720},{0.76604,-0.64279},{1.00000,0.00000},{-0.90097,-0.43388},{-0.79713,-0.60380},{0.82624,0.56332},{0.95557,0.29476},{-0.96908,0.24676},{-0.50000,0.86603},{-0.50000,-0.86603},{-0.96908,0.24676},{-0.22252,-0.97493},{-0.90097,0.43388},{-0.79713,-0.60380},{0.07473,0.99720},{-0.50000,0.86603},{0.76604,-0.64279},{0.82624,-0.56332},{-0.73305,0.68017},{-0.41129,0.91151},{-0.22252,-0.97493},{-0.90097,-0.43388},{0.76604,-0.64279},{-0.50000,-0.86603},{0.82624,-0.56332},{-0.79713,-0.60380},{0.07473,-0.99720},{-0.73305,0.68017},{-0.96908,0.24676},{1.00000,-0.00000}}};

	int m_syncSymbolSize = 62;

	//int m_subframe_size = 30720;
	//int m_slot_size = 30720/2;
	int m_spectrum_scaling = 1;

	
	// vectors of Conjugate of the PSS sequences	
	cFloat **m_pss_F_conj_search;
	cFloat **m_pss_F_conj_tracking;

	// vectors for taking correlation from frequency to time and abs of it
	cFloat* m_rx;
	cFloat* m_rxF;
	cFloat** m_corrF;
	cFloat** m_corrT;
	double** m_corrTAbs;
	fftwf_plan m_rx2F;
	fftwf_plan *m_F2rx;

	cFloat* m_rx_long;
	cFloat* m_rxF_long;
	cFloat** m_corrF_long;
	cFloat** m_corrT_long;
	double** m_corrTAbs_long;
	fftwf_plan m_rx2F_long;
	fftwf_plan *m_F2rx_long;

	double m_maxValue[3]; // stores max values during cell search process
	double m_maxLocation[3]; 
	double decLevel = 5;
	int m_candidateSeq; // tracks which pss sequence gives highest value
	
};

