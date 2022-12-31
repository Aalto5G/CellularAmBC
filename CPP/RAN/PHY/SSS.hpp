#pragma once 

#include "Headers.hpp"
#include "sssSequences.hpp"

/***********************************************************************
 * Class SSS
 **********************************************************************/

class SSS{
	
	public: 
	SSS(){
	
	std::cout<<"SSS initialization"<<std::endl;
	// FFT preparation 
	
	// All the SSS sequences in frequency domain and their conjugates
	m_fftSizeSSS = 128;
    m_sssM0Freq_conj = new cFloat*[504]; 
    m_sssM5Freq_conj = new cFloat*[504]; 
    
    /*
    for(int i1 =0;i1<31;i1++)
		std::cout<<sssNamespace::Seq0[0][i1].real()<<" ";
	std::cout<<std::endl;
	
	for(int i1 =31;i1<62;i1++)
		std::cout<<sssNamespace::Seq0[0][i1].real()<<" ";
	std::cout<<std::endl;
*/
	
    
    for( int i1=0; i1<504; i1++){
		m_sssM0Freq_conj[i1] = (cFloat*) fftw_malloc(m_fftSizeSSS * sizeof(cFloat));
		//memset(&m_sssM0Freq_conj[i1][0],0, m_fftSizeSSS * sizeof(cFloat));
		std::fill(&m_sssM0Freq_conj[i1][0],&m_sssM0Freq_conj[i1][0]+m_fftSizeSSS, (cFloat)(0.0, 0.0));
		
		std::copy(&sssNamespace::Seq0[i1][0],&sssNamespace::Seq0[i1][32],&m_sssM0Freq_conj[i1][m_fftSizeSSS-1-30]);
		std::copy(&sssNamespace::Seq0[i1][31],&sssNamespace::Seq0[i1][62],&m_sssM0Freq_conj[i1][1]);

		//m_syncInF_conj = new cFloat[m_fftSize];
		for(int i2=0;i2<m_fftSizeSSS;i2++)
				m_sssM0Freq_conj[i1][i2] = std::conj(m_sssM0Freq_conj[i1][i2]);
				
				
		m_sssM5Freq_conj[i1] = (cFloat*) fftw_malloc(m_fftSizeSSS * sizeof(cFloat));
		memset(&m_sssM5Freq_conj[i1][0],0, m_fftSizeSSS * sizeof(cFloat));

		std::copy(&sssNamespace::Seq5[i1][0],&sssNamespace::Seq5[i1][32],&m_sssM5Freq_conj[i1][m_fftSizeSSS-1-30]);
		std::copy(&sssNamespace::Seq5[i1][31],&sssNamespace::Seq5[i1][62],&m_sssM5Freq_conj[i1][1]);

		//m_syncInF_conj = new cFloat[m_fftSize];
		for(int i2=0;i2<m_fftSizeSSS;i2++)
			m_sssM5Freq_conj[i1][i2] = std::conj(m_sssM5Freq_conj[i1][i2]);

		} 

	// matrixes holding the correlation results in frequency and time 
	m_sssCorrM0F = new cFloat*[504]; 
	m_sssCorrM0T = new cFloat*[504];
	m_sssCorrM0TAbs = new float*[504];
	m_sssCorrM5F = new cFloat*[504]; 
	m_sssCorrM5T = new cFloat*[504]; 
	m_sssCorrM5TAbs = new float*[504];
    	for( int i1=0; i1<504; i1++){
		m_sssCorrM0F[i1] = (cFloat*) fftwf_malloc(m_fftSizeSSS * sizeof(cFloat));
		m_sssCorrM0T[i1] = (cFloat*) fftwf_malloc(m_fftSizeSSS * sizeof(cFloat));
		m_sssCorrM0TAbs[i1] = new float[m_fft128];
		m_sssCorrM5F[i1] = (cFloat*) fftwf_malloc(m_fftSizeSSS * sizeof(cFloat));
		m_sssCorrM5T[i1] = (cFloat*) fftwf_malloc(m_fftSizeSSS * sizeof(cFloat));
		m_sssCorrM5TAbs[i1] = new float[m_fft128];
	}

	
	m_fftSize = 1024;
	m_rxSSST = new cFloat[m_fftSize];
	m_rxSSSF = new cFloat[m_fftSize];
	m_rxSSSF_short = new cFloat[m_fft128];
	m_rxSSST2F = fftwf_plan_dft_1d(m_fftSize,(fftwf_complex*)&m_rxSSST[0],(fftwf_complex*)&m_rxSSSF[0],FFTW_FORWARD,FFTW_ESTIMATE);
	 
	
	
	m_fftForSSSM0F2T= (fftwf_plan*) malloc(sizeof(fftwf_plan*)*504);
    for( int i1=0; i1<504; i1++){
      m_fftForSSSM0F2T[i1] = fftwf_plan_dft_1d(m_fftSizeSSS,(fftwf_complex*)&m_sssCorrM0F[i1][0],(fftwf_complex*)&m_sssCorrM0T[i1][0],FFTW_BACKWARD,FFTW_ESTIMATE);
      }

	m_fftForSSSM5F2T = (fftwf_plan*) malloc(sizeof(fftwf_plan*)*504);
    for( int i1=0; i1<504; i1++){
      m_fftForSSSM5F2T[i1] = fftwf_plan_dft_1d(m_fftSizeSSS,(fftwf_complex*)&m_sssCorrM5F[i1][0],(fftwf_complex*)&m_sssCorrM5T[i1][0],FFTW_BACKWARD,FFTW_ESTIMATE);
      }

	std::cout<<"SSS initialization done"<<std::endl;
	};
	
	~SSS(){
		
	    for( int i1=0; i1<504; i1++){
			delete(m_sssM0Freq_conj[i1]);
			delete(m_sssM5Freq_conj[i1]);
			
			fftwf_free(m_sssCorrM0F[i1]);
			fftwf_free(m_sssCorrM0T[i1]);
			delete(m_sssCorrM0TAbs[i1]);
			fftwf_free(m_sssCorrM5F[i1]);
			fftwf_free(m_sssCorrM5T[i1]);
			delete(m_sssCorrM5TAbs[i1]);
			

			fftwf_destroy_plan(m_fftForSSSM0F2T[i1]);
			fftwf_destroy_plan(m_fftForSSSM5F2T[i1]);
		}
		delete(m_sssM0Freq_conj);
		delete(m_sssM5Freq_conj);
		delete(m_sssCorrM0F);
		delete(m_sssCorrM0T);
		delete(m_sssCorrM0TAbs);
		delete(m_sssCorrM5F);
		delete(m_sssCorrM5T);
		delete(m_sssCorrM5TAbs);
		delete(m_rxSSST);
		delete(m_rxSSSF);
		delete(m_rxSSSF_short);

		fftwf_destroy_plan(m_rxSSST2F);
		free(m_fftForSSSM0F2T);
		free(m_fftForSSSM5F2T);

	};
	
	int searchFrameNumberFromCorrWithSSS(cFloat* w_rx, int w_N_id_1, int* o_FrameNumber, int* o_estSequence, int* o_maxLoc){
		
		m_N_id_1 = w_N_id_1;
		// copy rx data into fft structure 
		std::copy(&w_rx[0],&w_rx[m_fftSize],&m_rxSSST[0]);
		// fft of received data
		fftwf_execute(m_rxSSST2F);
		
		// copy freq components into smaller size 
		std::copy(&m_rxSSSF[0],&m_rxSSSF[64],&m_rxSSSF_short[0]);
		std::copy(&m_rxSSSF[1023-63],&m_rxSSSF[1024],&m_rxSSSF_short[64]);
		
		/*std::cout<<std::endl;
		std::cout<<std::endl;

		for(int i1 =0;i1<128;i1++)
			std::cout<<m_rxSSSF_short[i1].real()<<" ";
		std::cout<<std::endl;
		*/
		
		// mult with complex conjugate 
		for(int i1=m_N_id_1;i1<504;i1+=3)
		{
			
			for(int i2=0;i2<m_fft128;i2++)
			{
				m_sssCorrM0F[i1][i2]=m_rxSSSF_short[i2]*m_sssM0Freq_conj[i1][i2];
				m_sssCorrM5F[i1][i2]=m_rxSSSF_short[i2]*m_sssM5Freq_conj[i1][i2];
			}
		}
		// goes through ifft 
		for(int i1 = m_N_id_1;i1<504;i1+=3)
		{
			fftwf_execute(m_fftForSSSM0F2T[i1]);
			fftwf_execute(m_fftForSSSM5F2T[i1]);
		}
		// search for max 
		m_maxValM0=0;
		m_maxLocM0=-1;
		m_maxLocM0time=-1;
		m_maxValM5=0;
		m_maxLocM5=-1;
		m_maxLocM5time=-1;

		for(int i1 = m_N_id_1;i1<504;i1+=3)
		{
			int i2 = 0;
			for(int i2=0;i2<m_fft128/2;i2++)
			{
				m_sssCorrM0TAbs[i1][i2]=std::abs(m_sssCorrM0T[i1][i2]);
				m_sssCorrM5TAbs[i1][i2]=std::abs(m_sssCorrM5T[i1][i2]);
				if(m_sssCorrM0TAbs[i1][i2]>m_maxValM0)
					{
						m_maxValM0 = m_sssCorrM0TAbs[i1][i2];
						m_maxLocM0 = i1;
						m_maxLocM0time = i2;
					//	std::cout<<" M0:"<<m_maxValM0<<" "<<i1<<" "<<i2<<"\n";
					}
				if(m_sssCorrM5TAbs[i1][i2]>m_maxValM5)
					{
						m_maxValM5 = m_sssCorrM5TAbs[i1][i2];
						m_maxLocM5 = i1;
						m_maxLocM5time = i2;
					//	std::cout<<" M5:"<<m_maxValM5<<" "<<i1<<" "<<i2<<"\n";
					}
			}			
		}		
		m_FrameNumber = 0;
		m_maxVal = m_maxValM0;
		m_maxLoc = m_maxLocM0;
		m_maxLoctime = m_maxLocM0time;
		if(m_maxValM0<m_maxValM5)
		{
		m_FrameNumber = 5;
		m_maxVal = m_maxValM5;
		m_maxLoc = m_maxLocM5;
		m_maxLoctime = m_maxLocM5time;
		}
		
		o_FrameNumber[0] = m_FrameNumber;
		
		o_estSequence[0] = m_maxLoc;
		o_maxLoc[0] = m_maxLoctime;
		// compute cell ID
		
		return 0;
	};
	
	int printData()
	{
		std::cout<<"initializing SSS"<<std::endl;
		std::cout<<"fftSSS size:"<<m_fftSizeSSS<<std::endl;
		std::ofstream myfile4;
		myfile4.open("fSSS1.dat", std::ios::binary | std::ios::out);	
		//myfile.write((char*)&dl_buff_collected[0],sizeof(cFloat)*tmpSamps);
		myfile4.write((char*)&m_sssM0Freq_conj[0][0],sizeof(cFloat)*m_fftSizeSSS);
		myfile4.close();
		for(int i1 =0;i1<128;i1++)
		{
			std::cout<<m_sssM0Freq_conj[0][i1].real()<<" ";
		}
		std::cout<<std::endl;
		
		// test that the algorith works 
		
		// generate the sequene N in time. 
		int N = 12;
		cFloat* tmpDataRxF = new cFloat[m_fftSize];
		cFloat* tmpDataRxT = new cFloat[m_fftSize];
		
		memset(&tmpDataRxF[0],0, m_fftSize * sizeof(cFloat));
		
		
		{
		int i1 = N;
		std::copy(&sssNamespace::Seq5[i1][0],&sssNamespace::Seq5[i1][32],&tmpDataRxF[m_fftSize-1-30]);
		std::copy(&sssNamespace::Seq5[i1][31],&sssNamespace::Seq5[i1][62],&tmpDataRxF[1]);
		
		
		auto test_rxF2T = fftwf_plan_dft_1d(m_fftSize,(fftwf_complex*)&tmpDataRxF[0],(fftwf_complex*)&tmpDataRxT[0],FFTW_BACKWARD,FFTW_ESTIMATE);
		fftwf_execute(test_rxF2T);
		}
		
		
		int frameNr=-1;
		int estSequence = -1;
		int maxLoc = -1;

		int w_N_id_1 = 0;
		searchFrameNumberFromCorrWithSSS(tmpDataRxT, w_N_id_1, &frameNr, &estSequence, &maxLoc);
		
		std::cout<<"estimated frameNr"<<frameNr<<" "<<estSequence<<" "<<maxLoc<<std::endl;
		
		
		return 0;
	};
	
	public:
	int m_fftSize;
	int m_fftSizeSSS;
	int m_fft128 = 128;

	// vectors for received signal FFT and scaling down to 128 elements
	cFloat *m_rxSSST;
	cFloat *m_rxSSSF;
	cFloat *m_rxSSSF_short;

	// vectors of Conjugate of the SSS sequences	
	cFloat **m_sssM0Freq_conj;
	cFloat **m_sssM5Freq_conj;

	// vectors for taking correlation if frequency to time and abs of it
	cFloat **m_sssCorrM0F;
	cFloat **m_sssCorrM0T;
	float  **m_sssCorrM0TAbs;
	cFloat **m_sssCorrM5F;
	cFloat **m_sssCorrM5T;
	float  **m_sssCorrM5TAbs;

	// storage of abs values and locations 	
	int m_maxValM0;
	int m_maxLocM0;
	int m_maxLocM0time;
	int m_maxValM5;
	int m_maxLocM5;
	int m_maxLocM5time;

	int m_maxVal;
	int m_maxLoc;
	int m_maxLoctime;
	
	int m_FrameNumber;

	int m_N_id_1;
	int m_N_id_2;

	fftwf_plan m_rxSSST2F;
	fftwf_plan *m_fftForSSSM0F2T;
	fftwf_plan *m_fftForSSSM5F2T;

};
