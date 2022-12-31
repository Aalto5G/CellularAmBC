#pragma once 

#include "Headers.hpp"
//#include "SYNC.hpp"

// SF reception
class SF{
	public:
	/*
	SF(SYNC *w_sync):m_sync(w_sync)
	{
	 m_channel_coeff = new double[7];
	};
	*/
	
	SF()
	{
	 m_channel_coeff = new double[7];
	};
	
	~SF()
	{
	};
	
	int initRx(int w_cell_id,int w_NRBDL)
	{
		p.generateAllPilots(w_cell_id, w_NRBDL);
		return 0;
	};
	
	
	int receiveSF(int subframe_number, int slot_number, int NRBDL, cFloat** w_sf, int w_fft_size)
	{
		cFloat* symF;
		
		/*
		//cFloat** sf;
		
		// m_sync->getSF();
		
		// get SF number 
		// get slot number
		int subframe_number = m_sync->getFrameNumber();
		int slot_number = m_sync->getSlotNumber();
		int NRBDL = m_sync->getNRBDL();
		*/
			
		int pilot_symbol_l=0;
		
		cFloat* pilots_in_this_symbol;
		int *pilots_location_in_this_symbol;

		// cFloat* estimated_channel_coef = new cFloat[m_sync->getFFTSize()];
		cFloat* estimated_channel_coef = new cFloat[w_fft_size];

		// goes through the symbols and makes channel estimate
		
		//std::fill(&m_channel_coeff[0],&m_channel_coeff[0]+7, (cFloat)(0.0, 0.0));
		std::fill(&m_channel_coeff[0],&m_channel_coeff[0]+7, (double)(0.0));
		
		for(int i1 = 0;i1<7;i1++)
		{
		pilot_symbol_l = 0;
		if((i1 == 0) || (i1 == 4))
		{
			pilot_symbol_l = 0;
			if(i1==4)
				pilot_symbol_l=1;
		
			pilots_in_this_symbol = p.getPilotsForTheSymbol(subframe_number,slot_number, pilot_symbol_l);
			pilots_location_in_this_symbol = p.getPilotsLocationsForTheSymbol(subframe_number,slot_number, pilot_symbol_l);
		
			symF = &w_sf[i1][0];
			//symF = m_sync->getSFSymbolfreqComponents(i1);
		
			//memset(&estimated_channel_coef[0],0,sizeof(cFloat)*m_sync->getFFTSize());
			memset(&estimated_channel_coef[0],0,sizeof(cFloat)*w_fft_size);
			// equalize 
			double tmp_channel_abs =0;
			for( int i2 = 0; i2<(2*NRBDL);i2++)
			{
				// std::cout<<pilots_location_in_this_symbol[i2]<<std::endl;
				cFloat tmpMult = symF[pilots_location_in_this_symbol[i2]]*pilots_in_this_symbol[i2];
				estimated_channel_coef[pilots_location_in_this_symbol[i2]] = tmpMult;
				tmp_channel_abs += std::abs(tmpMult);
				//	std::cout<<pilots_location_in_this_symbol[i2]<<" pilot:"<<pilots_in_this_symbol[i2]<<" rx_data:"<<rxF[pilots_location_in_this_symbol[i2]]<<" mult:"<<estimated_channel_coef[pilots_location_in_this_symbol[i2]]<<std::endl;
			}

			// makes max path estimate			
			m_channel_coeff[i1]= tmp_channel_abs;

		}
		std::cout<<m_channel_coeff[i1]<<" ";
		}
		std::cout<<std::endl;
		return 0;
	};
	
	double* getChannelCoeff()
	{
		return m_channel_coeff;
	}
	public:
//	SYNC *m_sync;
	Pilots p;	
	
	double* m_channel_coeff;
};

